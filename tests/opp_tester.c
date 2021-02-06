#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define IN_FILE  "data.str"
#define OUT_FILE "out.opp"

typedef struct {
	FILE *in, *out; 
} Ctx;

static Ctx ctx;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//abcdefghi
static char *pstring="abcdefghisl";

char *perm, *poly;
int   plen;

void perminit(char *s) 
{
    plen = strlen(s);
    perm = (char *)malloc((plen+1)*sizeof(char));
    perm[plen]=0;
    poly = (char *)malloc((plen+1)*sizeof(char));
    int p;
    for(p=0;p<plen;p++) poly[p]=0; 
} 

int permtick(void) 
{ 
   
    int ret=-1;  
    int p=plen-2;

    while( p >= 0 ) {
        poly[p]++;        
        if(poly[p]>=(plen-p)) {
            poly[p]=0;    
            p--;        

        } else {
            p=-2;       
        }
    }

    if(p==-1) ret=0;

    return(ret);
}


void buildperm(char *s) {

    char c;
    int i;
    for(i=0;i<plen;i++) perm[i]=s[i]; 

    for(i=0;i<(plen-1);i++) if(poly[i]>0) {
        c              =perm[i];
        perm[i]        =perm[i+poly[i]];
        perm[i+poly[i]]=c;
    }  
}

void test_local_var(unsigned int num)
{
	char data[40] = {0};
	fprintf(ctx.out, "void main() {\n");

	for (unsigned int i = 0; i < num; i++) {
		
		fgets(data, 40, ctx.in);
		data[strlen(data)-1] = ' ';
		fprintf(ctx.out, "int %s = 2;\n", data);
	}

	fprintf(ctx.out, "}\n // will this work??");
}

void test_struct_elems(unsigned int num)
{
	char data[15] = {0};
	fprintf(ctx.out, "struct t1 {\n");

	char* types[] = {"int", "long", "short", "char"};

	for (unsigned int i = 0; i < num; i++) {
		
		fgets(data, 15, ctx.in);
		data[strlen(data)-1] = ' ';

		int r = rand() % 4;
		int u = rand() % 2;

		if (u)
			fprintf(ctx.out, "\tunsigned %s %s;\n", types[r], data);
		else
			fprintf(ctx.out, "\t%s %s;\n", types[r], data);
	}

	fprintf(ctx.out, "};\n // will this work??\n");
	fprintf(ctx.out, "void main() { t1 a; }\n");
}

void test_exprs(unsigned int num)
{
	char data[40] = {0};
	fprintf(ctx.out, "void main() {\n");

	for (unsigned int i = 0; i < num; i++) {
		
		fgets(data, 40, ctx.in);
		data[strlen(data)-1] = ' ';
		fprintf(ctx.out, "int %s = 2;\n", data);
		fprintf(ctx.out, "%s = (%s + 3) + (%s + (5*2));\n", data, data, data);
	}

	fprintf(ctx.out, "}\n // will this work??");
}

int main(int argc, const char** argv)
{
	ctx.in = fopen(IN_FILE, "r");
	ctx.out = fopen(OUT_FILE, "w");

	test_local_var(900000);
	// test_struct_elems(200000);
	// test_exprs(300000);


    // perminit(pstring);
    // do {
    // buildperm(pstring);
    // puts(perm);
    // } while(permtick()); 

	fclose(ctx.in);
	fclose(ctx.out);

	return 0;
}


