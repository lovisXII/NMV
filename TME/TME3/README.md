# Introduction

Dans ce TP on va chercher à implémenter une shadow table pour l'hyperviseur janus.

On va utiliser le système d'exploitation Rackdoll déjà utilisé lors du TP 1.

La gestion de la mémoire dans Janus se fait de la manière suivante :
* Au lancement de l'hyperviseur, celui ci crée un fichier qui servira de mémoire physique à la VM. Ce fichier est appelé **backend mémoire**
* L'hyperviseur donne accès à ce backend mémoire en mappant le fichier en mémoire virtuelle.

Dans Janus, l'hyperviseur et le système invité partagent le même espace d'adressage virtuel au sein d'un unique processus en mode user.

# Exercice 1 :

# Question 1 :

```
Quel mapping l’hyperviseur doit-il mettre en place pour donner l’impression au syst`eme invit ́e qu’il acc`ede
directement `a la m ́emoire physique ?
```

Lors du boot la table des pages n'étant pas encore remplie il faut mettre en place un mapping par identité pour donner l'impression au système invité qu'il accède directement à la mémoire physique.

# Question 2 :

```
Le fichier monitor/shadow.c contient le squelette de la fonction void set flat mapping(size t
ram) qui donne acc`es au syst`eme invit ́e `a la m ́emoire physique de ram octets via un mod`ele plat. Impl ́ementez
la fonction set flat mapping. Vous disposez pour cela de la fonction void map page(vaddr t vaddr,
paddr t paddr, size t len) qui mappe l’adresse virtuelle vaddr sur l’adresse physique paddr du
backend m ́emoire pour une taille de len octets
```
```c
* +----------------------+ 0xfffffffffffffff
 * | Monitor              |
 * | (forbidden accesses) |
 * +----------------------+ 0x700000000000
 * | Guest                |
 * +----------------------+ 0x100000000
 * | Monitor              |
 * | (forbidden accesses) |
 * +----------------------+ 0x80000000
 * | Guest                |
 * +----------------------+ 0x100000
 * | Monitor              |
 * | (trapped accesses)   |
 * +----------------------+ 0x0
 * 
```

Une manière simple de faire serait de faire un gros mmap sur les adresses de l'invité Guest. L'allocation étant une allocation paresseuse, faire un gros mmap ne pose pas problème.

```c
void set_flat_mapping(size_t ram)
{
	uint64_t bottom_guest_1 = 0x100000;
	uint64_t top_guest_1 	= 0x80000000;

	uint64_t bottom_guest_2 = 0x100000000;
	uint64_t top_guest_2 	= 0x700000000000;
	map_page(bottom_guest_1, bottom_guest_1, top_guest_1-bottom_guest_1);
	map_page(bottom_guest_2, bottom_guest_2,top_guest_2-bottom_guest_2);
}
```

# Exercice 2 :

Un des premiers mode de communication d'un OS sur un PC est l'écran CGA. Le système peut afficher du texte sur cet écran en écrivant dans une zone située à l'adresse 0xb8000. 

Sur une machine physique, cette adresse pointe sur la mémoire vidéo et les caractères écrits dans cette zone sont affichés à l'écran.

On va chercher à émuler l'accès à la mémoire CGA. 

# Question 1 : 
```
Avec un mod`ele m ́emoire plat, `a quelle adresse virtuelle le syst`eme invit ́e doit-il  ́ecrire pour atteindre l’adresse
physique 0xb8000 ?
```
Si on utilise un système à mémoire plat, le système invité doit écrire à l'adresse virtuelle 0xb8000 pour accéder à l'adresse physique 0xb8000.

# Question 2 :

```
́Etant donn ́e le mod`ele m ́emoire de Janus, que se passe-t-il quand le syst`eme invit ́e essaye d’afficher un
caract`ere sur l’ ́ecran CGA ?
```
Ca déclenche un trap étant donné que l'on cherche à écrire dans la zone "trapped" entre les adresses 0x0 et 0x100000.

# Question 3 :

```
Completez le corps la fonction trap write pour  ́emuler l’affichage de caract`eres sur un  ́ecran CGA. Vous
pouvez pour cela utiliser la fonction void write vga(uint16 t off, uint16 t val) qui marque le
caract`ere val dans l’ ́ecran virtuel CGA `a la position off (la position 0 indique le caract`ere en haut `a gauche
de l’ ́ecran
```

On va chercher à écrire la donnée val dans toutes les cases du CGA entre addr et size.
La fonction ``write_vga()`` prends en entrée l'index du CGA à écrire et la valeur à écrire.

On va donc écrire l'ensemble des adresses entre addr et addr + size en vérifiant que cette range fait bien partie de l'intervalle du terminal CGA.

```c
int trap_write(vaddr_t addr, size_t size, uint64_t val)
{
	if(addr >= 0xb8000 && addr <= (0xb8000 + VGA_SIZE*2))
	{
		size_t start = (addr - 0xb8000) / 2 ;
		for(size_t i = start; i < start + (size / 2); i++)
			write_vga(i, val);
		return 1;
	}
	else
	{
		fprintf(stderr, "trap_write unimplemented at %lx\n", addr);
		exit(EXIT_FAILURE);
	}
}
```

# Exercice 3 :

# Question 1 :

```
Impl ́ementez une nouvelle fonction, que nous appellerons parse page table, qui parcours la nouvelle table des pages mise en place par le syst`eme invit ́e. Cette fonction affichera quels mappings virtuel →physique
sont indiqu ́es par la table des pages. On pourra, pour cette question, s’aider des r ́eponses aux questions du
TP m ́emoire virtuelle. Vous invoquerez cette fonction depuis set page table.
```

# Question 2 :

```
Pour des raisons pratiques, on vous propose d’enregistrer chaque mapping virtuel →physique indiqu ́e dans
la table des pages dans un tableau. On d ́efinit un mapping par trois propri ́et ́es :
— l’adresse virtuelle initiale
— l’adresse physique initiale
— la taille
Pour ce TP, on suppose qu’une table des pages n’indiquera jamais plus de 64 mappings. Modifiez les fonctions
set page table et parse page table pour qu’elles enregistrent les mappings de la table des pages mise
en place au lieu de les afficher
```
# Rappel :
* Hyperviseur type 1 = processus en mode user sur la machine hôte
* Si jamais l'hyperviseur tente d'accéder à une addresse virtuelle (donc addresse physique de la fausse mmu ) déjà utilisée par le système hote, on peut simplement déplacer les adresses virtuelles du système invité
* On accède jamais directement à un fichier, c'est le buffer cache qui va chercher les blocs des fichiers et qui les met en mémoire 