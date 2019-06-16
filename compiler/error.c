
void ERROR_FOUND(int in, char* c, char* b)
{
	if (in == 1) {printf("Expected -> %c but got => %c\n", *c, *b);}
	if (in == 2) {printf("Expected -> %s but got => %s\n", c, b);}
	if (in == 3) {printf("Expected -> %c after %s\n", *c, b);}
}

void SMALL_ERR(int in)
{
	if (in == 1) printf("Expected Identifer after @\n");
}