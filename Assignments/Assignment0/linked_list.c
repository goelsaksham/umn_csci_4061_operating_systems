#include<stdio.h>

typedef struct Node {
	int x;
	struct Node* next;
}start_node;

struct Node* get_integers(int num) {
	int temp = 0;
	scanf("%d", &temp);

	struct Node* head = (struct Node *)malloc(sizeof(struct Node)); //allocate memory
  *head = (struct Node){temp,NULL}; //cast to struct node 

	struct Node* ptr = head;

	for(int i = 0; i < num-1; i ++)
	{
		scanf("%d", &temp);
		struct Node* tmp = (struct Node *)malloc(sizeof(struct Node)); //allocate memory
	  *tmp = (struct Node){temp,NULL}; //cast to struct node 
		ptr->next = tmp;
		ptr = ptr->next;
	}
	return head;
}

int get_element(struct Node* start, int index) {
	struct Node* head = start;
	for (int i = 0; i < index; i++) {
			head = head->next;
	}
	return head->x;
}

int set_element(struct Node* start, int index, int elem) {
	struct Node* head = start;
	for (int i = 0; i < index; i++) {
			head = head->next;
	}
	head->x = elem;
}

void sort(struct Node* start, int len) {
	if (start-> next == NULL) {

	} else {
		for (int i = 1; i < len; i++) {
			int key = get_element(start, i);
      int j = i-1;
      while (j >= 0 && get_element(start, j) > key)
      {
         set_element(start, j+1, get_element(start, j));
         j = j-1;
      }
			set_element(start, j+1, key);
		}
	}
}

void print_sorted(struct Node* start) {
	printf("%s\n", "PRINTING ELEMENTS IN SORTED ORDER");
	struct Node* iter = start;
	while(iter != NULL) {
		printf("%d\n", iter->x);
		iter = iter->next;
	}
}


int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Nedd to supply the no. of Numbers for input\n");
	} else {
		int num = atoi(argv[1]);
		struct Node* l = get_integers(num);
		sort(l, num);
		print_sorted(l);
	}
	return 0;
}

