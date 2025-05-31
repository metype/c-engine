#ifndef CENGINE_HASHMAP_H
#define CENGINE_HASHMAP_H

#define map_set(map, key, value) Map_insert(map, key, (void*)(value))
#define map_get(map, key, type) (type)Map_search(map, key)

typedef struct node_s {
    char* key;
    void* value;
    struct node_s* next;
} node_s;

void Map_set_node(node_s* node, char* key, void* value);


typedef struct hash_map_s {

    // Current number of elements in hashMap
    // and capacity of hashMap
    int numOfElements, capacity;

    // hold base address array of linked list
    struct node_s** arr;
} hash_map_s;

void Map_init(hash_map_s* mp);

int Map_hash(hash_map_s* mp, const char* key);

void Map_insert(hash_map_s* mp, char* key, void* value);

void Map_delete(hash_map_s* mp, char* key);

void* Map_search(hash_map_s* mp, const char* key);
#endif //CENGINE_HASHMAP_H
