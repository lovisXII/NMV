# Topologie mémoire 
# Multicoeur et cohérence de caches

Une exécution est cohérente séquentiellement s'il existe un ordre total des opérations qui préserve l'ordre des instructions de chaque coeur et qui conduit au même résultat.

En gros ca veut dire que si on a ca :
```c
void on_core_O(void){
A = 1;
B = 1;
}

void on_core_1(void){
b = B;
a = A;
}
```
Avec les deux qui s'exécutent en parallèle, bah l'état (a,b) = (0, 1) est impossible.

Les architectures actuellent cherchent à garantier la cohérence séquentielle des exécutions pour que le fonctionnement vis à vis d'un programmeur soit le plus naturel possible.

## **Protocole MESI(Modified, Exclusive, Shared, Invalid)**

* Chaque cache associe un état à chaque ligne 
    * L'état d'une ligne indique l'état global du RW-lock

Un peu comme le 2 bit saturating coutner pour la pred, on va avoir un FSM et chaque ligne du cache va stocker une valeur correspondant à cette FSM. Les états sont les suivants :

* INVALID   : 
la ligne n'est pas valide et ne contient pas de data

* SHARED    : 
Indique que cette ligne de cache peut être stockée dans d'autres caches de la machine et qu'elle n'est pas dirty. La ligne peut être rejetée (passage à l'état Invalide) à tout moment.

* MODIFIED  : 
La ligne de cache n'est présente que dans le cache actuel, et elle est dirty. Le cache doit réécrire les données dans la mémoire principale à un moment donné dans le futur, avant de permettre toute autre lecture de l'état (qui n'est plus valide) de la mémoire principale. La réécriture fait passer la ligne à l'état SHARED.

* EXCLUSIVE : 
La ligne de cache est présente uniquement dans le cache actuel, elle n'est pas dirty - elle correspond à la mémoire principale. Elle peut passer à l'état SHARED à tout moment, en réponse à une demande de lecture. Elle peut également passer à l'état MODIFIED lorsqu'on y écrit.

Quand un block est marqué comme MODIFIED ou EXCLUSIVE, les copies de ce bloc dans les autres caches sont marqués comme INVALID.

**broadcast == ligne de snoop en gros**

Ce qu'on peut retenir :
* Si la ligne est SHARED, lorsqu'un autre cache fait un **set** alors la ligne est invalidée
* Si la ligne est MODIFIED, lorsqu'un autre cache fait un **set** la ligne est invalidée
* Si la ligne est MODIFIED, lorsqu'un autre cache fait un **get** la ligne devient SHARED

## **Résumé du protocole MESI**

* Les mémoires caches assurent la cohérence séquentielle
    * Tout coeur travaille toujours sur la dernière version de chaque ligne
    * Chaque ligne est protégée par un verrou lecteur/écrivain

* Une implémentation du verrou lecteur/écrivain est le protocole MESI
    * MESI implémenté par caches INTEL

* Existe des variantes comme MOESI
    * MOESI implémenté par caches AMD

# Architecture NUMA

Problème classique : trop de clients pour une seule ressources (trop de coeur et pas assez de mémoire).

Pour palier au pb on créer des clusters appelés **noeuds (coeur + RAM)**.
Tous coeur peut accéder à toute RAM via l'**interconnect**. Mais accéder à la RAM locale est plus rapide qu'accéder à la RAM distante.

Le matériel route automatiquement les requêtes mémoire vers la bonne mémoire principale.

Afin de réduire le nombre de broadcast entre les noeuds NUMA, les caches modernes utilisent des protocoles **directory based**.

Pour chaque ligne de cache on va définir un **home node**. Le home node d'une ligne est donc déterminé par son adresse physique et correspond au noeud (cluster) d'où la data provient.
Sur chaque ligne de cache on aura donc l'état MESI + home node.

Le directory based permet de réduire le nombre de broadcast entre les noeuds NUMA. ATTENTION il ne change rien au broadcast à l'intérieur des noeuds.

Si la donnée n'est pas présente dans les caches locaux, il faut passer par le home node.

## **Placement des données : le first touch**

Intuition : la tache qui alloue un espace mémoire est souvent la tache qui utilise cet espace
Principe : allouer l'espace mémoire sur le noeud allocateur 

configuration explicite sous linux avec : ``numactl --localalloc``

Observation : la répartition de la mémoire est le facteur le plus important sur les performances NUMA
Principe : répartir équitablement les données sur tous les noeuds pour répartir la charge

configuration explicite sous linux avec : ``numactl --interleave``

```c
void worker ( void * area )
{
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

Execution first touch : rapide
Execution interleaving : lente -> mauvaise localité

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
first touch : très lent -> contention mémoire
interleaving : rapide
-> dans cet exemple tous les threads vont utiliser l'ini du main, il faut utiliser NUMA 

## Conclusion

* Pour etre efficace sur une archi NUMA, le logiciel doit maximiser :
    * une bonne répartition de la charge mémoire
    * la limitation des transferts entre noeuds
    * une bonne localité des accès

* logiciel peut agir sur placement des données et des taches :
    * placement initial des données -> sur quel noeud allouer la mémoire
    * migration des données ap allocation -> pas vu dans ce cours

* first touch :
    * allouer sur le meme noeud que la tache qui alloue
    * performant quand tache alloue sa mémoire

* interleaving :
    * répartir équitablement entre les noeuds
    * évite la contention mémoire 