
static char print_stm[200] = {
	".data\nstr: .asciz \"%s\"\n.text\n.global main\nmain: \n\tsub $32, %%rsp\n\tlea str(%%rip), %%rcx\n\tcall puts\n\tadd $32, %%rsp\n\txor %%eax, %%eax\n\tret\n"};

void begin_gen(Gen_Asm *ga, char* str)
{
	ga->fp = fopen(str, "w");
	strcpy(ga->filename, str);
}

void gen_print(Gen_Asm *ga, char* str)
{
	fprintf(ga->fp, print_stm, str);
	fclose(ga->fp);
	char text[100];
	sprintf(text, "gcc %s -o out", ga->filename);
	system(text);
}
