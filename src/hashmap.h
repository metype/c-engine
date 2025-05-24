#ifndef CENGINE_HASHMAP_H
#define CENGINE_HASHMAP_H

#define map_set(map, key, value) map_insert(map, key, (void*)value)
#define map_get(map, key, type) (type)map_search(map, key)

typedef struct node {
    char* key;
    void* value;
    struct node* next;
} node;

void map_set_node(node* node, char* key, void* value);

typedef struct hash_map {

    // Current number of elements in hashMap
    // and capacity of hashMap
    int numOfElements, capacity;

    // hold base address array of linked list
    struct node** arr;
} hash_map;

void map_init(hash_map* mp);

int map_hash(hash_map* mp, char* key);

void map_insert(hash_map* mp, char* key, void* value);

void map_delete(hash_map* mp, char* key);

void* map_search(hash_map* mp, char* key);
#endif //CENGINE_HASHMAP_H
