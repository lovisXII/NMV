# Introduction : Gestion de la mémoire 

Un OS n'est ni plus ni moins qu'un gestionnaire d'interruption.

SIMD ~= opération vectorielle (single instruction multiple data)

Deux leviers pour gérer mémoire :
* systeme efficace -> bonne gestion de la mémoire
* applications efficaces -> tirent parti de la gestion de la mémoire

# Gestion de la mémoire dynamique : la pile

## En x86 :

### Architecture size
Ax -> nom registre 16 bits\
EAx -> nom registre 32 bits\
RAx -> 64 bits

## Gestion pile exemple

code c:
```c
int f(long x)
{
    long y = 13
    return x+y
}

int main(void){
    long a,b = 12
    a = f(b)
    return 0
}
```
code assembleur format x86 64 bits :
```s
sub $8, %rsp        # case vide pour a
push $12            # push 12 dans la pile pour b
mov 0(%rsp), %rdi   # passage de paramètre
call f              # appel de f
add $16, %rsp       # restaure la pile 
ret
```

### **Epilogue** :

```c
add $16, %rsp       # restaure la pile 
ret
```

### **Syntaxe** :

* Quand on a ``%rsp`` on voit direct que c'est du 64 bits.\
* Si on avait ``%esp`` par exemple on sait que c'est du 32 bits.
* Les immédiats sont notés avec des dollars en x86 ``$8``.
* ``push`` -> instruction qui permet de manipuler la pile en x86.

### **Passage 64 vs 32** :

* Passer en 64 bits ca implique d'avoir des pointeurs 2 fois plus gros donc pas forcément nécessaire.

Au dessus de la pile (en mémoire) il y a les variables d'environement. 

### **ABI (application binary interface)**\
-> convention qui dit que tout paramètre d'une fonction doit être mis dans la pile
-> en vrai convention dit que on met les n 1ere varibles d'une fonction dans les registres et les suivants dans la pile (3 en MIPS)
-> retour fonction dans ``%rax``

### **Epilogue d'une fonction** :
```s
add $8, %rsp #permet de décrémenter la pile pour accéder à l'adresse de retour de la fonction
ret
```

### Conclusion

* On remarque qu'il y a des variables jamais mis en mémoire -> pas d'impact en mémoire (genre var d'appel de funct)
* Variable mis en mémoire -> on n'a pas choisi l'adresse, elle est mise dans la pile 
* Les OS implémente ajd la **randomization** de la pile pour éviter les **buffer overflow**
* Le compilateur **NE DECIDE PAS DES ADRESSES** c'est l'OS qui s'en charge
* Quand une fonction est appelée, elle a sa propre pile avec son propre contexte

Variable locale n'est pas accessible à une autre fonction justement psq chaque function/programme a son propre contexte/pile.

**Thread = une pile + un rsp**

# Autre type d'adresse

**PILE != TAS**

Les variables alloués dans le **tas** -> allocation dynamique en utilisant ``malloc`` qui appartient à la ``glibc``.

```c
int main(){
    int a*;
    a = malloc(20);
}
```
a stocke un pointeur sur le tas.\
**a se situe dans la pile**.

* fragmentation interne : on alloue plus que ce qu'on utilise
* fragmentation externe : miette aux milieux de 2 allocations

Remarques : 
* librairie gère le tas
* prologue et épilogue gèrent la pile
* **le compilo ne connait pas non plus les adresses du tas**

### .o

* Les adresses ne sont pas défini dans le .o, elles sont remplacées lors de l'édition de lien. En effet si deux programmes initialise une variable globale n et qu'on génère les .o.
Ils pourraient très bien placer deux variables à la même adresse et ca ferait des conflits. C'est pour ca que les adresses sont mises uniquement au link
* on met donc des labels à la place des adresses, ces labels seront remplacé lors de l'édition de lien

## Executable

Créer les adresse en dur.

## Colisions d'adresses

Pour éviter collisions d'adresses on utilise la mémoire virtuelle. En effet deux binaires identiques ayant les même adresses si on les executes deux fois ca peut faire de la merde si on a que de la mémoire physique.


Si on a un malware, il peut très bien forcer une case mémoire à une adresse. Avec la mémoire virtuelle pas de pb. Donc ajoute aussi de la **sécurité**.

# MMU

RAM = tout ensemble de mémoire adressable directement par le CPU.\
Alors que disque sont pas adressables directement par le CPU, il faut passer par un controleur.

La MMU permet de traduire les adresses virtuelles en adresse physique via une table de traduction. Les adresses recues par la MMU sont celles choisies à **l'édition de lien**. **CPU ne voit que des adresses virtuelles.**

# Les pages

Page vs segment :\
Page = taille fixe\
-> Avantage : pas de fragmentation\
-> Désavantage : On ne peut pas dire à quoi sert tel ou tel page\
Segement = taille variable\
**Pour désactiver les segments, linux fait un gros segment de 0X00000000 à 0XFFFFFFFF.**

Il y a une table de traduction des pages et pas des adresses.\
Un emplacement de page est l'endroit ou l'on met les données alors que la page contient les données.

Comme la taille d'une page est fixe on défini l'adresse d'une page avec :
* INDEX     : numéro de la page
* OFFSET    : emplacement dans la page, décalage par rapport au début de la page


Lors d'une traduction avec la table des pages, seul l'INDEX est modifié.

**Défaut de page** -> pas de traduction, génère une exceptions

Le registre CR3 stocke l'adresse de la table des pages. Il est situé dans l'espace physique. Si c'était une adresse virtuelle, la MMU decoderait cette adresse en utilisant CR3 mais ca marcherait pas. **BREF CR3 CONTIENT UNE ADRSSE PHYSIQUE, IL EST DANS LE CPU (C'EST COMME UN REGISTRE CSR)**.


# Table de traduction

Adresse 0x2000 -> C'est la page numéro 2
Adresse 0x12000 -> C'est la page numéro 18

Si on veut écrire la traduction d'une adresse dans la table des pages il faut faire gaffe car l'adresse du store va être traduite.\
Il faut anticiper la traduction en mettant un truc virtuelle qui va être traduit.

Une page = 4K=2^12
KiB = bail pour pas confondre kilo et puis les conneries avec 1024 et pas 1000 octets

Traduire par page et pas par adresse permet de gagner un peu de place dans la table de traduction mais c'est pas suffisant.


## Example and explanation

Translate virtual adress to physical :\
Adresse virtuel de taille n, donc 2^n adresses possibles\
Page size : m octets. -> 2^m adresses dans une page.\
Donc il y a 2^n/2^m=2^a pages.

Il faut donc a bits pour déterminer le numéro de la table des pages. Les bits restant seront pour choisir l'offset.

example :
Virtual adresse : 16 bits, donc 2^16 adresses
Une page = 4ko = 4*2^10=2^12
Donc il faut 2^4 pages, donc 4 bits d'index et 12 bits d'offset.

Imaginons que la table des pages est à l'adresse 0x1000.
Le CPU envoie 0x04010, cela signifie que l'on veut accéder à la 4ème entrée de la table des pages. L'adresse de base de la table des pages est 0x1000, donc la MMU va traduire l'adresse en 0x1020, car on adresse à la 4ème entrée : 0x1000 + 4*0X8 = 0x1020.

Imaginons que la case 0x1020 de la table des pages contienne 0x3000, alors l'adresse physique accédé sera 0x3010

# Table des pages et découpage d'adresses

On redécoupe l'adresse d'une page :
* OFFSET : 12 bits
* INDICES : 36 bits (4 trucs de 9 bits)
* EXTENSION DE SIGNE : 16 bits

La table des pages est décomposée en 4 niveaux.

Indice sur 9 bits psq on a une page qui fait $2^{12}$ octets et une case fait 8 octets (1 mot 64 bits). Du coup on a $2^9$ cases, d'où les 9 bits.

D'où le 2^48 pages virtuelles possibles.
# Segments :
* .BSS :variables globales non initialisées ou initialiser à 0
* .text : le code (binaire des inst)
* .data : variables globales


# Performance et traduction d'adresse

Pour accéder une donnée il faut 5 accès mémoires.
TLB -> cache qui stocke la traduction des pages

# Exceptions

Une exception ne conduit pas forcément au crach, ca appelle l'OS qui parfois retente l'opération.\
Quand on fait un malloc c'est la glibc qui donne les adresses, **ce n'est pas l'OS**.

malloc() appelle mmap qui gère uniquement du mapping. Le mmap() ne fait pas l'allocation en RAM.\
mmap() autorise seulement à avoir la plage d'adresse.\

Avant mmap() mappait des adresses virtuelles en adresse physique, de nos jours il ne les alloue pas vraiment, il fait juste un mapping. Sil mapp une page qui n'est pas dans la table des pages, cela va en faite généré une exception.

Le gestionnaire d'exception de page fault va être appelé et c'est lui qui va en faite alloué l'adresse au programme. Car une adresse n'est en faite jamais allouée tant qu'elle n'est pas utilisée.

mmap ne rempli pas la table des pages, il rempli juste ``tree_contains`` qui contient les adresses virtuelle valides.

``allocate_page()`` : va chercher une page physique disponible.

**Avant mmap faisait le allocate_page() et le map_page().**

malloc n'appelle pas l'OS il utilise juste les adresses gérée par la **GLIBC**, si la GLIBC n'a plus de place elle va faire un appel système

Au démarrage la glibc fait un mmap gigantesque, ca ne pose pas pb puisque en pratique les adresses sont alloués que quand on les utilises.

En faite quand on fait un malloc gigantesque ca sert a rien psq c'est alloué que quand on l'utilise.

Ce mécananisme ou on appelle le kernel et c'est lui qui autorise l'allocation permet d'éviter d'allouer n'importe quoi.


**Mémoire virtuelle et allouée pdt appel de mmap tandis que la mémoire physique est alloué quand la page est utilisée.**

# Fork utilise même type de mécanisme que mmap

Quand on fait un fork on duplique tout le processus, quand on a des données qui sont que en lecture ca sert à rien de les dupliquer. On aimerait donc dupliquer que quand on modifie. L'idée c'est de protéger la page en écriture, donc si le père ou le fils veulent écrire dedans le kernel reprend la main et duplique la page.\
**Technique num 1 en kernel c'est de provoquer une erreur pour reprendre la main.**

# Evaluation paresseuse

L'évaluation paresseuse n'est affectée qu'au moment ou on en a besoin.

Genre :
```c
if(func1() && func2())
```
Le && n'est fait que si **func1()** est vrai.

# Appel systeme et kernel

L'interruption numéro 80 est l'appel syst. \
Les adresses du noyau (sur du 32 bits) sont en général du 3eme au 4eme Go des adresses virtuelles. Les variables locales du kernel doivent être les même dans tous les processus, cela implique donc que les adresses virtuelles soit mappés au même endroit dans la mémoire physique, peut importe le processus.

On passe quand même par des adresses virtuelles car le CPU manipule que des adresses virtuelles donc ca foutrait la merde de les mettre en physique même si elles sont mappés au meme endroit.

# Remarques && others

## Exercice

TLB peut stocker 4 traductions.
On a une fonction access_mem() qui génère deux accès à l'adresse addr.


## Remarque

* **garbage collector** -> checker ce que c'est
* ``git diff --staged`` compare le contenu de l'INDEX et du HEAD (ce qui est sur le dernier commit commité, donc + sur l'index)
* Checker ce que c'est un **fork**
* Checker ce que c'est un buffer **overflow**
* Checker ce que c'est BSS (variable globale non initialisé)
* En x86 :\
rdi = 1er paramètre de call, rsi = 2eme


* La pile et le tas ne peuvent jamais se collisionner car entre les deux espaces il y a les librairies dynamiques.
* Les piles prévues par le noyaux sont disjointes 
* malloc implique de devoir faire **un free qui est couteux**, alors que si on alloue dans la pile il suffit de sortir de la fonction pour libérer la pile
* **alloca** permet d'allouer dans la pile, c'est un genre de malloc dans la pile -> **POSE UN GROS PB** :\
Les variables locales n'ont pas de nom, ni d'adresse, elles ont des adresses relatives.\
Si on ajoute un tableau dans la pile avec alloca, ca décale la pile et donc on ne peut plus savoir ou se trouvent nos variables. Pour éviter ce pb le registre bp (ou rbp) stocke l'adresse de base de la pile.

* Le c++ permet de créer des objets dans la pile, alors qu'en java il n'y a que des refs vers des objets dans la pile -> explique pq c++ est bcp + perf que java

* ``ul`` : unsigned long
* différence en sécurité et sureté :
sureté = ne crash pas
sécu = pas de truc malicieux
## Threads


* Utile : https://sites.uclouvain.be/SyllabusC/notes/Theorie/Threads/threads2.html

* Les **threads** sont différents des **process**. 
Un thread est un contexte d'exécution, tandis qu'un processus est un ensemble de ressources associées à un calcul.\
Un process peut avoir un ou plusieurs threads.

En effet les ressources associés à un process incluent les pages mémoires (tous les threads dans un process ont une meme vue depuis la mémoire), file descriptor, et sécurité.
* Lors de sa création, un thread prend comme argument les arguments de la fonction mutlithreadé. Ces arguments sont donc placé en pile

## Questions

* Ce matin Sopena a dit qu'une variable locale n'était pas accessible depuis un autre thread. Mais la pile est commune à tous les threads sur un même coeur. Donc si un thread i écrit une donnée à une adresse A, un second thread peut très bien accéder à B en modifier la valeur dans le stack pointer pour accéder à A ?
* OS gère les sp pour les threads ? -> Oui, nombre fini de thread
* Diff processus/thread ?
https://www.guru99.com/difference-between-process-and-thread.html#:~:text=Process%20means%20a%20program%20is,takes%20less%20time%20for%20creation.
* Pile continue entre plusieurs fonctions ?
* Pile justaposée pour main et appel fonction ? On pourrait dépiler suffisamment pour accéder aux var locales du main par ex -> On peut le faire, juste ca pue la merde

