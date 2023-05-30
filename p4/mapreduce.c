#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "mapreduce.h"

/**
 * @brief Global variables are initialized here.
 * 
 */
struct HashMap** ptr;
Partitioner partitioner;
Mapper mapper;
Reducer reducer;
int MAP_INIT_CAPACITY = 11;
int partition_number;
int file_num;
int curr_file;
int curr_partition;
pthread_mutex_t fileLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *partitionLock;
struct HashNode** reducerNode;


/**
 * @brief Introduce a new version of the hash map, using nodes and HashSubNodes 
 * containing the key and value pairs.
 */
struct HashNode {
    char* key;
    struct HashNode *next;
    struct HashSubNode *HashSubNode;
    struct HashSubNode *current;
    int size;
};

struct HashMap {
    int capacity;
    long nodeSize;
    struct HashNode **contents;
    pthread_mutex_t *locks;
    pthread_mutex_t keyLock;
};

struct HashSubNode {
    char* value;
    struct HashSubNode* next;
};

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create a hash map object. Using this in place of the provided hashmap.
 * 
 * @param size 
 * @return struct HashMap* 
 */
struct HashMap *createMap(int capacity) {
    struct HashMap *h = (struct HashMap*)malloc(sizeof(struct HashMap));
    h -> nodeSize = 0;
    h -> capacity = capacity;
    h -> contents = malloc(sizeof(struct HashNode*)*capacity);
    h -> locks = malloc(sizeof(pthread_mutex_t)*capacity);
    pthread_mutex_init(&h -> keyLock, NULL);
    
    for (int i = 0; i < capacity; i++){
        h -> contents[i] = NULL;
        if (pthread_mutex_init(&(h -> locks[i]), NULL) != 0) {
                printf("Lock initialization failed!\n");
        }
    }
    return h;
}

/**
 * @brief Generic hashing function, based off of one featured on 
 * 
 * @param str 
 * @return unsigned long 
 */
unsigned long hashFunction(char *str) {
     int count;
     unsigned long hash = 5381;
 
     while((count = *str++)!= '\0') {
        hash = ((hash << 5) + hash) + count;
     }
     return hash = hash % MAP_INIT_CAPACITY;
}

/**
 * @brief Insert method for the hash table. 
 * 
 * @param t Take in the table as an argument, parse through and add respective
 * key and value pairs to the nodes/HashSubNodes.
 * @param key Key to be inserted.
 * @param value Value to be inserted.
 */
void hashInsert(struct HashMap *h, char* key, char* value) {  
    long position = hashFunction(key);
    pthread_mutex_t *lock = h -> locks + position;
    struct HashSubNode *initHashSubNode = malloc(sizeof(struct HashSubNode));
    initHashSubNode -> value = strdup(value);
    initHashSubNode -> next = NULL;
    pthread_mutex_lock(lock);
    struct HashNode *contents = h -> contents[position];
    struct HashNode *tempHash = contents;

    while (tempHash) {
        if (strcmp(tempHash -> key, key) == 0) {
            struct HashSubNode *subcontents = tempHash -> HashSubNode;
            initHashSubNode -> next=subcontents;
            tempHash -> size++;
            tempHash -> HashSubNode = initHashSubNode;
            tempHash -> current = initHashSubNode;
            pthread_mutex_unlock(lock);
            return;
        }
        tempHash = tempHash -> next;
    }
    pthread_mutex_lock(&h -> keyLock);
    h -> nodeSize++;
    pthread_mutex_unlock(&h -> keyLock);

    struct HashNode *initNode = malloc(sizeof(struct HashNode));
    initNode -> key = strdup(key);
    initNode -> HashSubNode = initHashSubNode;
    initNode -> current = initNode -> HashSubNode;    
    initNode -> next = contents;
    h -> contents[position] = initNode;
    pthread_mutex_unlock(lock);
}

/**
 * @brief Compare function, takes in the contents of two nodes and preforms a 
 * simple equivalence comparison on the contents.
 * 
 * @param string1 
 * @param string2 
 * @return int 
 */
int compareNode(const void *string1, const void *string2) {
     struct HashNode **node1 = (struct HashNode **)string1;
     struct HashNode **node2 = (struct HashNode **)string2;

     if(*node1 == NULL && *node2 == NULL) {
        return 0;
     } else if(*node1 == NULL) {
        return -1;
     } else if(*node2 == NULL) {
        return 1;
     } else {
        return strcmp((*node1) -> key, (*node2) -> key);
     }
}

/**
 * @brief Get the Next object in the hash map.
 * 
 * @param key 
 * @param partition_num 
 * @return char* 
 */
char* getNext(char* key, int partition_num) {
    struct HashNode *tempNode = reducerNode[partition_num];
    struct HashSubNode *address = tempNode -> current;
    if (address == NULL){
        return NULL;
      }
     tempNode -> current = address -> next;
     return address -> value;
}

/**
 * @brief Simple helper function used to launch the mapper. Takes in a filename.
 * 
 * @param fileName 
 * @return void* 
 */
void* mapperHelper(char *fileName){
    mapper(fileName);
    return NULL;
}

/**
 * @brief Reducer function, used to optimize the hash map.
 * 
 * @param i 
 */
void reduce(int i) {
    if (ptr[i] == NULL) {
        return;
    }
    struct HashMap* tempTable = ptr[i];
    struct HashNode *contents[ptr[i] -> nodeSize];
    long item = 0;
    for (int j = 0; j < MAP_INIT_CAPACITY; j++) {
        if (tempTable -> contents[j] == NULL) {
                continue;
        }
        struct HashNode* tempNode = tempTable -> contents[j];
        while (tempNode) {
            contents[item] = tempNode;
            item++;
            tempNode = tempNode -> next;
        }
    }
    qsort(contents, ptr[i] -> nodeSize, sizeof(struct HashNode *), compareNode);
    for (int k = 0; k < item; k++) {
        reducerNode[i] = contents[k];
        reducer(contents[k] -> key, getNext, i);
    }
}

/**
 * @brief Helper function used to call the reduce function. Void return, simply
 * used to handle the acquistion and release of locks.
 * 
 * @return void*
 */
void* reducerHelper() {
    while(1) {
        pthread_mutex_lock(&fileLock);
        int part;

        if (curr_partition >= partition_number) {
            pthread_mutex_unlock(&fileLock);
            return NULL;
        }

        if (curr_partition < partition_number) {
            part = curr_partition;
            curr_partition++;
        }
        pthread_mutex_unlock(&fileLock);
	    reduce(part);
    }    
}

/**
 * @brief Getter function for files. Returns null, but passes files on the the
 * mapperhelper function. 
 * 
 * @param files 
 * @return void* 
 */
void *getFiles(void *files) {    
    char **arguments = (char**) files;
    while(1) {
        pthread_mutex_lock(&fileLock);
        int file;

        if (curr_file > file_num) {
            pthread_mutex_unlock(&fileLock); 
            return NULL;
        }

        if (curr_file <= file_num) {
            file = curr_file;
            curr_file++;
        }
        pthread_mutex_unlock(&fileLock); 
        mapperHelper(arguments[file]);  
    }
    return NULL;
}

/**
 * @brief Function used to free up the memory allocated to the hashmap.
 * 
 * @param t 
 */
void freeHashMap(struct HashMap *h) {
    for (int i = 0; i < h -> capacity; i++) {
	struct HashNode *contents1 = h -> contents[i];
        struct HashNode *tempHash2 = contents1;
        pthread_mutex_t *l = &(h -> locks[i]);

        while (tempHash2) { 	
            struct HashNode *tempNode = tempHash2;
            struct HashSubNode *tempHash = tempNode->HashSubNode;
            while (tempHash) {
                struct HashSubNode *subcontents = tempHash;
                tempHash = tempHash -> next; 
		free(subcontents -> value);	
                free(subcontents);
            }
            tempHash2 = tempHash2 -> next;
            free(tempNode -> key);
            free(tempNode); 
	    }
	    pthread_mutex_destroy(l);
    }
    
    free(h -> locks);
    free(h -> contents);
    free(h);
}

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Default hash partition is to be used in the event that, a custom hash
 * is not implemented.
 * 
 * @param key 
 * @param num_partitions 
 * @return unsigned long 
 */
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    int count;
    unsigned long hash = 5381;
    
    while ((count = *key++) != '\0') {
        hash = hash * 33 + count;
    }
    return hash % num_partitions;
}

/**
 * @brief 
 * 
 * @param key 
 * @param value 
 */
void MR_Emit(char *key, char *value) {
    long partNumber;
    if (partitioner != NULL) {
        partNumber = partitioner(key, partition_number);
    } else {
        partNumber = MR_DefaultHashPartition(key, partition_number);
    } 
    hashInsert(ptr[partNumber], key, value);    
}

/**
 * @brief Mapreduce run function takes in arguments in the form of mappers, 
 * reducers, and partitioner assistance functions.
 * 
 * @param argc 
 * @param argv 
 * @param map 
 * @param num_mappers 
 * @param reduce 
 * @param num_reducers 
 * @param partition 
 */
void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition) {

        // Initialize variables:
        curr_partition = 0;
        curr_file = 1;
        partitioner = partition;
        mapper = map;
        reducer = reduce;
        partition_number = num_reducers;
        file_num = argc - 1;
        ptr = malloc(sizeof(struct HashMap *)*num_reducers);
        
        //Create Partitions
        for (int i = 0; i < partition_number; i++) {
                struct HashMap *HashMap = createMap(MAP_INIT_CAPACITY);
                ptr[i] = HashMap;
        }
        
        //Start Mapping Process
        pthread_t mappers[num_mappers];
        for(int i = 0; i < num_mappers; i++){
                pthread_create(&mappers[i], NULL, getFiles, (void*) argv);
        } 

        //Join the Mappers       
        for (int i = 0; i < num_mappers; i++) {
                pthread_join(mappers[i], NULL);
        }

        //Start Reducing Process
        pthread_t reducers[num_reducers];
        reducerNode = malloc(sizeof(struct HashNode *) * num_reducers);
        for (int i = 0; i < num_reducers; i++) {
                pthread_create(&reducers[i], NULL, reducerHelper, NULL);
        }
        
        //Join the Reducers
        for (int i = 0; i < num_reducers; i++) {
                pthread_join(reducers[i], NULL);
        }
        
        //Memory clean-up
        for (int i = 0; i < partition_number; i++) {
                freeHashMap(ptr[i]);
        }

        free(partitionLock);
        free(reducerNode);
        free(ptr);
}
