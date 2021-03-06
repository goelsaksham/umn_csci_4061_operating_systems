#include <time.h>
#include <stdlib.h>
#include <stdio.h>

struct node {
	int val;
	struct node* left;
	struct node* right;
};

struct btree {
	struct node* root;
};

void insert(struct btree* tree, int val) {
	struct node* ptr = tree->root;
	int flag = 1;
	while (ptr != NULL && flag) {
		if (ptr->val > val) {
			if (ptr->left == NULL) {
				struct node* head = (struct node *)malloc(sizeof(struct node)); //allocate memory
			  *head = (struct node){val,NULL, NULL}; //cast to struct node 
				ptr->left = head;
				flag = 0;
			} else {
				ptr = ptr->left;
			}
		} else {
			if (ptr->right == NULL) {
				struct node* head = (struct node *)malloc(sizeof(struct node)); //allocate memory
			  *head = (struct node){val,NULL, NULL}; //cast to struct node 
				ptr->right = head;
				flag = 0;
			} else {
				ptr = ptr->right;
			}
		}
	}
	// TODO: Implement insertion.
}

struct btree* populate(int size) {

	// DO NOT CHANGE OR REUSE THIS LINE!
	srand(time(NULL));

	// TODO: Insert 'size' number of random nodes.
	// Obtain random number using rand()
	int x = rand();

	struct node* head = (struct node *)malloc(sizeof(struct node)); //allocate memory
	*head = (struct node){x, NULL, NULL}; //cast to struct node 

	struct btree* bt = (struct btree *)malloc(sizeof(struct btree)); //allocate memory
	*bt = (struct btree){head}; //cast to struct node 

	for (int i = 1; i < size; i++) {
		int x = rand();
		insert(bt, x);
	}
	return bt;
}


void print_tree(struct node* root) {
	if (root == NULL) {
		printf("%s", "");
	} else {
		print_tree(root->left);
		printf("%d\n", root->val);
		print_tree(root->right);
	}
}


int main(int argc, char** argv) {

	if (argc < 2) {

		printf("Invalid number of args");
		exit(1);
	}

	int size = atoi(argv[1]);

	if (size < 1) {

		printf("Size must be > 0");
		exit(1);
	}

	// Randomly populate a linked list.
	struct btree* tree = populate(size);
	print_tree(tree->root);

	return 0;
}
