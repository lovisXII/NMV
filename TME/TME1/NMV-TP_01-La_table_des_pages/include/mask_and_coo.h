#define PGT_ADRESS_MASK 		0xFFFFFFFFFF000

#define PGT_IS_VALID(p)	        p & 0x1
#define PGT_IS_HUGE_PAGE(p)		(p & 0x80)
#define PGT_ADRESS(p)			(p & PGT_ADRESS_MASK)



#define PGT_PML4_INDEX(vaddr) 	((vaddr >> (12+9*3)) 	& 0x1FF)
#define PGT_PML3_INDEX(vaddr) 	((vaddr >> (12+9*2)) 	& 0x1FF)
#define PGT_PML2_INDEX(vaddr) 	((vaddr >> (12+9)) 		& 0x1FF)
#define PGT_PML1_INDEX(vaddr) 	((vaddr >> (12)) 		& 0x1FF)

#define PHYSICAL_POOL_PAGES  	64
#define PHYSICAL_POOL_BYTES  	(PHYSICAL_POOL_PAGES << 12)
#define BITSET_SIZE          	(PHYSICAL_POOL_PAGES >> 6)


#define PGT_W_P_U_MASK	0x11 // mask pour User+Writtable+valid

