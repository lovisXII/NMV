#include <memory.h>
#include <printk.h>
#include <string.h>
#include <x86.h>
#include <mask_and_coo.h>


// #define DEBUG_map_page
// #define DEBUG_load_task


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
 * +----------------------+ 
 * 
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
// We're getting a task ctx and we want to mapp
// the virtual vaddr on paddr in
// its space addresses. 
{
	uint64_t *p = (uint64_t *) ctx->pgt; // we're getting the first level page table paddr
	
	#ifdef DEBUG_map_page
	printk("Allocating virtual adresse : 0x%lx to physical adresse 0x%lx\n",vaddr,paddr);
	printk("PML4 of the task is located at 0x%lx\n",p);
	#endif
	paddr_t tmp;
	
	// We suppose that the physical of PML4 is correct
		
	#ifdef DEBUG_map_page
	printk("PML4 : 0x%lx\n", PGT_PML4_INDEX(vaddr));
	printk("PML3 : 0x%lx\n", PGT_PML3_INDEX(vaddr));
	printk("PML2 : 0x%lx\n", PGT_PML2_INDEX(vaddr));
	printk("PML1 : 0x%lx\n", PGT_PML1_INDEX(vaddr));
	#endif

	// Checking if PML3 is valid
	// If it's not valid, we need to allocate it :

	if(!PGT_IS_VALID(p[PGT_PML4_INDEX(vaddr)])) 
	{
			
		#ifdef DEBUG_map_page
		printk("PML3 not initialy valid, we do alloc it\n");
		#endif
		// We're getting a new page which is not yet used
		// We'll use the adress of this new page as starting point for PML3

		tmp = alloc_page(); 
		
		// We initialize all the data in the page to 0
		
		// There is 512*8 bytes -> 4096 bytes
		
		memset((void*) tmp, 0, 4096) ;

		
		// We write the new page allocated inside the PML4

		p[PGT_PML4_INDEX(vaddr)] = (tmp | PGT_W_P_U_MASK); 
	}
	else{
		#ifdef DEBUG_map_page
		printk("PML3 was already allocated\n");
		printk("PML3 adress is 0x%lx\n", p[PGT_PML4_INDEX(vaddr)]);
		#endif
	}
	
	// Getting the adress of PML3

	p = (uint64_t *) PGT_ADRESS(p[PGT_PML4_INDEX(vaddr)]); 

	// Checking if PML2 entry is valid, if not we
	// need to allocate it

	if(!PGT_IS_VALID(p[PGT_PML3_INDEX(vaddr)]))
	{
		#ifdef DEBUG_map_page		
		printk("PML2 not initialy valid, we do alloc it\n");
		#endif
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);

		p[PGT_PML3_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}
	else{
		#ifdef DEBUG_map_page
		printk("PML2 was already allocated\n");
		printk("PML2 adress is 0x%lx\n", p[PGT_PML3_INDEX(vaddr)]);
		#endif
	}

	// Getting the adress of PML2

	p = (uint64_t *) PGT_ADRESS(p[PGT_PML3_INDEX(vaddr)]); 
		
	#ifdef DEBUG_map_page
	printk("0x%lx is containing 0x%lx\n",p,*p);
	printk("0x%lx is containing 0x%lx\n",p+1,*(p+1));
	printk("PML2 is : 0x%lx\n", p);
	#endif

	// Checking if PML1 entry is valid, if not we
	// need to allocate it

	if(!PGT_IS_VALID(p[PGT_PML2_INDEX(vaddr)]))
	{	
		#ifdef DEBUG_map_page
		printk("PML1 not initialy valid, we do alloc it\n");
		#endif
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);
		p[PGT_PML2_INDEX(vaddr)] |= (tmp | PGT_W_P_U_MASK); 
	}
	else{	
		#ifdef DEBUG_map_page
		printk("PML1 was already allocated\n");
		printk("PML1 adress is 0x%lx\n", p[PGT_PML2_INDEX(vaddr)]);
		#endif
	}

	// Getting the adress of PML1

	p = (uint64_t *) PGT_ADRESS(p[PGT_PML2_INDEX(vaddr)]); 


	// Checking if paddr entry is valid, if not we
	// need to allocate it

	if(!PGT_IS_VALID(p[PGT_PML1_INDEX(vaddr)]))
	{	
		#ifdef DEBUG_map_page
		printk("Mapping virtual adresse 0x%lx on physical adresse 0x%lx\n",vaddr,paddr);
		#endif
		tmp = alloc_page();
		memset((void *)tmp, 0, 4096);
		p[PGT_PML1_INDEX(vaddr)] |= (paddr | PGT_W_P_U_MASK); 
	}
	else{	
		#ifdef DEBUG_map_page
		printk("[error] virtual adresse 0x%lx was already translated\n",vaddr);
		#endif
	}
}

void load_task(struct task *ctx)
// This function loads the code of a specific task inside the memory

{
	#ifdef DEBUG_load_task
		printk("Loading task\n");	
	#endif

	/*
	1st step : We need to copy PML3[1] into the new PML3[1]
	*/

	paddr_t *PML4;	
	paddr_t *PML3;	

	PML4 = ctx->pgt;
	
	// Allocating PML3 :

	PML3 = alloc_page();
	memset((void *)PML3, 0, 4096);

	// Setting up PML4[0]

	PML4[0] = (paddr_t) PML3 | PGT_W_P_U_MASK ;

	// Getting the right PML3[1] :

	paddr_t *current_cr3 = store_cr3();
	paddr_t *ptr_current_cr3 = PGT_ADRESS(current_cr3[0]); 
	PML3[1] = ptr_current_cr3[1] ;


	#ifdef DEBUG_load_task
	printk("[Debug] PML4 base address is : 0x%lx\n", PML4);
	printk("[Debug] PML3 base address is : 0x%lx\n", PML3);
	printk("[Debug] cr3 for previus task was : 0x%lx\n", current_cr3);
	#endif


	/*
	2nd step : 
	Mapping virtual address for the payload and the bss.
	Since both are contigus it's pretty easy to do the mapping. 
	*/

	#ifdef DEBUG_load_task
	printk("[Debug] load_vaddr : 0x%lx\n", ctx->load_vaddr);
	printk("[Debug] load_paddr : 0x%lx\n", ctx->load_paddr);
	#endif
	for(uint64_t i = 0; ctx->load_vaddr + i < ctx->bss_end_vaddr; i+=4096)
	{
		map_page(ctx, ctx->load_vaddr + i,ctx->load_paddr + i);
	}	
}

void set_task(struct task *ctx)
{
	load_cr3(ctx->pgt);
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
