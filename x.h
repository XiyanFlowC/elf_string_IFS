/*code chunk x.h*/
/*NOT A REAL HEADER FILE. DO NOT INCLUDE ME!*/

int xref(const char * param)
{
	ptr_t target = elf_p2a(np);
	
	ptr_t* p = (ptr_t*)EIS_elf;
	ptr_t* uplim = (ptr_t*)(EIS_elf + EIS_elflen);
	
	printf("xref: searching xref to A:%08lX\n==========\n", target);
	while(p < uplim)
	{
		if(target == *p)
			printf("xref: dataxref: O:%08lX A:%08lX\n", elf_p2o((byte_t*)p), elf_p2a((byte_t*)p));
		++p;
	}
	putchar('\n');
	return 0;
}
