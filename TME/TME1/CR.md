# Introduction

# Exercice 1 :

# Question 1 :
```
Dans le cadre de ce TP on supposera toujours qu’un niveau intermédiaire de la table des pages est mappé
par une identité (il est accessible par une adresse virtuelle égale à son adresse physique). Pourquoi cette supposition est-elle importante ?
```

L'adresse physique de la table des pages est stockée dans le registre CR3, le CPU manipulant uniquement des adresses virtuelles il a besoin que l'adresse de la table des pages n'ait pas besoin d'être traduite. Si elle avait besoin d'une traduction il faudrait qu'il accède à la table des pages, or il ne peut pas y accéder s'il manipule une adresse virtuelle nécessitant une traduction.


# Question 2 :

```
Combien y a-t-il d’entrée par niveau de table des pages pour une architecture x86 64 bits ? Comment la
MMU détermine-t-elle si une entrée d’un niveau de table des pages est valide ? Si elle est terminale ?
```

La table des pages est divisée en 4 PML, chacune de ces PML étant encodé par 9 bits de l'adresse, elles contiennent donc 2^9 entrées.

La table des pages contient une adresse sur 64 bits, l'adresse est découpée comme suit :
* addr[63] : NX, indique si la page est accessible en exécution
* addr[62:52] : 
* addr[51:12] : Adresse de la page physique
* addr[11:0] : bits donnant des informations sur la page

On trouve dans les bits [11:0] des informations sur la validité de la page.



# Question 3 : 

```
Dans kernel/main.c, programmez et testez la nouvelle fonction print_pgt. Vous testerez votre nouvelle fonction en l’appellant depuis la fonction main multiboot2 située dans le fichier kernel/main.c juste avant le chargement des tâches utilisateur. Pour cela vous pouvez utiliser la fonction uint64 t
store cr3(void) qui retourne le contenu du registre CR3.
Si votre fonction d’affichage fonctionne correctement, vous devriez observer une table des pages avec la
structure indiquée dans l’annexe de ce sujet.
```

# Exercice 2 :

Pour des soucis de simplicité, on fait les suppositions suivantes :
* Les niveaux intermédiaire de la table des pages sont tous mappés par une identité
* Tous les niveaux de mappings seront fait avec les droits utilisateurs et en écriture
    * bit U/S à 0
    * bit R/W à 1
* On ne gère pas les huge page, donc on a bien les 4 niveaux de PML
* Si un mapping est demandé pour une adresse virtuelle a, l'adresse virtuelle a n'est pas déjà mappée. La fonction ``paddr_t alloc_page()`` alloue une page déjà mappée par une identité.

# Question 1 :

```
Etant donné une adresse virtuelle vaddr à mapper et la hauteur courante dans la table des pages lvl
(avec lvl = 4 pour le niveau indiqué dans le CR3), donnez la formule qui indique l’index de l’entrée
correspondante dans le niveau courant.
```

* On élimine les bits de poids fort : vaddr & 0xFFFF FFFFFFFF
* On shift de sorte à éliminer les 12 bits de poids faible et à accéder à la bonne PML :
    (vaddr & 0xFFFF FFFFFFFF) >> (12 + 9*(lvl - 1))

# Question 2 :

Implémentez la fonction ``void map_page(struct task *ctx, vaddr_t vaddr, paddr_t paddr)``
dans le fichier kernel/memory.c qui mappe l’adresse virtuelle vaddr sur l’adresse physique ``paddr`` sur
un espace d’une page pour la tâche ctx. L’adresse physique du premier niveau de la table des pages est
indiquée par ctx->pgt.
Pour cet exercice, n’hésitez pas à tester très régulièrement votre fonction à chaque étape de son implémentation.
On pourra pour cela utiliser le morceau de code suivant depuis kernel/main.c :

```c
struct task fake;
paddr t new;

fake.pgt = store_cr3();
new = alloc_page();

map_page(&fake, 0x201000, new);
```

Si votre fonction map page fonctionne, vous devriez pouvoir inspecter la mémoire à l’adresse 0x201000 à
l’aide du moniteur de Qemu avec la commande x/8g 0x201000.

