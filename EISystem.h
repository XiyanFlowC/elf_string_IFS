#ifndef _XY_EISYSTEM_H_
#define _XY_EISYSTEM_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static long long EIS_POS_delta;

typedef unsigned char  byte_t;
typedef unsigned short word_t;
typedef unsigned long  dword_t;
typedef unsigned long long qword_t;

typedef dword_t ptr_t;
typedef dword_t bool_t;

static byte_t* EIS_elf    = NULL;    /*elf content*/
static qword_t EIS_elflen = 0;       /*elf length*/ 

struct permissive_blk {
    ptr_t begin;
    ptr_t end;   /*NOT including itself*/
};

struct elf_header {
	byte_t  ident[16];  /*we don't care info in it*/
	word_t  type;
	word_t  machine;
	dword_t version;
	dword_t entry;
	dword_t phoff;
	dword_t shoff;
	dword_t flags;
	word_t  ehsize;
	word_t  phentsize;
	word_t  phnum;
	word_t  shentsize;
	word_t  shnum;
	word_t  shstrndx;
};

struct section_header {
	dword_t  name;
	dword_t  type;
	dword_t  flags;
	dword_t  addr;
	dword_t  offset;
	dword_t  size;
	dword_t  link;
	dword_t  info;
	dword_t  align;
	dword_t  entsize;
};

struct program_header {
	dword_t  type;
	dword_t  offset;
	dword_t  vaddr;
	dword_t  paddr;
	dword_t  filesz;
	dword_t  memsz;
	dword_t  flags;
	dword_t  align;
};

struct rel_entry {
	ptr_t   offset;
	dword_t info;
};

static byte_t*  elf_a2p(ptr_t p_addr);
static byte_t*  elf_o2p(dword_t p_ofst);
static dword_t  elf_a2o(ptr_t p_addr);
static dword_t  elf_p2o(byte_t* p_ptr);
static ptr_t    elf_o2a(dword_t p_ofst);
static ptr_t    elf_p2a(byte_t* p_ptr);

static struct section_header* EIS_shs;
static dword_t                EIS_shs_cnt;
static struct program_header* EIS_phs;
static dword_t                EIS_phs_cnt;

static int elf_rinit(const char* elfpath)
{
	FILE* elf = fopen(elfpath, "rb");
	if(elf == NULL) 
	{
		fputs("elf_init: fatal: elf open failed.\n", stderr);
		return -1;
	}
	
	fseek(elf, 0, SEEK_END);
	EIS_elflen = ftell(elf);
	rewind(elf);
	
	EIS_elf = (byte_t*)malloc(sizeof(byte_t) * EIS_elflen);
	qword_t readlen = fread(EIS_elf, 1, EIS_elflen, elf);
	if(readlen != EIS_elflen)
	{
		fputs("elf_init: fatal: elf read failed.\n", stderr);
		return -2;
	}
	return 0;
}

static int EIS_pdmsg = 0; /*Print diagnostic message or not*/

static void (*EIS_elf_secfnd)(struct section_header* shd) = NULL;
static void (*EIS_elf_segfnd)(struct program_header* phd) = NULL;
static void (*EIS_elf_relfnd)(struct section_header* shd, struct rel_entry* ree) = NULL;

/*Ensure EIS_elf valued before invoke this func
Ensure EIS_database valued before invoke this func*/
static int elf_mark()
{
	struct elf_header* elfh = (struct elf_header*)EIS_elf;
	if(elfh->ident[0] != 0x7F
	    || elfh->ident[1] != 'E'
	    || elfh->ident[2] != 'L' 
		|| elfh->ident[3] != 'F')/*magic head chk*/
	{
		fprintf(stderr, "elf_mark:ELF format incorrect.\n");
		return -1;
	}
	
	if(elfh->ident[4] != 0x1 || elfh->ident[5] != 0x1)
	{
		fprintf(stderr, "elf_mark:Unsupport file format. (must be 32-bit, little endian)\n");
		return -1;
	}
	
	if(EIS_pdmsg) printf("elf_mark: ELF basical verifacation passed.\nELF Info:\n");
	if(EIS_pdmsg) printf("type: 0x%hX\nmachine: 0x%hX\nentry: 0x%lX\n", elfh->type, elfh->machine, elfh->entry);
	
	/*for sections...*/
	EIS_shs = (struct section_header*)(EIS_elf + elfh->shoff);
	EIS_shs_cnt = elfh->shnum;
	int i;
	for(i = 0; i < elfh->shnum; ++i)
	{
		if(EIS_pdmsg) printf("elf_mark: section %d[%s] found:\noffset:%lX\nsize:%lu\naddr:%lX\n"
			, i
			, elf_o2p(EIS_shs[elfh->shstrndx].offset) + EIS_shs[i].name
			, EIS_shs[i].offset
			, EIS_shs[i].size
			, EIS_shs[i].addr);
		
		if(EIS_elf_secfnd != NULL) EIS_elf_secfnd(EIS_shs + i);
		
		if(EIS_shs[i].type == 9)/*SHT_REL*/
		{
			if(EIS_pdmsg) printf("elf_mark: section %d is a rel sect. Solving the relocation data.", i);
			struct rel_entry *rees = (struct rel_entry*)elf_o2p(EIS_shs[i].offset);
			while(rees < (struct rel_entry*)elf_o2p(EIS_shs[i].offset + EIS_shs[i].size))
			{
				if(EIS_pdmsg) printf("%lX,%lX\n", elf_p2o((byte_t*)rees), rees->offset);
				
				if(EIS_elf_relfnd != NULL) EIS_elf_relfnd(EIS_shs + i, rees);
				rees++;
			}
		}
	}
	
	/*for segments*/
	EIS_phs     = (struct program_header*)(EIS_elf + elfh->phoff);
	EIS_phs_cnt = elfh->phnum;
	for(i = 0; i < elfh->phnum; ++i)
	{
		if(EIS_pdmsg) printf("elf_mark: segment %d found:\nvaddr:%lX\npaddr:%lX\noffset:%lX\nsize:%lX\n"
			, i
			, EIS_phs[i].vaddr
			, EIS_phs[i].paddr
			, EIS_phs[i].offset
			, EIS_phs[i].filesz);
		
		if(EIS_elf_segfnd != NULL) EIS_elf_segfnd(EIS_phs + i);
	}
	return 0;
}

/*access elf by offset*/
static byte_t* elf_o2p(dword_t p_ofst)
{
	return EIS_elf + p_ofst;
}

/*convert offset to address*/
static ptr_t elf_o2a(dword_t p_ofst)
{
	int i;
	for(i = 0; i < EIS_phs_cnt; ++i)
	{
		if(EIS_phs[i].offset <= p_ofst && EIS_phs[i].offset + EIS_phs[i].filesz > p_ofst)
		{
			int delta = EIS_phs[i].vaddr - EIS_phs[i].offset;
			return p_ofst + delta;
		}
	}
	return -1;
}

/*convert pointer to offset*/
static dword_t elf_p2o(byte_t* p_ptr)
{
	return (qword_t)p_ptr - (qword_t)EIS_elf;
}

/*convert pointer to address*/
static ptr_t elf_p2a(byte_t* p_ptr)
{
	return elf_o2a(elf_p2o(p_ptr));
}

/*convert address to offset*/
static dword_t elf_a2o(ptr_t p_addr)
{
	int i;
	for(i = 0; i < EIS_phs_cnt; ++i)
	{
		if(EIS_phs[i].vaddr <= p_addr && EIS_phs[i].vaddr + EIS_phs[i].memsz > p_addr)
		{
			int delta = EIS_phs[i].vaddr - EIS_phs[i].offset;
			return p_addr - delta;
		}
	}
	return -1;
}

/*access elf by address*/
static byte_t* elf_a2p(ptr_t p_addr)
{
	dword_t ans = elf_a2o(p_addr);
	if(ans == -1) return (byte_t*)-1;
	return elf_o2p(ans);
}

/*naive address 2 pointer(works well)*/
static byte_t* addrmap(ptr_t p_addr)
{
	return EIS_elf + p_addr - EIS_POS_delta;
}

/*naive offset 2 pointer(works well)*/
static ptr_t ofstmap(byte_t* p_elfp)
{
	return (qword_t)p_elfp - (qword_t)EIS_elf - EIS_POS_delta;
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*EISYSTEM_H_*/
