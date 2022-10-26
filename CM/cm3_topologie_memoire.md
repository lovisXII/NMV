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

# Remarque

* Comment sont adressés les tab à deux dimensions : On dirait qu'il ne manipule un seul tableau. Si on utilise tab[i][j] il va juste alloué i+j case.
* OS vs kernel :
    * la glibc, le gestionnaire de fenetre, des services dans un micro noyaux -> fait parti de l'os mais pas du noyau. Un OS c'est juste une abstraction du matériel avec une API
    * noyau : tout le code qui s'exéute en mode s
