#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NOT_FOUND -1

typedef struct _Node {
	struct _Node* prev;
	struct _Node* next;
	int key;
	int value;
} Node;

typedef struct _Cache {
	int size;
	int limit;
	Node* header;
	Node* tail;
} Cache;

void lru_init(Cache* cache, int size);

int lru_get(Cache* cache, int key); 

void lru_put(Cache* cache, int key, int value);

void remove_header(Cache* cache);
