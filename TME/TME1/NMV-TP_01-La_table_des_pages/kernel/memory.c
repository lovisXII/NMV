#include <memory.h>
#include <printk.h>
#include <string.h>
#include <x86.h>


#define PHYSICAL_POOL_PAGES  	64
#define PHYSICAL_POOL_BYTES  	(PHYSICAL_POOL_PAGES << 12)
#define BITSET_SIZE          	(PHYSICAL_POOL_PAGES >> 6)

#define PGT_VALID_MASK 			0x1
#define PGT_ADRESS_MASK 		0xFFFFFFFFFF000
#define PGT_HUGEPAGE_MASK 		0x80

#define PGT_IS_VALID(p) 		(p & PGT_VALID_MASK)
#define PGT_IS_HUGE_PAGE(p)		(p & PGT_HUGEPAGE_MASK)
#define PGT_ADRESS(p)			(p & PGT_ADRESS_MASK)

#define PGT_PML4_INDEX(vaddr) 	((vaddr >> (12+9*3)) 	& 0x1FF)
#define PGT_PML3_INDEX(vaddr) 	((vaddr >> (12+9*2)) 	& 0x1FF)
#define PGT_PML2_INDEX(vaddr) 	((vaddr >> (12+9)) 		& 0x1FF)
#define PGT_PML1_INDEX(vaddr) 	((vaddr >> (12)) 		& 0x1FF)


#define PGT_W_P_U_MASK	0x11 // mask pour User+Writtable+valid



extern __attribute__((noreturn)) void die(void);

static uint64_t bitset[BITSET_SIZE];

static uint8_t pool[PHYSICAL_POOL_BYTES] __attribute__((aligned(0x1000)));


paddr_t alloc_page(void)
{
	size_t i, j;
	uint64_t v;

	for (i = 0; i < BITSET_SIZE; i++) {
		if (bitset[i] == 0xffffffffffffffff)
			continue;

		for (j = 0; j < 64; j++) {
			v = 1ul << j;
			if (bitset[i] & v)
				continue;

			bitset[i] |= v;
			return (((64 * i) + j) << 12) + ((paddr_t) &pool);
		}
	}

	printk("[error] Not enough identity free page\n");
	return 0;
}

void free_page(paddr_t addr)
{
	paddr_t tmp = addr;
	size_t i, j;
	uint64_t v;

	tmp = tmp - ((paddr_t) &pool);
	tmp = tmp >> 12;

	i = tmp / 64;
	j = tmp % 64;
	v = 1ul << j;

	if ((bitset[i] & v) == 0) {
		printk("[error] Invalid page free %p\n", addr);
		die();
	}

	bitset[i] &= ~v;
}

/*
 * Memory model for Rackdoll OS
 *
 * +----------------------+ 0xffffffffffffffff
 * | Higher half          |
 * | (unused)             |
 * +----------------------+ 0xffff800000000000
 * | (impossible address) |
 * +----------------------+ 0x00007fffffffffff
 * | User                 |
 * | (text + data + heap) |
 * +----------------------+ 0x2000000000
 * | User                 |
 * | (stack)              |
 * +----------------------+ 0x40000000
 * | Kernel               |
 * | (valloc)             |
 * +----------------------+ 0x201000
 * | Kernel               |
 * | (APIC)               |
 * +----------------------+ 0x200000
 * | Kernel               |
 * | (text + data)        |
 * +----------------------+ 0x100000
 * | Kernel               |
 * | (BIOS + VGA)         |
 * +----------------------+ 0x0
 *
 * This is the memory model for Rackdoll OS: the kernel is located in low
 * addresses. The first 2 MiB are identity mapped and not cached.
 * Between 2 MiB and 1 GiB, there are kernel addresses which are not mapped
 * with an identity table.
 * Between 1 GiB and 128 GiB is the stack addresses for user processes growing
 * down from 128 GiB.
 * The user processes expect these addresses are always available and that
 * there is no need to map them explicitely.
 * Between 128 GiB and 128 TiB is the heap addresses for user processes.
 * The user processes have to explicitely map them in order to use them.
 */



void map_page(struct task *ctx, vaddr_t vaddr, paddr_t paddr)

{
	uint64_t *p = (uint64_t *) ctx->pgt; //adresse physique du 1er niveau de la table de pages
	paddr_t tmp;
	
	// On part du principe de PML4 existe et est correct

	uint64_t pml4_index = PGT_PML4_INDEX(vaddr); 
	uint64_t pml3_index = PGT_PML3_INDEX(vaddr); 
	uint64_t pml2_index = PGT_PML2_INDEX(vaddr); 
	uint64_t pml1_index = PGT_PML1_INDEX(vaddr); 
	
	printk("PML4 : 0x%lx\n", pml4_index);
	printk("PML3 : 0x%lx\n", pml3_index);
	printk("PML2 : 0x%lx\n", pml2_index);
	printk("PML1 : 0x%lx\n", pml1_index);


	// Allocation de PML3 au cas où il ne serait pas valide :

	if(!PGT_IS_VALID(p[PGT_PML4_INDEX(vaddr)])) 
	// vérifie que l'adresse de la PML 4 est valide 
	{
		printk("PML3 not initialy valid, we do alloc it\n");
		// On va récupérer une page valide affectée via la fonction identité :

		tmp = alloc_page(); 
		
		// On initialise la page récupérée à zero
		
		memset((void*) tmp, 0, 4096) ;

		// On va vouloir écrire dans l'adresse récupérée  :
		
		// p contient l'adresse physique du 1er niveau de la table des pages (PML4)
		// p[PGT_PML4_INDEX(vaddr)] correspond à l'index dans la page de l'adresse
		// virtuelle envoyée

		// Allocation de PML3

		p[PGT_PML4_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}
	else{

		printk("PML3 was already allocated\n");
	}
	p = (uint64_t *) PGT_ADRESS(p[PGT_PML4_INDEX(vaddr)]); // On récupère l'adresse de PML3

	if(!PGT_IS_VALID(p[PGT_PML3_INDEX(vaddr)])){
		printk("PML2 not initialy valid, we do alloc it\n");
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);
		p[PGT_PML3_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}


	p = (uint64_t *) PGT_ADRESS(p[PGT_PML3_INDEX(vaddr)]); // On récupère l'adresse de PML2

	if(!PGT_IS_VALID(p[PGT_PML2_INDEX(vaddr)])){
		printk("PML1 not initialy valid, we do alloc it\n");
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);
		p[PGT_PML2_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}


	p = (uint64_t *) PGT_ADRESS(p[PGT_PML2_INDEX(vaddr)]); // On récupère l'adresse de PML1
	if(!PGT_IS_VALID(p[PGT_PML1_INDEX(vaddr)])){
		printk("PML0 not initialy valid, we do alloc it\n");
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);
		p[PGT_PML1_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}

}
void load_task(struct task *ctx)
{
}

void set_task(struct task *ctx)
{
}

void mmap(struct task *ctx, vaddr_t vaddr)
{
}

void munmap(struct task *ctx, vaddr_t vaddr)
{
}

void pgfault(struct interrupt_context *ctx)
{
	printk("Page fault at %p\n", ctx->rip);
	printk("  cr2 = %p\n", store_cr2());
	asm volatile ("hlt");
}

void duplicate_task(struct task *ctx)
{
}
