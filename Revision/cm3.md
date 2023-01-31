# Mémoire cache en monocoeur

Blabla habituel sur les caches, zone de stockage intermédiaire, localité spatiale et temporelle...etc.
jargon :
* MISS/HIT 
* get(A) -> requete lecture bloc A, par CPU->cache/cache->MP
* set(A) -> requete écriture bloc A

Mécanisme de préfétching, pas plus couteux de transférer deux blocs qu'un seul. 

Visiblement les caches étudiés sont write back psq quand écriture fait MISS on ramène la donnée pour l'écrire dans le cache.

## **Hit rate**

* Donnée dans cache : cache hit -> faible latence
* Donnée pas dans le cache : cache miss -> forte latence

hit_rate = $\frac{nbr_hit}{nbr_access}$

Plus le hit_rate est élevé, moins le processeur stall.

## **Unité d'échange mémoire : la ligne de cache**

Ligne de cache est une donnée de taille fixe, taille invariable sur un même matériel.
* TAG
* INDEX
* OFFSET

Dans le jardon sopena :
* cache direct = direct mapping // opposition avec cache associatif
## **Exercice collision d'adresse**

* On considère un cache direct de 32Ko = 2^15
    * ligne de cache = 64 oct = 2^6

Quel est le hit rate :

```c
char *src   = (char *) 0x18000;
char *dest  = (char *) 0x10000;
char tmp;
for(int i = 0; i < 10; i++){
    tmp     = src[i];   
    dest[i] = tmp;      
} 
```
* OFFSET = 6 bits
* INDEX  = 9 bits 

Donc les deux tableaux ont le même tag -> miss à chaque fois

-> Solution cache associatif
 * cache associatif est composé d'ensembles d'emplacement``set``
 * dans un cache M-associatif, les sets sont composés de M-way

 Stratégie d'évincement :
 * LRU (least recently used) -> prodfite loca temporelle
 * NRU (not recently used)  -> approximation peu couteuse de LRU
 * MRU (most recently used) -> activée quand le hit rate devient trop faible

# Caches hiérarchiques

Pour éviter les MISS on pourrait augmenté la taille du cache, le pb c'est que c'est impossible de faire ca sans augmenter la latence. Solution -> ``ajouter des niveaux de caches``

Cache L2, L3 (appelé LLC (last level cache) si c'est le dernier nv).

En général, il y a un L1 et un L2 par coeur tandis que le L3 est partagé.

Les caches peuvent être **inclusif**, **exclusif** ou **NINE(Ni Inclusif Ni Exclusif)**

## **Caches Inclusifs**

Le cache LX est inclusif au cache LY (Y < X) ie :
* Si une ligne A est présente dans LY
* Alors la ligne est présente dans LX
-> ex : Si A présente dans L1 alors elle est présente dans L2

Quand :
*  LY miss et miss LX           -> la nouvelle ligne va dans LX et LY
*  la ligne est évincée de LY   -> pas d'effet sur LX
*  la ligne est évincée de LX   -> évincée de LY
## **Caches Exclusifs**

Les caches LX et LY sont exclusifs :*
* Une ligne A peut être présente dans au plus un des caches
* Si Y = X - 1, on dit que LX est ``victim cache`` de LY (donc cache de haut nv est victime bas nv)

Quand :
* LY miss et LX miss            -> la nouvelle ligne va dans LY (cache de plus bas niveau)
* LY miss et LX hit             -> la ligne est évincée dans LX et est copiée dans LY (on ramène vers bas niveau)
* la ligne A est évincée de LY  -> elle va dans LX

En gros dans ce cas l'idée, c'est qu'on ramène les données vers le cache de plus bas niveau. Quand les données de bas niveaux sont évincées elles remontent vers les caches de plus haut niveau
## **Caches NINE**

Caches LX et LY sont NINE
Quand :
* miss LY et miss LX    -> nouvelle ligne va dans LX et LY
* A évincé de LY        -> pas d'effet sur LX
* A évincé de LX        -> pas d'effet sur LY

## **Avantages et inconvéniants des politiques**

* inclusive 
    * gain de temps -> si data dans LX pas besoin d'aller dans LY
    * perte de place -> psq copie des données
* exclusive 
    * gain de place 
    * conso BP pour transfert entre les caches
* NINE
    * gain de place 
    * pas de gain ni de perte de temps


## **Prefetching automatique**

Les controleurs de cache tentent de prédire les accès futurs en détectant les motifs d'accès.\
Le prefetch et le CPU fonctionnent en parallèle, ca permet d'éviter les stalls

Le logiciel peut demander un prefetch -> instruction dédiée
* prefetch : prefetch simple en lecture
* prefetchw : prefetch simple en écriture

``Premature optimization is the root of all evil (or at least most of it) in programming, Donald Knuth`` -> just funny

## Eviction

Il y a aussi des instructions dédiée pour flusher le cache

## Table des pages

Les bits :
* PCD (page cache disable) : indique que les pages ne peuvent pas être mises en cache
* PWT (page write through) : indique si les lignes de la page peuvent être dirty 
