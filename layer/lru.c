#include "lru.h"

void lru_init(Cache* cache, int size) {
	// Size of the cache
	cache->size = 0;
	cache->limit = size;
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
void remove_header(Cache* cache) {
	if (cache->size == 0)
		return;
	Node* oldheader = cache->header;
	cache->header = cache->header->next;
	if (cache->header != NULL)
		cache->header->prev = NULL;
	cache->size -= 1;
	free(oldheader);
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

Node* find_node(Cache* cache, int key) {
	Node* current = cache->header;
	while (current != NULL) {
		if (current->key == key) {
			// Disconnect current node
			detach(cache, current);
			append_node(cache, current);
			return current;
		}
		current = current->next;
	}
	return NULL;
}

int lru_get(Cache* cache, int key) {
	Node* n = find_node(cache, key);
	if (n != NULL)
		return n->value;
	return NOT_FOUND;
}

void lru_put(Cache* cache, int key, int value) {
	Node* n = find_node(cache, key);
	if (n != NULL) {
		n->value = value;
	} else {
		while (cache->size >= cache->limit) {
			// Remove header
			remove_header(cache);
		}
		Node *newentry = (Node*) malloc(sizeof(Node));
		newentry->key = key;
		newentry->value = value;
		append_node(cache, newentry);
	}
}

