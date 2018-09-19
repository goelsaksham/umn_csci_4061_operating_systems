#include <stdlib.h>
#include <stdio.h>

struct bankAccount {
	char * name;
	double balance;
};

// Returns a pointer that points to the value of val.
// added a & sign so that pointer points to the address of the variable
int* getInt(int val) {
	int* p;
	p = &val;
	return p;
}

int main() {

	// Pointer review. You MUST use getInt(int) (you may alter the function though).
	int* num = getInt(5);
	printf("Value of x = %d\n", *num);

	// Create an array of 100 integers.
	// used malloc call as provided in the ppt to make the array containing 100 integers
	int* array = (int *)malloc(sizeof(int)*100);

	// Create a two-dimensional array of size 10x25.
	int x = 10;
	int y = 25;
	// changed the int** to int* and made a 2D array by multiplying x and y to get 250 integers in the 2d array
	int* twoDim = (int *)malloc(sizeof(int)*y*x);
	for (int i=0; i < x; ++i) {
		for (int j=0; j < y; ++j) {
			*(twoDim + i*y + j) = i * j;
		}
	}
	printf("Element at location (5,5) = %d\n", *(twoDim + 5*y + 5));

	// Structure review. The print statement must use a struct pointer.
	struct bankAccount acc1 = {"My Account", 1000};
	struct bankAccount* p = &acc1;
	// changed the . operator to a -> operator
	printf("Balance in the account %s = %f\n", p->name, p->balance);
}
