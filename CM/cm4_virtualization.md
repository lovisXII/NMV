# Introduction Virtualisation système : Pourquoi ?

Utiliser dans de nombreux domaines. Gros impact de perf (via impact mem et latences).

-> Simulation cycle accurate :\
Permet simulation d'un composant (systemC/VHDL)

Deux types de moniteurs de MV (machine virtuelles) :
* **moniteur de type 1** : s'exécute en mode privilégié, unique programme hote (VMware, ESX, Xen). On boot dessus normalement au démarrage de la machine. Apparemment sert principalement à émuler un type d'architecture.
* **moniteur de type 2** : s'exécute en mode U souvent c'est une application, en concurrence avec d'autres programmes hotes. Exemple type VirtualBox, c'est un truc qui tourne sur un OS.

Qemu-KVM est de type "1.5" (KVM = kernel virtual machine, c'est un module qu'on insère dans le noyau Linux)

Hyperviseur = moniteur de machine virtuelle

**Uni-kernel : noyau dédié à une appli qui tourne dans le noyau, en gros c'est une appli normale mais qui tourne en mode S**
**Exo-kernel : offre uniquement un service d'interruption**

# Emulation d'instruction

Le guest c'est la machine virtuelle invité.

Etape :
* récupère le code système invité (inst suivante)
* décode l'instruction
* le guest execute l'instruction

Pour chaque CPU virtuel on va lui associer une structure virtuelle qui contient ses registres. Cette structure est accessible par le moniteur de la VM. L'adresse de ces registres n'est pas accessible depuis la machine guest.

Pour intéragir avec cette structure le système guest va envoyer des instructions assembleurs, de son point de vu il manipule les registres "normalement".


# Emulation d'instructions optimisées 

Quand on a une boucle, on va en permanence décoder les même trucs.  Le travail du décodage pourrait être réexécuté.\
C'est de l'interprétation ce qu'on a fait la, c'est le concept de bash, il lit, il décode et exécute en séquentiel.

Au lieu de faire inst par inst on va travailler sur des basic block. On va fetch, decodé et exécuté des **basic block**.

(rip = pointeur d'inst)

**basic block** = partie du code non conditionnelle, ie partie du code délimitée par des branchements. En pratique il doit y avoir une taille limite mais rarement atteinte.

Il n'y a pas toujours une correspondance 1:1 entre les jeux d'instructions. Parfois une instruction devient deux instructions.

# Branchements et chainage de blocs

Le fait qu'on ait pas une correspondance 1:1 sur les instructions rends difficile le chainage des blocs. En effet la taille du code peut varier et donc les adresses de retour ou les adresses de branchement peuvent changer.

Il faut sauvegarder le contexte du cpu puis le restaurer en chaque basic block. 

Les blocs résultats peuvent être mis en cache pour être réutilisé (dans le cas des boucles par exemple)

Rosetta2 : apple passé de x86 à du arm -> leur pb c'était surtout comment émuler un jeu d'instruction.

# VM java

C'est un langage compilé maintenant. Au départ on compilait pas (langage interprété avant).
Java fait de la compilation "just in time", ie on compile que quand on va se servir du code.

Compiler peut être lent car il fait de l'optimisation. Le plus lent c'est pas de compiler, c'est de faire de l'optimisation.

inline (mot clef c) : parfois fait par défaut par le compilo pour faire de l'opti

Les objets sont alloués dans la pile en java

# Virtualisation et adressage

On a un virtual cpu qui va accéder à une virtual ram. La vram c'est rien d'autre que des varibles. Une adresse physique de la fausse ram est en faite stockée dans une adresse virtuelle du vrai cpu.

Il y a un problème d'isolation entre une app dans la vm et dans la machine hote.

On peut pas avoir une bijection entre le VMM et le vrai OS, la VMM ne peut pas imposer des adresses au vrai OS.

Dans un système 32 bit, le kernerl est mappé sur le dernier giga (de 3 à 4).

Donc si l'hyperviseur choisi les adresses de 3 à 4Go pour son OS ca va faire un conflit avec le vrai OS.

## Dans moniteur de type 1

adresse virtuelle : adresse dans la vm
adresse physique = ram virtuelle
adresse machine = vrai ram

# Virtualisation et MMU logicielle

__translate_guest_to_host : emule la ram

Il faut une fausse MMU, qui traduire les adresses virtuelles du vcpu vers la vram. 

1. Je récupère du code qui est une adresse virtuelle dans la vm
2. 
3. 
4. 

# Shadowing mémoire : principe

La fausse MMU fonctionne comme la vrai MMU, on fait comme ca puisque l'OS invité ne sait pas qu'il est dans un VM. Il faut donc qu'il puisse fonctionner normalement.

Le vcpu accède à la vMMU en envoyant une adresse GVA (guest virtual adress) qui va être traduite en GPA (guest physical adresse).\
Cette GPA va être envoyée à une fonction de mapping dans l'hyperviseur qui va la transformer en HVA (host virtual adress) qui va ensuite être manipulée normalement par le cpu et envoyée à la MMU pour être traduite en HPA (host physical adress).

CR0 : flag de fonctionnement, dont le mode du cpu 
CR3 : adresse de la table des pages


# Shadowing de table de pages : VMM de type 1

On veut dans un premier temps accéder au vCR3. 
Pour faire ca on doit traduire l'adresse du vCR3.
Le vCR3 est à l'adresse 0x1000, cette adresse contient la valeur du vCR3.

Protection en écriture de la table invitée pour générer une exception.

Dans la slide : RW (read/write) & RO (read only) 

# Shadowing de table de pages : VMM de type 2

Dans un type 2, l'os invité est en mode U donc il ne peut pas modifier la table des pages.

On va utiliser ``mprotect`` pour protéger la table des pages invitées et générer un appel système. mprotect ne crash pas le programme, il redonne la main au handler.\


# Interface matérielle et coût d'interception

# Des émulateurs aux hyperviseurs

Truc qui gère les interruptions (équivalent ICU):
* pic (program interruptions control)
* apic (advanced program interuption control)

Implémentation d'un virtual controleur d'interruption en software.

# Remarques

* ServerX : X c'est la v2 de W, ca vient de windows à la base (W->X)
* Wayland : Wayland est un protocole de serveur d'affichage, ainsi qu'une bibliothèque logicielle libre disponible sur les systèmes d'exploitation GNU/Linux. Wayland fournit un moyen pour les gestionnaires de fenêtres composite de communiquer directement avec les applications graphiques ainsi que le matériel vidéo.