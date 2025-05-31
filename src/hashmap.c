#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

void Map_set_node(node_s* node, char* key, void* value)
{
    node->key = key;
    node->value = value;
    node->next = nullptr;
}

// like constructor
void Map_init(hash_map_s* mp)
{

    // Default capacity in this case
    mp->capacity = 100;
    mp->numOfElements = 0;

    // array of size = 1
    mp->arr = malloc(sizeof(node_s*) * mp->capacity);

    for(int i = 0;i < mp->capacity ; i++){
        mp->arr[i] = nullptr;
    }
}

int Map_hash(hash_map_s* mp, const char* key)
{
    int bucketIndex;
    int sum = 0, factor = 31;
    for (int i = 0; i < strlen(key); i++) {

        // sum = sum + (ascii value of
        // char * (primeNumber ^ x))...
        // where x = 1, 2, 3....n
        sum = ((sum % mp->capacity)
               + (((int)key[i]) * factor) % mp->capacity)
              % mp->capacity;

        // factor = factor * prime
        // number....(prime
        // number) ^ x
        factor = ((factor % __INT16_MAX__)
                  * (31 % __INT16_MAX__))
                 % __INT16_MAX__;
    }

    bucketIndex = sum;
    return bucketIndex;
}

void Map_insert(hash_map_s* mp, char* key, void* value)
{

    // Getting bucket index for the given
    // key - value pair
    int bucketIndex = Map_hash(mp, key);
    struct node_s* newNode = (struct node_s*)malloc(

            // Creating a new node
            sizeof(struct node_s));

    // Setting value of node
    Map_set_node(newNode, key, value);
    mp->numOfElements++;

    // Bucket index is empty....no collision
    if (mp->arr[bucketIndex] == nullptr) {
        mp->arr[bucketIndex] = newNode;
    }

        // Collision
    else {

        // Adding newNode at the head of
        // linked list which is present
        // at bucket index....insertion at
        // head in linked list
        newNode->next = mp->arr[bucketIndex];
        mp->arr[bucketIndex] = newNode;
    }
}

void Map_delete(hash_map_s* mp, char* key)
{

    // Getting bucket index for the
    // given key
    int bucketIndex = Map_hash(mp, key);

    struct node_s* prevNode = nullptr;

    // Points to the head of
    // linked list present at
    // bucket index
    struct node_s* currNode = mp->arr[bucketIndex];
    mp->numOfElements--;

    while (currNode != nullptr) {

        // Key is matched at delete this
        // node from linked list
        if (strcmp(key, currNode->key) == 0) {

            // Head node
            // deletion
            if (currNode == mp->arr[bucketIndex]) {
                mp->arr[bucketIndex] = currNode->next;
            }

                // Last node or middle node
            else {
                prevNode->next = currNode->next;
            }
            free(currNode);
            break;
        }
        prevNode = currNode;
        currNode = currNode->next;
    }
}

void* Map_search(hash_map_s* mp, const char* key)
{
    // Getting the bucket index
    // for the given key
    int bucketIndex = Map_hash(mp, key);

    // Head of the linked list
    // present at bucket index
    struct node_s* bucketHead = mp->arr[bucketIndex];
    while (bucketHead != nullptr) {

        // Key is found in the hashMap
        if (strcmp(bucketHead->key, key) == 0) {
            return bucketHead->value;
        }
        bucketHead = bucketHead->next;
    }

    return nullptr;
}