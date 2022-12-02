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
