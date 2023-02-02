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

# Virtualisation comme isolation