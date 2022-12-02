**ATTENTION : ON NE DIT PAS CACHE L3 MAIS LLC (LAST LEVEL CACHE)**
# Introduction

Tout ce qu'on a vu jusqu'a maintenant c'est la base de la virtualisation.

Ce cours est la continuité de géré la mémoire et on va s'interesser au cache.

# Hiérarchie mémoire

-> caches (1 à 4 cycles)
-> SDRAM (150 cycles)

Comment transformer la localité spatiale en localité temporelle ? -> Si on fonctionne par paquet de données.

La différence entre la ram et le disque :
* vitesse accès (mais permet pas de def)
* taille        (mais permet pas de def)
* volatile/non volatile -> mais faux maintenant
* **LA RAM EST DIRECTEMENT ADRESSABLE PAR LE CPU -> SEUL VRAIE DIFF DE NOS JOURS** 

hit_rate = nbre_hit / nbre_access

Avoir un bon hit_rate est un mixte logiciel et matériel.

**ACHTUNG : LE CACHE NE MANIPULE PAS DES PAGES, CE SONT DES ADRESSES**

**Les caches = mécanismes d'indexation et d'éviction**

Quand on fait un char* tab et qu'on accède tab[0], il est à l'adresse tab, tab[1] est à l'adresse tab + 1 octet.\
Donc dans une case mémoire sur une archi 64 bits, on peut stocker 8 char.

-> cache associatif (n index pour différent tag)\
set = ligne

Visiblement les caches considérés ici sont uniquement write back. 

Le cache L3 est appelé le LLC (**last level cache**) -> dernier niveau de cache avant l'accès à la mémoire.

# Multi coeur

```c
volatile
```
Sert à informer au compilo de pas mettre la donnée dans un registre.\
Par exemple si on a deux coeurs avec deux threads.\
Un thread fait ``si var = true -> sleep``.\
Et que le 2eme fait ``var = false``.\
Bah si les variables sont en registre ca boucle indéfiniment.

Slide 27 -> (0,1) pas possible car ca veut dire qu'on aurait exécuté B=1 de on_core_0 avant A=1 et on est sur du séquentiel donc ca ne peut pas arriver. (**Cohérence séquentielle**)

# Protocole MESI : une implémentation RW-lock

Il y a un automate par cache. Chaque cache associe un état à une ligne de cache.

Si on a pas la valeur (qu'on fait un MISS) on va get la valeur dans le cache de nv sup ou dans la RAM. Si on fait une écriture (on fait set) on passe dans l'état WRITE.

On a plusieurs lecteurs et un seul écrivain.  
 -> il a parlé de **broadcast** ou jsp quoi\
Les modèles de cohérence ne sont pas universel.

De nos jours **MOESI** et plus **MESI**.

# Multicoeur et contention matérielle

Problème des archis many : goulot d'étranglement sur le bus (cad le débit, pas la latence)
**Coloration de cache : imposer que le coeur i ait accès à que 10% du cache LLC par ex -> ca revient à partager le cache en software**

Pour palier au problème de contention, il suffit de partitionner la mémoire entre tous les clusters MAIS faire ca impose des problèmes de cohérence.\
On split la ram sur chacun des clusters mais on veut quand même avoir un interconnect qui permet à tous les coeurs d'accéder à n'importe quel segment de ram.

On parle de noeuds **NUMA** (non uniform memory access).

**Noeud Numa ?**

Dans le directory on va donc ajouter un bit pour donner une information sur le noeuds dans lequel se trouve la RAM que l'on veut accéder. **(page 20/30)**\
Etat de la ligne dans MOESI est aussi dans l'index

LE DIRECTORY C'EST UN TRUC A PART, FAIT PAS PARTI DE LA MMU ASKIP.

Sur les serveurs, les cm permettent de ne pas exposé le NUMA ie de donner l'impression qu'il y a un seul noeud. Sur certains serveurs on peut avoir un algo qui se charge de répartir les charges au nv hardware et d'exposer à l'utilisateur un seul noeud (utilisateur = neoud), ie que l'ensemble de la ram est équivalent pour l'OS. C'est le matos qui va géré la répartition, on peut lui dire ce qu'on veut il le respectera pas.
**-> C'est masqué**
Le scheduleur peut migrer les threads sur différents noeuds pour équilibrer les accès.

# Placement de données : l'interleaving

La répartition de la charge mémoire est le facteur le plus important pour les performances NUMA
-> L'idée c'est de répartir équitablement les données sur tous les noeuds pour équilibrer la charge.
```c
void worker ( void * area )
{
    do_some_stuff ( area );
}
void main ( void )
{
    char * area = mmap ( NULL , TASK * LEN , ...);
    int i;
    initialize_area ( area );
    for (i = 0; i < TASK ; i ++)
        launch_worker ( worker , area + i * LEN );
    wait_workers ();
    use_result ( area , TASK * LEN );
}
```
-> dans cet exemple tous les threads vont utiliser l'ini du main, il faut utiliser NUMA 

# Placement de données : first touch

La tache qui alloue un espace en mémoire est souvent la tache qui utilise cet espace.\
L'allocation mémoire est paresseuse, si un thread alloue la donnée, il n'y un mapping physique/virtuelle que la première fois que la donnée est utilisée.


```c
void worker ( void * area )
{
    initialize_area ( area );
    do_some_stuff ( area );
}
void main ( void )
{
    char * area = mmap ( NULL , TASK * LEN , ...);
    int i;
    for (i = 0; i < TASK ; i ++)
        launch_worker ( worker , area + i * LEN );
    wait_workers ();
    use_result ( area , TASK * LEN );
}
```
-> dans cet exemple chaque thread va manipuler une zone mémoire, pas la peine d'exploiter NUMA 

# Remarque

* Comment sont adressés les tab à deux dimensions : On dirait qu'il ne manipule un seul tableau. Si on utilise tab[i][j] il va juste alloué i+j case.
* OS vs kernel :
    * la glibc, le gestionnaire de fenetre, des services dans un micro noyaux -> fait parti de l'os mais pas du noyau. Un OS c'est juste une abstraction du matériel avec une API
    * noyau : tout le code qui s'exéute en mode s
* loi d'ahmdal