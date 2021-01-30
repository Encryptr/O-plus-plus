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

static char *pstring="abcdefghi";

char *perm, *poly;
int   plen;

void perminit(char *s) {
    /* We have moved the init code to an init function,
    /* where it truly belongs */

    plen=strlen(s);
    
    perm=(char *)malloc((plen+1)*sizeof(char));
    perm[plen]=0;

    poly=(char *)malloc((plen+1)*sizeof(char));

    /* poly is a byte array that we are going to use as a big counter */
    int p;
    for(p=0;p<plen;p++) poly[p]=0; 
} 

int permtick(void) { 
    /* Each time we call permtick, it increments our poly counter */ 

    int ret=-1;   /* Return True by default */ 
    int p=plen-2; /* Start at 2nd to last position */ 

    while( p >= 0 ) {
        /* Increment poly digit */
        poly[p]++;        

        /* If poly digit exceeds plen-p, move to 
        /* the next digit and loop */
        if(poly[p]>=(plen-p)) {
            poly[p]=0;    
            p--;        

            /* FYI - this is why poly[plen-1] is always 0:
            /* That's it's maximum value, which is why we
            /* start at plen-2 */
        } else {
            p=-2;        /* Done looping */
        }
    }

    /* All permutations have been calculated and p=-1 */
    if(p==-1) ret=0;

    return(ret);
}


void buildperm(char *s) {
    /* Build a permutation from the poly counter */

    char c;
    int i;

    /* Start with a fresh copy of the string */
    for(i=0;i<plen;i++) perm[i]=s[i]; 

    /* Swap digits based on each poly digit */ 
    /* if poly[i]>0 then swap with the (i+nth) digit */
    for(i=0;i<(plen-1);i++) if(poly[i]>0) {
        c              =perm[i];
        perm[i]        =perm[i+poly[i]];
        perm[i+poly[i]]=c;
    }  
}

void test_local_var(unsigned int num)
{
	char data[15] = {0};
	fprintf(ctx.out, "void main() {\n");

	for (unsigned int i = 0; i < num; i++) {
		
		fgets(data, 15, ctx.in);
		data[strlen(data)-1] = ' ';
		fprintf(ctx.out, "int %s = 2;\n", data);
	}

	fprintf(ctx.out, "}\n // will this work??");
}

void test_struct_elems(unsigned int num)
{
	char data[10] = {0};
	fprintf(ctx.out, "struct t1 {\n");

	char* types[] = {"int", "long", "short", "char"};

	for (unsigned int i = 0; i < num; i++) {
		
		fgets(data, 10, ctx.in);
		data[strlen(data)-1] = ' ';

		int r = rand() % 4;
		int u = rand() % 2;

		if (u)
			fprintf(ctx.out, "\tunsigned %s %s;\n", types[r], data);
		else
			fprintf(ctx.out, "\t%s %s;\n", types[r], data);
	}

	fprintf(ctx.out, "};\n // will this work??\n");
}

int main(int argc, const char** argv)
{
	ctx.in = fopen(IN_FILE, "r");
	ctx.out = fopen(OUT_FILE, "w");

	// test_local_var(200000);
	// test_struct_elems(10000);

    // perminit(pstring);
    // do {
    // buildperm(pstring);
    // puts(perm);
    // } while(permtick()); 

	fclose(ctx.in);
	fclose(ctx.out);

	return 0;
}


