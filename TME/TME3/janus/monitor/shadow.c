#include "shadow.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "memory.h"
#include "state.h"
#include "vector.h"

#define PML1(x) 	(x >> 12) & 0x1FF
#define PML2(x) 	(x >> (12 + 9 * 1)) & 0x1FF
#define PML3(x) 	(x >> (12 + 9 * 2)) & 0x1FF
#define PML4(x) 	(x >> (12 + 9 * 3)) & 0x1FF
#define ADDRESS(x)	(x & 0xFFFFFFFFFFFFF000)

#define IS_VALID(x)		x & 0x1
#define IS_HUGE_PAGE(x)	x & 0x80

#define MAX_GUEST_PROCESSES 10

struct shadow_page_table{
	paddr_t cr3;
	struct{
		vaddr_t vaddr;
		paddr_t paddr;
		size_t size;
	}mappings[64];
}shadow_page_table[MAX_GUEST_PROCESSES];

void set_flat_mapping(size_t ram)
{
	uint64_t bottom_guest_1 = 0x100000;
	uint64_t top_guest_1 	= 0x80000000;

	uint64_t bottom_guest_2 = 0x100000000;
	uint64_t top_guest_2 	= 0x700000000000;
	// map_page(bottom_guest_1, bottom_guest_1, top_guest_1-bottom_guest_1);
	// map_page(bottom_guest_2, bottom_guest_2,top_guest_2-bottom_guest_2);

	//correction :
	// Si quantité de ram est trop faible on map que ce que l'on peut sur le bas
	// Si la ram est suffisante, on va mapper tout le bas
	// Et on va mapper tout le haut dans la quantité de ram restante 
	// ram - (top_guest_1-bottom_guest_1) est la quantité de ram restante
	// après le mapping du guest_1

	if (ram < top_guest_1 - bottom_guest_1)
		map_page(bottom_guest_1, bottom_guest_1, ram);
	else{
		map_page(bottom_guest_1, bottom_guest_1, top_guest_1-bottom_guest_1);
		map_page(bottom_guest_2, bottom_guest_2, ram - (top_guest_1-bottom_guest_1));
	}
	// display_mapping();
	// fprintf(stderr, "set_flat_mapping unimplemented\n");
	// exit(EXIT_FAILURE);
}
void parse_pml_lvl(paddr_t pml, vaddr_t prefix, uint8_t lvl)
// On veut juste afficher les mappings, donc adresse
// physique et adresse virtuelle
{
	// uint64_t *pml4 = (uint64_t*) mov_from_control(3); 
	// uint64_t *pml3 = (uint64_t*) ADDRESS(pml4[PML4(vaddr)]) ; 
	// uint64_t *pml2 = (uint64_t*) ADDRESS(pml3[PML3(vaddr)]) ; 
	// uint64_t *pml1 = (uint64_t*) ADDRESS(pml2[PML2(vaddr)]) ;

	// printf("v : %lx -> p : %lx\n", vaddr, ADDRESS(pml1[PML1]));

	uint64_t *p = malloc(4096);
	vaddr_t new_prefix;
	int i;
	read_physical(p, 4096, pml);

	for(i = 0; i <512; i++){
		if(IS_VALID(p[i])){
			new_prefix = prefix + (i << (12 + (lvl - 1) *9));
			if(lvl == 1  || IS_HUGE_PAGE(p[i])){
				printf("v : %lx \tp: %lx\n", new_prefix, ADDRESS(p[i]));
			}
			else{
				parse_pml_lvl(ADDRESS(p[i]), new_prefix, lvl - 1);
			}
		}
	}
	free(p);
}
void parse_page_table(paddr_t cr3){
	parse_pml_lvl(cr3, 0, 4);
}
void set_page_table(void)
{
	paddr_t cr3 = mov_from_control(3);
	parse_page_table(cr3);
}

int trap_read(vaddr_t addr, size_t size, uint64_t *val)
{
	fprintf(stderr, "trap_read unimplemented at %lx\n", addr);
	exit(EXIT_FAILURE);
}

int trap_write(vaddr_t addr, size_t size, uint64_t val)
// 2 octets par case
{
	// display_mapping();
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
	// fprintf(stderr, "trap_write unimplemented at %lx\n", addr);
	// exit(EXIT_FAILURE);
}
