# Virtualisation d'instructions
## Simulation cycle accurate

Dans le cas d'un simulation cycle accurate, on va simuler l'ensemble des composants de la machine virtuelles. On peut faire ca grace à des langages HDL (VHDL, verilog, systemC...etc).

Cette simulation permet une observation fine des exécutions :
* estimation des perfs au cycle près
* interaction logiciel/matériel observable et reproductible
....

Beaucoup de calculs nécessaire -> simulation lente (facteur 100)
## De la simu à l'ému

* Usage de VM :
    * simuler nveau matériel
    * portage logiciel sur différentes archi

Bref l'idée c'est d'émuler le jeu d'instruction.

Vocabulaire :
* **système hote**    : ensemble des programmes qui utilisent directement les ressources de la machine
* **système invité**  : ensemble des programmes qui utilisent les ressources de la machine via l'interface d'une VM
* **moniteur de machine virtuelles** : VMM est un logiciel qui présente une interface de VM aux système invité

![plot](images/VMM.png)

* Moniteur de type 1 : 
Il s'execute en mode privilégié et est l'unique programme hote. On gros c'est comme si on lancait un OS sauf que l'hyperviseur va permettre de démarrer plusieurs machine virtuelle typiquement.
* Moniteur de type 2 : 
Il s'exécute en mode User et est en concurrance avec d'autres programmes hôtes

![plot](images/vmm_2.png)

Qemu-KVM est de type "1.5" (KVM = kernel virtual machine, c'est un module qu'on insère dans le noyau Linux)

## **Emulation d'instruction**

Etape :
* récupère le code système invité (inst suivante)
* décode l'instruction
* le guest execute l'instruction

exemple :
```c
while(TRUE){
    bin = guest.next_instruction();
    inst = decode_instruction();
    guest.execute_instruction();
}
```
Les instructions décodées logiciellement selon l'opcode de la machine virtuelle.

L'hyperviseur doit émuler les composants matériels, il va donc créer un vcpu qui est une structure mémoire qui va contenir les registres virtuelles. L'adresse de ces registres n'est pas accessible depuis la machine guest.

**VMM = virtual machine manager**

Le VMM utilise uniquement des instructions non privilégiées pour modifier l'état de la VM.

### **Conclusion**
* L'émulation d'instructions est une technique de virtualisation où le VMM décode et interprète chaque instruction du système invité.
* Le VMM execute les instructions décodés en utilisant un état virtuel de la machine exposée au système invité
* L'émulation est plus rapide que la simu bit accurate mais reste plus lente qu'une exécution physique

## **Émulation optimisée**

Quand on a une boucle, on va en permanence décoder les même trucs.  Le travail du décodage pourrait être réexécuté.

Au lieu de faire inst par inst on va travailler sur des basic block. On va fetch, decoder et exécuter des **basic block**.

**basic block** = partie du code non conditionnelle, ie partie du code délimitée par des branchements. En pratique il doit y avoir une taille limite mais rarement atteinte.

**Compilation à la volée :**
Abandon de la boucle **fetch-decod-execute**, utilisation d'une boucle **fetch-compile-branch**.

Les basic blocks sont compilés vers le jeu d'instructions de l'hôte. Cette compilation s'appelle une **Dynamic Binary Translation** (DBT).

### Problèmes

Il n'y a pas toujours une correspondance 1:1 entre les jeux d'instructions. Parfois une instruction devient deux instructions.

Le pb c'est que si la taille du bloc varie les adresses des instructions peuvent changer et par conséquent les adresses des branchements peuvent ne plus être les même.

Il faut sauvegarder le contexte du cpu puis le restaurer entre chaque basic bloc.

Les blocs source sont branchés les uns sur les autres.

Les instructions privilégiées sont traduitent par des appels de fonctions d'émulation.

### Conclusion 

* **Le dynamic binary translation** est une technique de virtualisation où le VMM compile et exécute chaque basic block du système invité
* un basic block est une liste d'instruction dont la dernière est l'unique instruction de saut
* instructions non privilégiées sont traduites par une ou plusieurs instrucitons non privilégiée équivalente
* instruction privilégiées sont traduites par un appel à la fonction d'émulation correspondante

# Virtualisation de ressources

Le système invité accède à sa propre mémoire, il a donc besoin d'avoir une correspondance mémoire virtuelle <--> mémoire physique.

![plot](images/virtua_adresses.png)

Pour les VMM de type 2, on a donc les types d'adresse suivantes :
* GVA : guest virtual addresses
* GPA : guest physical addresses
* HVA : host virtual addresses
* HPA : host physical addresses


Pour les VMM de type 1, on ne distingue pas les (GPA) et les (HVA), on utilise plutôt la terminologie virtuelle/physique/machine.

La VMM utilise une structure logicielle quelconque pour faire l'association `GPA -> HVA`.
La MMU physique utilise la table des pages du système hôte pour faire la traduction `HVA -> HPA`.

Le système invité attend que la MMU fasse la traduction `GVA -> GPA` :
* Solution : émuler l'action de la MMU à chaque accès mémoire invité
* Le VMM parcours la PTE en utilisant un CR3 virtuel

Donc on part d'un vCR3, qui donne accès a une vMMU qui va permettre de faire la traduction `GVA` -> `GPA`. 

![plot](images/shadowing.png)

Le problème c'est que faire ca c'est super couteux, donc on va plutôt essayer d'utiliser la MMU physique pour faire directement `GVA` -> `HPA`. Mais on ne peut pas laisser l'invité accéder directement la MMU physique. On fait comme ca du coup :

![plot](images/exercice_shadowing.png)

Le programme veut accéder à la case qui contient 0x23000 qui est en faite mappée sur 0x17000.

Donc le guest envoie 0x1000 (GVA) pour accéder à 0x23000 (GPA) qui est traduit en 0x17000 (HPA)

On imagine maintenant que l'invité veut faire un mapping VA = 0x4000 -> PA = 0x14000 :
* Pour intercepter l'écriture le VMM protège la table invitée en écriture
* Le VMM est averti de cette tentative d'écriture par une page fault
* Le VMM associe l'adresse virtuelle fautive à l'adresse machine de son choix
* Le VMM modifie la table des pages invité pour écrire l'adresse physique

Ca veut dire que le système invité ne peut pas directement écrire dans la table des pages invité, ca passe par le VMM et c'est lui qui réalise l'écriture.

Exemple avec VMM type 1 :
![plot](images/shadowing_vmm1.png)

Pour les VMM de type 2, c'est un peu différent :

![plot](images/shadowing_vmm2.png)

* L'hote et l'invité partagent le même espace virtuel :
    * l'invité peut utiliser une VA déjà atttribuée à l'hôte
    * les GVA et les HVA sont traitées de la même manière par la MMU physique

* différentes solutions pour isoler l'invité 
    * utiliser un espace virtuel dédié pour l'invité
    * émuler tous les accès à une zone réservée à l'hote
    * déplacer l'hôte en cas de conflit 
## **Résumé**

* Le système invité définit une correspondance GVA et GPA
* Le système hôte utilise la MMU pour associer HVA et HPA
* La VMM assure une traduction GVA vers HPA sans collision :
    * entre les différents espaces virtuels invités
    * entre les espaces virtuels invités et l'espace virtuel hôte
* La première méthode est la traduction logicielle
    * La VMM émule la traduction via une MMU logicielle
    * Les GPA sont traduites une seconde fois pour éviter les collisions
* La deuxième méthode est la shadow table
    * Le VMM maj la table des pages hote en fonction des actions de l'invité -> MMU traduit GVA en HPA
    * Le VMM utilise une combinaisons d'autres méthodes pour éviter collision hote/invité.

# Paravirtualisation et interface matérielle

shadowing de PT évite traduction logicielle des adresses -> accès mémoire éfficace mais :
* surcout pour tte écriture dans PT
* surcout pour tte modification du vCR3

Le shadowing est possible pour du matériel configuré par l'hôte mais dont l'exécution est automatique -> MMU, ICU...etc

Impossible pour le matériel aux actions commandées explicitement par l'hote -> controleur d'E/S, instructions privilégiées...etc


Le communication entre invité et VMM est complexe :
* Interface machine complexe
* Pilotes complexes

Complexité a un impact sur les performances

Solution -> supprimer l'interface machine virtuelle :
* modifier l'invité pour faire directement appel au VMM
* L'invité sait qu'il est virtualisé
* Communication par hypercall

Pour un invité sans paravirtualisation 
* VMM doit émuler chaque instruction privilégiée

Un système paravirtualisé utilise un unique hypercall à la place :
* Peut être n'importe quelle inst que le VMM intercepte et qui ne correspond à aucune action légitime pour le matériel
* La VMM discrimine les hypercalls des fautes avec une ABI prédéfinie
* Si c'est un hypercall, le VMM décode selon l'ABI définie et traite la demande si elle est légitime

## Résumé paravirtualisation

```
La paravirtualisation est une technique de virtualisation qui implique la modification du système d'exploitation invité pour fonctionner en collaboration avec l'hyperviseur. Au lieu d'émuler complètement la machine physique pour chaque machine virtuelle, les invités sont modifiés pour être conscients de la virtualisation et communiquer directement avec l'hyperviseur.
```

Les interfaces matérielles complexes sont remplacées par des interfaces paravirtualisées basées sur les hypercalls :
* hypercall simple et rapide à décoder par le VMM
* pilotes invités paravirtualisés simples et efficaces
* système invité est modifié pour s'exécuter dans un VMM donné

# Virtualisation comme isolation

## Des émulateurs aux hyperviseurs

Rappel objectif emulation : 
* simuler nveau matos
* porter logiciel sur différentes archi
* partage ressources -> exécution simultannée Windows/Linux
* Isolation de services -> exécution simultannée de plusieurs Linux

Dans cas où on veut émuler une VM semblable à machine physique (meme jeu instruction) : 
* pas besoin de traduire instructions non privilégiées
* besoin intercepter sauts et instructions privi

### **Exécution directe du code invité** 

Dynamic block + rapide quand archi source et cible sont les même.
Blocs de code non privilégié ont juste un surcout de chainage. Surcout disparait quand tous les blocs sont traduits. **Traduction inutile pour code non privi**.

* Après phase initialisation hyperviseur se branche directement sur le code du système invité en mode user

* Instructions non privilégiées sont exécutée normalement, isolation mémoire assurée via pagination.

`Isolation par pagination` = Chaque machine virtuelle a sa propre table de pages, qui mappe ses adresses de mémoire virtuelles à des adresses de mémoire physique sur le système hôte.\
Lorsqu'une machine virtuelle accède à sa mémoire, l'hyperviseur intercepte l'opération et la traduit en accès à la mémoire physique sur le système hôte. Les pages de mémoire utilisées par chaque machine virtuelle sont protégées les unes des autres, ce qui signifie que les erreurs dans une machine virtuelle ne peuvent pas affecter les autres machines virtuelles ou le système hôte.

* processeur n'exécute pas les instructions privilégiées en mode user :
    * a la place le cpu declenche une **faute de protection**
    * la faute est traitée par l'hyperviseur et l'instruction invitée est émulée
    * dans le cas d'un hyperviseur de type 2, l'OS redirige la faute de protection vers l'hyperviseur

Mieux résumé par SOPENA :
```
Truc qui gère les interruptions (équivalent ICU):
* pic (program interruptions control)
* apic (advanced program interuption control)

Implémentation d'un virtual controleur d'interruption en software.

Pour faire des instructions privilégiés, on les laisse s'exécuter et ca va provoquer une exception que l'on va récupérer.

Si on est en type 2 où on émule tout, on remplace les syscall par le code qui les traite. (Le remplacement est fait à la compilation)
En type 1 on laisse les instructions s'exécuter et c'est le handler de l'hyperviseur qui récupère l'interruption (puisqu'il est en mode S) et qui va la traiter. 
```

Pb avec instructions qui ont un comportement différent en USER et SUPERVISEUR

## Résumé exécution directe et interception

* Un cas particulier de VMM est l'**hyperviseur** :
    * La VM exposée au systeme invité a la même architecture que la machine physique 
    * La traduction binaire dynamique y est plus rapide -> copie des instructions privilégies

* Exécution directe est la méthode qui consiste à exécuter directement le code invité en mode user
    * isolation mémoire identique à celle des processus hote
    * instructions privilégiées interceptée et émulées par l'hyperviseur
## Cout de la virtualisation

* Une DBT (dynamic block translation) ou une exécution directe supprime le surcout d'exécution des instructions non privilégiées.
* Les opérations privilégiées ne sont pas sensiblement dégradée 

* Operation invitées qui réduisent les perfs sont (sans paravirtualisation) :
    * shadowing PT (cout du contexte switch)
    * émulation matériel I/O
* Exécution directe impossible pour archi X86 à cause des inst silent fail

CPU moderne fournissent assistance matérielle à la virtualisation

## Assistance matérielle à la virtualisation : principe

* Hyperviseur définit une zone de contrôle par vCPU et une zone de sauvegarde par CPU
* instruction dédiée permet au cpu de passer en mode invité

Cette instruction c'est `vmrun`, quand elle est appelée le contexte du cpu va etre sauvegardé dans la zone dédiée (la zone est en mémoire RAM) et le contexte de l'invité va être chargé.

Deux modes :
* cpl : KERNEL/USER
* mode : GUEST/HOST

Le processeur peut néamoins intercepter certaines instructions : il y alors une VMEXIT

* En mode hote la MMU traduit les adresses avec la table des pages de l'hyperviseur pointée physiquement par le cr3.
* En mode guest, le cr3 pointe physiquement sur la table des pages du systeme invité.
Un autre registre ncr3 contient l'adresse machine d'une seconde table des pages accessible uniquement par l'hyperviseur.

Nulle part dans le noyau on modifie ncr3, en faite le noyau modifie tjrs cr3 et c'est en fonction du mode que le CPU va écrire dans ncr3 ou cr3.

C'est la MMU qui gère en faite la traduction d'adresse virtuelle -> physique et physique -> machine.
En gros elle recoit adresse virtuelle du cpu, elle traduit ca en adresse physique en regardant la valeur du cr3. Ensuite elle regarde le mode et si ce mode est à GUEST, alors elle va traduire l'adresse physique en adresse machine en utilisant l'adresse contenue dans ncr3.

Ce qu'on a fait en logiciel avec la shadow table est en faite géré directement par la MMU physique.

## IOMMU et transfert DMA

Machine moderne utilisent des IOMMU, placées entre la mémoire et les controleurs DMA

# LIRE LES 3 DERNIERES SLIDES OU IL RESUME TOUT