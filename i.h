/*code chunk i.h*/
/*NOT A REAL HEADER FILE. DO NOT INCLUDE ME!*/

#define STR_INFOHELP \
"ih - show info here\n\
ia - show info about the address\n\
io - show info about the offset\n\
ip N - show info of program header which id is N\n\
is N - show info of section header which id is N\n\
i? - show this help\n"

void elfinfo(dword_t loc)
{
	dword_t p_addr = elf_o2a(loc);
	int sgi, sci;
	for(sgi = 0; sgi < EIS_phs_cnt; ++sgi)
	{
		if(EIS_phs[sgi].vaddr <= p_addr && EIS_phs[sgi].vaddr + EIS_phs[sgi].memsz > p_addr)
		{
			break;
		}
	}
	for(sci = 0; sci < EIS_shs_cnt; ++sci)
	{
		if(EIS_shs[sci].addr <= p_addr && EIS_shs[sci].addr + EIS_shs[sci].size > p_addr)
		{
			break;
		}
	}
	
	if(sgi == EIS_phs_cnt || sci == EIS_shs_cnt)
	{
		printf("Here(O:%08lX)", loc);
		if(sgi == EIS_phs_cnt) printf(" is no seg,");
		else printf(" is in seg:[%d],", sgi);
		if(sci == EIS_shs_cnt) printf(" and no sec.\n");
		else printf(" but in seg:[%d].\n", sci);
		return;
	}
	
	printf("Here(O:%08lX) is in the seg:[%d] and the sec:[%d][%s]\n"
		, loc
		, sgi
		, sci
		, elf_o2p(EIS_shs[((elf_header*)EIS_elf)->shstrndx].offset) + EIS_shs[sci].name);
}

void shinfo(int idx)
{
	if(idx < 0 || idx >= EIS_shs_cnt)
	{
		fputs("shinfo: Invalid index.\n", stderr);
		return;
	}
	
	section_header* sh = EIS_shs + idx;
	
	printf("The %d[%s] section:\nAddr:[%lX] Offset:[%lX] Size:[%lX]\ninfo [%lX], type [%lX]\n"
	, idx
	, elf_o2p(EIS_shs[((elf_header*)EIS_elf)->shstrndx].offset) + sh->name
	, sh->addr
	, sh->offset
	, sh->size
	, sh->info
	, sh->type);
}

void phinfo(int idx)
{
	if(idx < 0 || idx >= EIS_phs_cnt)
	{
		fputs("phinfo: Invalid index.\n", stderr);
		return;
	}
	
	program_header* ph = EIS_phs + idx;
	
	printf("The %d segment:\nPAddr[%lX] VAddr[%lX]\nOffset:[%lX] FSize:[%lX] MSize:[%lX]\n"
		, idx
		, ph->paddr
		, ph->vaddr
		, ph->offset
		, ph->filesz
		, ph->memsz);
}

int info(const char * param)
{
	//qword_t tmpq;
	//word_t tmpw;
	//byte_t tmpb;
	static dword_t tmpd;
	static ptr_t tmpp;
	
	switch(param[0])
	{
		case 's':
			sscanf(param + 2, "%ld", &tmpd);
			shinfo(tmpd);
			break;
		case 'p':
			sscanf(param + 2, "%ld", &tmpd);
			phinfo(tmpd);
			break;
		case 'o':
			sscanf(param + 2, "%lX", &tmpd);
			elfinfo(tmpd);
			break;
		case 'a':
			sscanf(param + 2, "%lX", &tmpp);
			elfinfo(elf_a2o(tmpp));
			break;
		case 'h':
			elfinfo(elf_p2o(np));
			break;
		case '?':
			printf(STR_INFOHELP);
			break;
		default:
			printf("Use i? to get help.\n");
	}
	return 0;
}
