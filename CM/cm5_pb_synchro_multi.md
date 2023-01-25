# Problème de synchro multi coeur

Les pbs de synchro mono et multi sont les même mais les solutions ne sont pas les même.

En mono si on a du multi thread ca peut aussi foutre la merde. Solution **MUTEX**

CAS (compare and swap) & TestAndSet.

RCU (truc de synchro comme CAS) -> on utilise une structure qui contient toutes les données. Avantage du RCU est de marcher sur plusieurs données


# Remarques
* linux est pré-emptif depuis version 2.6