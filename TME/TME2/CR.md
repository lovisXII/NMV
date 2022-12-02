# Introduction : Topologie mémoire

L'objectif de ce TP est de découvrir les propriétés du cache d'une machine. Pour faire cela on va utiliser des codes assez simples, spécialement concu pour stresser une partie précise du matériel.

# Exercice 1

```c
char *mem = allocate_and_touch_memory(MEMORY_SIZE) ;
start_timer();
for(int i = 0; i < MEMORY_SIZE ; i += PARAM)
    mem[i] = 1;
stop_timer();
```

# Question 1 :

Initialiser une zone mémoire permet de faire un grand nombre d'accès mémoire dans une zone contigue. On va donc exploiter la localité spatiale et pouvoir mesurer comment le cache répond.

# Question 2

Elle génère les deux.

# Question 3 

Lorsque le paramètre PARAM augmente, le nombre de HIT diminue et le nombre de MISS diminue.

# Question 4

