/*code chunk de.h*/
/*NOT A REAL HEADER FILE. DO NOT INCLUDE ME!*/

#define STR_EHELP "el - echo uint64 here\n\
ew - echo uint16 here\n\
ec - echo char here\n\
ei - echo uint32 here\n\
es - echo string start here\n\
ed - echo datas\n\
e? - show this help\n"

#define STR_DHELP "df - set dump file path\n\
ds - dump string start here\n\
dp - dump pointer table\n\
dd - dump data\n\
d? - show this help\n" 

static FILE* deout = NULL;
static FILE* doutf = NULL;

static int code[20480];
static char dec[81960];

int p = 0, pc = 0;

int predef_bad_get_align() { return 8;}

static int (*get_align)() = predef_bad_get_align; 

void sspn(byte_t* tar, int lmt);
void dstring(byte_t* ptr);

void readencode(const char * path)
{
	FILE* encoding = fopen(path, "r");
	
	int rst = 0;
	int cod;
	char s[64];
	
	while(EOF != (rst = fscanf(encoding, "%X=%s", &cod, s)))
	{
		if(rst == 2)
		{
			code[p++] = cod;
			
			int leng = strlen(s);
			strcpy(dec + pc, s);
			pc += leng + 1;
		}
	}
	
	fclose(encoding);
}

void dpp(byte_t* str, int lmt);
int dppjmp(const char * param);

int dumpptt(const char * param)
{
	static int len;
	
	if(deout == NULL)
	{
		fputs("dumpptt: F can not dump things to NULL\n", stderr);
	}
	switch(param[0])
	{
		case 'p':
			return dppjmp(param + 1);
		case 'd':
			if(1 != sscanf(param + 1, "%d", &len))
			{
				fputs("dumpptt: F parameter incorrect\n", stderr);
				return -2;
			}
			fprintf(deout, "\nDS (O:%08lX, %08lX) =====\n", elf_p2o(np), elf_p2o(np + len));
			sspn(np, len);
			break;
		case '?':
			puts("dpp N - dump pointer point to string only for next N pointer(s)(uint32)\n\
dpd N - dump pointer with other data for next N byte(s)\n\
dp? - show this help\n\n\
Note: this is NOT the best way. Code your new codes with EISystem.h\n");
			break;
		default:
			puts("Use dp? to show help.\n");
	}
	return 0;
}

int dmpdat(const char * param);

int dump(const char * param)
{
	deout = doutf;
	switch(param[0])
	{
		case 'f':
			if(doutf != NULL) fclose(doutf);
			doutf = fopen(param + 1, "a");
			if(doutf == NULL)
			{
				fprintf(stderr, "dump: E can not open file [%s]\n", param + 1);
				return -1;
			}
			break;
		case 's':
			if(deout == NULL)
			{
				fputs("dump: F can not write to NULL\n", stderr);
				return -2;
			}
			fprintf(deout, "%08X,%d,", elf_p2o(np), strlen((char*)np));
			dstring(np);
			fputs("\n\n", deout);
			break;
		case 'p':
			return dumpptt(param + 1);
			break;
		case 'd':
			return dmpdat(param + 1);
			break;
		case '?':
			puts(STR_DHELP);
			break;
		default:
			puts("Use d? to get help.\n");
	}
	return 0;
}

void dstring(byte_t* ptr)
{
	int inch;
	int dbc = 0;
	int tc = 0;
	int i = 0;
	while(0 != (inch = ptr[i++]))
	{
		if(!dbc && inch <= 0x7F)
		{
			if(inch == '\r') fputs("\\r", deout);
			else if(inch == '\n') fputs("\\n", deout);
			else fputc(inch, deout);
		}
		else if(!dbc && inch > 0x7F)
		{
			dbc = 1;
			tc = inch << 8;
		}
		else if(dbc)
		{
			dbc = 0;
			tc += inch;
			for(int j = 0; j < p; ++j)
			{
				if(code[j] == tc)
				{
					pc = 0;
					for(int k = 0; k < j; ++k)
					{
						while(dec[pc] != 0) ++pc;
						++pc;
					}
					fputs(dec + pc, deout);
					break;
				}
			}
		}
		else fputs("dstring: ERROR\n", stderr);
	}
}

void echodata(const char * param);

int echo(const char * param)
{
	deout = stdout;
	switch(param[0])
	{
		case 'd':
			echodata(param + 1);
			break;
		case 'l':
			printf("0x%llX\n", *((qword_t*)np));
			break;
		case 'w':
			printf("0x%hX\n", *((word_t*)np));
			break;
		case 'c':
			printf("0x%hhX(%c)\n", *((byte_t*)np), *((char*)np));
			break;
		case 'i':
			printf("%ld\n", *((dword_t*)np));
			break;
		case 's':
			dstring(np);
			break;
		case '?':
			printf(STR_EHELP);
			break;
		default:
			printf("Use e? to show help.\n");
			return -1;
	}
	putchar('\n');
	return 0;
}

void lnstrcvt(byte_t* src, int lmt)
{
	int inch;
	int dbc = 0;
	int tc = 0;
	int i = 0;
	while(i < lmt)
	{
		inch = src[i++];
		//if(i >= lmt) break;
		if(!dbc && (inch < ' ' || inch == 0x7F))
		{
			fputc('.', deout);
			continue;
		}
		if(!dbc && inch <= 0x7F)
			fputc(inch, deout);
		else if(!dbc && inch > 0x7F)
		{
			dbc = 1;
			tc = inch << 8;
		}
		else if(dbc)
		{
			dbc = 0;
			tc += inch;
			int j;
			for(j = 0; j < p; ++j)
			{
				if(code[j] == tc)
				{
					pc = 0;
					for(int k = 0; k < j; ++k)
					{
						while(dec[pc] != 0) ++pc;
						++pc;
					}
					fputs(dec + pc, deout);
					break;
				}
			}
			if(j == p) fputs("..", deout);
		}
		else fputs("lnstrcvt: ERROR\n", stderr);
	}
}

void dwind(byte_t* st, int lmt)
{
	puts(" Offset | +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F | 0123456789ABCDEF\n");
	for(int i = 0; i < lmt; i = i + 0x10)
	{
		printf("%08lX| %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX | "
			, elf_p2o(st + i)
			, *(byte_t*)(st + i + 0)
			, *(byte_t*)(st + i + 1)
			, *(byte_t*)(st + i + 2)
			, *(byte_t*)(st + i + 3)
			, *(byte_t*)(st + i + 4)
			, *(byte_t*)(st + i + 5)
			, *(byte_t*)(st + i + 6)
			, *(byte_t*)(st + i + 7)
			, *(byte_t*)(st + i + 8)
			, *(byte_t*)(st + i + 9)
			, *(byte_t*)(st + i + 0xA)
			, *(byte_t*)(st + i + 0xB)
			, *(byte_t*)(st + i + 0xC)
			, *(byte_t*)(st + i + 0xD)
			, *(byte_t*)(st + i + 0xE)
			, *(byte_t*)(st + i + 0xF));
		lnstrcvt(st + i, 16);
		putchar('\n');
	}
}

void sspn(byte_t* tar, int lmt)
{
	ptr_t* ssp = (ptr_t*) tar;
	
	while((void*)ssp < (void*)(tar + lmt))
	{
		fprintf(deout, "O:%08lX(value:%08lX) : ", elf_p2o((byte_t*)ssp), *((dword_t*)ssp));
		byte_t* str = elf_a2p(*ssp);
		if(str < EIS_elf || str > EIS_elf + EIS_elflen) fputs("[Not a String]", deout);
		else dstring(elf_a2p(*ssp));
		fputc('\n', deout);
		ssp ++;
	}
}

void echodata(const char * param)
{
	static dword_t tmpd;
	switch(param[0])
	{
		case 'l':
			sscanf(param + 2, "%ld", &tmpd);
			dwind(np, tmpd);
			break;
		case 's':
			sscanf(param + 2, "%ld", &tmpd);
			sspn(np, tmpd);
			break;
		case 'd':
			dwind(np, 256);
			break;
		case '?':
			printf("edl N - print N bytes\neds N - print N byte with string solve\nedd - print next 256 bytes");
			break;
		default:
			printf("Use ed? to show help.\n");
	}
}

int dppjmp(const char * param)
{
	static int len;
	if(1 != sscanf(param, "%d", &len))
	{
		fputs("dppjmp: F parameter incorrect\n", stderr);
		return -2;
	}
	
	dpp(np, len);
	return 0;
}

int is_rcgnzbl(byte_t* src)
{
	int inch = *src;
	if(inch <= 0x7F)
	{
		if((inch < ' ' || inch == 0x7F) && (inch != '\r' || inch != '\n')) return 0;
		return 1;
	}
	else/*dbc*/
	{
		inch <<= 8;
		inch |= *(src + 1);
		int j;
		for(j = 0; j < p; ++j)
		{
			if(code[j] == inch)
			{
				return 1;
			}
		}
		return 0;
	}
}

int dmpstr(int Soff, int Toff)
{
	if(Soff & 0x3)
	{
		fprintf(stderr, "dmpstr: E Soff(%08X) not aligned!\n", Soff);
		return -2;
	}
	byte_t *s = elf_o2p(Soff), *t = elf_o2p(Toff);
	
	fprintf(deout, "\nSTRDMP (O:%08X,%08X) =====\n", Soff, Toff);
	while(s < t)
	{
		if(is_rcgnzbl(s))
		{
			fprintf(deout, "%08lX,", elf_p2o(s));
			dstring(s);
			fputs("\n\n", deout);
			unsigned int len = strlen((char*)s) + 1;
			len = (len + get_align()) & ~((unsigned int)get_align() - 1);
			s += len;
		}
		else s+=4;
	}
	return 0;
}

int dmpdat(const char * param)
{
	if(deout == NULL)
	{
		fputs("dmpdat: F can not dump things to NULL.\n", stderr);
		return -2;
	}
	int S, T, N;
	byte_t* ps, *pt;
	switch(param[0])
	{
		case 's':
			N = sscanf(param + 1, "%X %X", &S, &T);
			S = (input_mode == ISF_IM_ADDR) ? elf_a2o(S) : S;
			T = (input_mode == ISF_IM_ADDR) ? elf_a2o(T) : T;
			return dmpstr(S, T);
			break;
		case 'd':
			N = sscanf(param + 1, "%X %X", &S, &T);
			ps = (input_mode == ISF_IM_ADDR) ? elf_a2p(S) : elf_o2p(S);
			pt = (input_mode == ISF_IM_ADDR) ? elf_a2p(T) : elf_o2p(T);
			fprintf(deout, "\nDump (O:%08lX,%08lX) =====\n", elf_p2o(ps), elf_p2o(pt));
			while(ps < pt)
			{
				for(int i = 0; i < 16; ++i)
					if(ps < pt) fprintf(deout, "%02hhX ", *ps++);
					else break;
				fputc('\n', deout);
			}
			break;
		case '?':
			printf("ddd S T - dump data during S(included) to T(not included)\n\
dds S T - dump string S(included) to T(not included)\n\
ddn N - dump next N byte(s)\n\
dd? - show this help\n");
		default:
			puts("Use dd? to get help.\n");
	}
	return 0;
}

void dpp(byte_t* str, int lmt)
{
	ptr_t* sp = (ptr_t*)str;
	fprintf(deout, "\nPST (O:%08lX,%08lX)=====\n", elf_p2o(str), elf_p2o(str + lmt*4));
	for(int i = 0; i < lmt; ++i)
	{
		if(elf_a2p(*sp) < EIS_elf || elf_a2p(*sp) >= EIS_elf + EIS_elflen)
		{
			sp++;
			continue;
		}
			
		else if(is_rcgnzbl(elf_a2p(*sp)))
		{
			fprintf(deout, "%08lX,%08lX:", elf_p2o((byte_t*)sp), elf_a2o(*sp));
			dstring(elf_a2p(*sp));
		}
		fputc('\n', deout);
		sp++;
	}
	fputc('\n', deout);
}
