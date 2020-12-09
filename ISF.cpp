#include "EISystem.h"
#include <string.h>
#include "common.h"
#include "x.h"
#include "i.h"
#include "de.h"

#define STR_HELP \
"q - quit environment.\n\
x - find data xref(s)\n\
o - change input mode to offset.\n\
a - change input mode to address.\n\
s - seek in elf.\n\
i - to echo elf info.\n\
d - to dump something.\n\
e - to echo something.\n\
/*m - do modify on elf memory image.\n\
y - seek specific binary code in image.\n*/\
? - show this help.\n"

static int (*regdfunc[128])(const char * param);

void register_func();

int callfunc(const char * param);

int main()
{
	printf("Interactive String Founder Ver.1.3x\n\
The decode table should be put with the executable file.\n\
And must named as 'Shift_JIS_table.tbl'\n");
	char buffer[1024];
	printf("Please, input the file name: ");
	scanf("%s", buffer);
	
	elf_rinit(buffer);
	elf_mark();
	readencode("Shift_JIS_table.tbl");
	np = EIS_elf;
	
	register_func();
	
	getchar(); 
	printf("[%c:%08lX]> ", (input_mode == ISF_IM_ADDR) ? 'A' : 'O'
		,(input_mode == ISF_IM_ADDR) ? elf_p2a(np) : elf_p2o(np));
	scanf("%[^\n]", buffer);
	getchar();
	while(buffer[0] != 'q')
	{
		if(callfunc(buffer))
			fputs("Occured error when process request.\n", stderr);
		
		printf("[%c:%08lX]> ", (input_mode == ISF_IM_ADDR) ? 'A' : 'O'
			,(input_mode == ISF_IM_ADDR) ? elf_p2a(np) : elf_p2o(np));
		scanf("%[^\n]", buffer);getchar();
	}
	return 0;
}

int callfunc(const char* param)
{
	if(regdfunc[(int)param[0]] == NULL) return -1;
	return regdfunc[(int)param[0]](param + 1);
}

int help(const char * param)
{
	puts(STR_HELP);
	return 0;
}

int tg2o(const char * param)
{
	input_mode = ISF_IM_OFST;
	return 0;
}

int tg2a(const char * param)
{
	input_mode = ISF_IM_ADDR;
	return 0;
}

int seek(const char * param)
{
	dword_t n = 0;
	
	dword_t mode;
	switch(param[0])
	{
		case '+':
			mode = 1;
			++param;
			break;
		case 'n':
			mode = 2;
			++param;
			break;
		case '-':
			mode = 3;
			++param;
			break;
		case 'p':
			mode = 4;
			++param;
			break;
		case '?':
			printf("s N - seek to location N\n\
s+ N - seek next N byte(s)\n\
s- N - seek prev N byte(s)\n\
sn N - seek next N byte(s) and invoke edd imd.\n\
sp N - seek prev N byte(s) and invoke edd imd.\n");
			break;
		default:
			mode = 0;
	}
	//n = elf_p2o(np);
	
	if(EOF == sscanf(param, "%lX", &n))
		n = -1;/*if user inpput nothing*/
	
	byte_t* tmp;
	if(mode == 0)
	{
		if(n == -1)
		{
			fputs("seek: Error: parameter incomplete.\n", stderr);
			return -1;
		}
		if((byte_t*)-1 == (tmp = (input_mode == ISF_IM_ADDR) ? elf_a2p(n) : elf_o2p(n)))
		{
			fprintf(stderr, "seek: Error: can not seek to %p(input:%08lX)\n", (void*)tmp, n);
			return -1;
		}
		else
		{
			np = tmp;
			return 0;
		}
	}
	else
	{
		if(n == -1) n = 256;
		if(mode < 3)
		{
			tmp = np + n;
		}
		else
		{
			tmp = np - n;
		}
		
		if(tmp > EIS_elf + EIS_elflen || tmp < EIS_elf)
		{
			fprintf(stderr, "seek: Error: can not seek to %p(input:%08lX)\n", (void*)tmp, n);
			return -1;
		}
		np = tmp;
		if(mode == 2 || mode == 4) 
		{
			deout=stdout;
			dwind(np, 256);
		}
		return 0;
	}
}

void register_func()
{
	regdfunc[(int)'?'] = help;
	regdfunc[(int)'x'] = xref;
	regdfunc[(int)'o'] = tg2o;
	regdfunc[(int)'a'] = tg2a;
	regdfunc[(int)'s'] = seek;
	regdfunc[(int)'d'] = dump;
	regdfunc[(int)'e'] = echo;
	regdfunc[(int)'i'] = info;
}
