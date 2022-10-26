# Introduction

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
Dans kernel/main.c, programmez et testez la nouvelle fonction print_pgt. Vous testerez votre nouvelle fonction en l’appellant depuis la fonction main multiboot2 située dans le fichier kernel/main.c juste avant le chargement des tˆaches utilisateur. Pour cela vous pouvez utiliser la fonction uint64 t
store cr3(void) qui retourne le contenu du registre CR3.
Si votre fonction d’affichage fonctionne correctement, vous devriez observer une table des pages avec la
structure indiquée dans l’annexe de ce sujet.
```

