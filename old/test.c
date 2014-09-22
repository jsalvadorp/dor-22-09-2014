#include <stdio.h>
#include "array.h"

define_array(int);

int main() {
	array(int) miarr = array_int_new();
	
	int x;
	
	while(scanf("%d", &x) != EOF) {
		array_int_push(&miarr, x);
	}
	
	printf("miarr: ");
	
	for(int i = 0; i < miarr.size; i++) {
		printf("%d ", array_at(miarr, i));
	}
	
	printf("\n");
}
