
#include "ss.h"

void init_layer(StorageLayer* layer, int size) {
       	layer->cache = (Cache*)malloc(sizeof(Cache));
	lru_init(layer->cache, size);
	layer->local = (int*)malloc(sizeof(int)*size);
}

void init_ss(StorageSystem* ss) {
	StorageLayer* cpucache = (StorageLayer*)malloc(sizeof(StorageLayer));
	init_layer(cpucache, 3);
	
	StorageLayer* memory = (StorageLayer*)malloc(sizeof(StorageLayer));
	init_layer(memory, 5);
	
	StorageLayer* disk = (StorageLayer*)malloc(sizeof(StorageLayer));
	init_layer(disk, 20);	

	cpucache->below = memory;
	memory->below = disk;
	disk->below = NULL;

	ss->topLayer = cpucache;
}

// TODO Implement this
int read_layer(StorageLayer* layer, int key) {
	if(layer==NULL) return NOT_FOUND;
	int index = lru_get(layer->cache,key);
	if(index !=-1){
		return layer->local[index];
	}else{
		read_layer(layer->below,key);
	}
}

// TODO Implement this
void write_layer(StorageLayer* layer, int key, int value){
	if(layer==NULL) return;
	int index = lru_get(layer->cache,key);
	if(index == NOT_FOUND){
		if(layer->cache->size == layer->cache->limit){
			layer->local[layer->cache->header->value]=value;
			lru_put(layer->cache,key,layer->cache->header->value);
		}else{
			layer->local[layer->cache->size]=value;		
			lru_put(layer->cache,key,layer->cache->size);
		}
	}else{
		layer->local[index]=value;
		lru_put(layer->cache,key,index);
	}
	write_layer(layer->below,key,value);
}

int read(StorageSystem* ss, int key) {
	return read_layer(ss->topLayer, key);
}

void write(StorageSystem* ss, int key, int value) {
	write_layer(ss->topLayer, key, value);
}
