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

void lru_init(Cache* cache) {
	// Size of the cache
	cache->size = 0;
	cache->limit = 4;
	cache->header = NULL;
	cache->tail = NULL;
}

// This function append a new node to the end of the linked list
void append_node(Cache* cache, Node* newentry) {
	newentry->prev = NULL;
	newentry->next = NULL;
	if (cache->size == 0) {
		// Empty cache
		cache->header = newentry;
	}
	if (cache->tail != NULL) {
		cache->tail->next = newentry;
		newentry->prev = cache->tail;
	}
	cache->tail = newentry;
	cache->size += 1;
}

// This function remove the header from a linked list
Node* remove_header(Cache* cache) {
	if (cache->size == 0)
		return NULL;
	Node* oldheader = cache->header;
	cache->header = cache->header->next;
	if (cache->header != NULL)
		cache->header->prev = NULL;
	free(oldheader);
	cache->size -= 1;
	return oldheader;
}

// This function detach a node from a linked list
void detach(Cache* cache, Node* current) {
	if (current->prev != NULL) {
		current->prev->next = current->next;
	} else {
		cache->header = current->next;
	}
	if (current->next != NULL) {
		current->next->prev = current->prev;
	} else {
		cache->tail = current->prev;
	}

	current->prev = NULL;
	current->next = NULL;
	cache->size -= 1;
}


// TODO Implement this operation
int lru_get(Cache* cache, int key) {
	Node* temp=NULL;
	for(temp=cache->header;temp!=NULL;temp=temp->next){
		if(temp->key==key){
			return temp->value;
		}
	}
	return NOT_FOUND;
}

void lru_put(Cache* cache, int key, int value) {

	Node *temp=cache->header;
	Node *newnode=malloc(sizeof(Node));
	newnode->key=key;
	newnode->value=value;
	if(cache->size < cache->limit && lru_get(cache,key)==NOT_FOUND){
		append_node(cache,newnode);
	}else if(cache->size < cache->limit && lru_get(cache,key)>0){
		for(temp;temp!=NULL;temp=temp->next){
			if(temp->key==key){
				detach(cache,temp);
				break;
			}
		}
		append_node(cache,newnode);
	}else if(cache->size == cache->limit && lru_get(cache,key)==NOT_FOUND){
		remove_header(cache);
		append_node(cache,newnode);
	}else if(cache->size == cache->limit && lru_get(cache,key)>0){
		for(temp;temp!=NULL;temp=temp->next){
				if(temp->key==key){
				detach(cache,temp);
							 break;
			}
		}
		append_node(cache,newnode);
	}
}

int main() {
	Cache* cache = (Cache*) malloc(sizeof(Cache));

	lru_init(cache);

	lru_put(cache, 1, 5);
	assert(cache->size == 1);

	lru_put(cache, 2, 2);
	assert(cache->size == 2);

	lru_put(cache, 3, 3);
	assert(cache->size == 3);

	lru_put(cache, 4, 9);
	assert(cache->size == 4);

	lru_put(cache, 1, 3);
	assert(lru_get(cache, 1) == 3);
	assert(cache->size == 4);

	lru_put(cache, 5, 1);
	assert(cache->size == 4);
	assert(lru_get(cache, 2) == NOT_FOUND);

	lru_put(cache, 6, 9);
	assert(cache->size == 4);
	assert(lru_get(cache, 3) == NOT_FOUND);

       lru_get(cache, 4);
        lru_put(cache, 7, 2);
        assert(cache->size == 4);
        assert(lru_get(cache, 1) == NOT_FOUND);

}
