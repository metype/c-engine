#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

void map_set_node(node* node, char* key, void* value)
{
    node->key = key;
    node->value = value;
    node->next = nullptr;
};

// like constructor
void map_init(hash_map* mp)
{
    // Default capacity in this case
    mp->capacity = 100;
    mp->numOfElements = 0;

    // array of size = 1
    mp->arr = malloc(sizeof(node*) * mp->capacity);

    for(int i = 0;i < mp->capacity ; i++){
        mp->arr[i] = nullptr;
    }
}

int map_hash(hash_map* mp, char* key)
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

void map_insert(hash_map* mp, char* key, void* value)
{

    // Getting bucket index for the given
    // key - value pair
    int bucketIndex = map_hash(mp, key);
    struct node* newNode = (struct node*)malloc(

            // Creating a new node
            sizeof(struct node));

    // Setting value of node
    map_set_node(newNode, key, value);

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

void map_delete(hash_map* mp, char* key)
{

    // Getting bucket index for the
    // given key
    int bucketIndex = map_hash(mp, key);

    struct node* prevNode = nullptr;

    // Points to the head of
    // linked list present at
    // bucket index
    struct node* currNode = mp->arr[bucketIndex];

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

void* map_search(hash_map* mp, char* key)
{
    // Getting the bucket index
    // for the given key
    int bucketIndex = map_hash(mp, key);

    // Head of the linked list
    // present at bucket index
    struct node* bucketHead = mp->arr[bucketIndex];
    while (bucketHead != nullptr) {

        // Key is found in the hashMap
        if (bucketHead->key == key) {
            return bucketHead->value;
        }
        bucketHead = bucketHead->next;
    }

    return nullptr;
}