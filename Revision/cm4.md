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

Ce qu'on peut retenir :
* Si la ligne est SHARED, lorsqu'un autre cache fait un **set** alors la ligne est invalidée
* Si la ligne est MODIFIED, lorsqu'un autre cache fait un **set** la ligne est invalidée
* Si la ligne est MODIFIED, lorsqu'un autre cache fait un **get** la ligne devient SHARED

# Architecture NUMA