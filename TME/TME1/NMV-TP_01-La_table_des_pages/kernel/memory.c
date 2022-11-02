#include <memory.h>
#include <printk.h>
#include <string.h>
#include <x86.h>
#include <mask_and_coo.h>




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
// Allow to mapp the virtual adresse vaddr on the physical one paddr
// ctx is the current task 
{
	printk("Allocating virtual adresse : 0x%lx to physical adresse 0x%lx\n",vaddr,paddr);
	uint64_t *p = (uint64_t *) ctx->pgt; // first leval page table paddr
	printk("PML4 of the task is located at 0x%lx\n",p);
	paddr_t tmp;
	
	// We suppose that the physical of PML4 is correct
	// We get the PML index of each PML from the vaddr

	uint64_t pml4_index = PGT_PML4_INDEX(vaddr); 
	uint64_t pml3_index = PGT_PML3_INDEX(vaddr); 
	uint64_t pml2_index = PGT_PML2_INDEX(vaddr); 
	uint64_t pml1_index = PGT_PML1_INDEX(vaddr); 
	
	printk("PML4 : 0x%lx\n", pml4_index);
	printk("PML3 : 0x%lx\n", pml3_index);
	printk("PML2 : 0x%lx\n", pml2_index);
	printk("PML1 : 0x%lx\n", pml1_index);


	// We are going the check if the PMLi are allocated and if not
	// we are going to do the allocation
	
	// p[PGT_PML4_INDEX(vaddr)] :

	// p 						= adr PML4
	// PGT_PML4_INDEX(vaddr) 	= index we want to access in PML4
	// p[PGT_PML4_INDEX(vaddr)] = p + PGT_PML4_INDEX(vaddr)

	if(!PGT_IS_VALID(p[PGT_PML4_INDEX(vaddr)])) 
	// Checking if PML3 is valid
	{
		printk("PML3 not initialy valid, we do alloc it\n");

		// We get a new page which is not yet used
		// We'll use the adress of this new page as starting point for PML3

		tmp = alloc_page(); 
		
		// We initialize all the data in the page to 0
		
		memset((void*) tmp, 0, 4096) ;

		
		// We write the new page allocated inside the PML4

		p[PGT_PML4_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}
	else{

		printk("PML3 was already allocated\n");
		printk("PML3 adress is 0x%lx\n", p[PGT_PML4_INDEX(vaddr)]);
	}
	
	// Getting the adress of PML3

	p = (uint64_t *) PGT_ADRESS(p[PGT_PML4_INDEX(vaddr)]); 

	if(!PGT_IS_VALID(p[PGT_PML3_INDEX(vaddr)])){
		printk("PML2 not initialy valid, we do alloc it\n");
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);

		p[PGT_PML3_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}
	else{

		printk("PML2 was already allocated\n");
		printk("PML2 adress is 0x%lx\n", p[PGT_PML3_INDEX(vaddr)]);
	}

	// PML2 :

	p = (uint64_t *) PGT_ADRESS(p[PGT_PML3_INDEX(vaddr)]); 

	printk("0x%lx is containing 0x%lx\n",p,*p);
	printk("0x%lx is containing 0x%lx\n",p+1,*(p+1));
	printk("PML2 is : 0x%lx\n", p);
	if(!PGT_IS_VALID(p[PGT_PML2_INDEX(vaddr)])){
		printk("PML1 not initialy valid, we do alloc it\n");
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);
		p[PGT_PML2_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}
	else{

		printk("PML1 was already allocated\n");
		printk("PML1 adress is 0x%lx\n", p[PGT_PML2_INDEX(vaddr)]);
	}

	// PML1 :
	p = (uint64_t *) PGT_ADRESS(p[PGT_PML2_INDEX(vaddr)]); 

	if(!PGT_IS_VALID(p[PGT_PML1_INDEX(vaddr)])){
		printk("Mapping virtual adresse 0x%lx on physical adresse 0x%lx\n",vaddr,paddr);
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);
		p[PGT_PML1_INDEX(vaddr)] |= (paddr | PGT_W_P_U_MASK); 
	}
	else{
		printk("[error] virtual adresse 0x%lx was already translated\n",vaddr);
	}
}
void load_task(struct task *ctx)
// This function loads the code of a specific task inside the memory
{
	paddr_t pml4_adress = ctx->pgt;
	// 1st step is to copy the 
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
