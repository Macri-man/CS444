
#include "lru.h"

typedef struct _StorageLayer {
	struct _StorageLayer* below;
	Cache* cache;
	int* local;
} StorageLayer;

typedef struct _StorageSystem {
	StorageLayer* topLayer;	
} StorageSystem;

void init_ss(StorageSystem* ss);

int read(StorageSystem* ss, int key);

void write(StorageSystem* ss, int key, int value);
