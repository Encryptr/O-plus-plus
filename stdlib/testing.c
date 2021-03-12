void printnumb(long num){
	printf("%ld\n", num);
}

int rand_to(int limit) {
	return (rand() % limit);
}

void rand_setup() {
	srand(time(0));
}