#ifndef HPG_MEMORY
#define HPG_MEMORY 1

#include <stdbool.h>

#define DEFAULT_VECTOR_SIZE 4

struct pool{
    unsigned int blockSize;
    unsigned int nBlocks;
    void **freeBlock;
    void *nextPool;
    char name[32];
};

typedef struct {
    void **data;
    size_t size;
    size_t capacity;
    bool isStatic; // Data cannot be grown
} HPGvector;

typedef void* HPGpool;

/* Pools */
HPGpool hpgMakePool(size_t blockSize, size_t nBlocks, char name[32]);

void hpgDeletePool(HPGpool pool);

void hpgClearPool(HPGpool pool);

void *hpgAllocateFrom(HPGpool pool);

void hpgDeleteFrom(void *block, HPGpool pool);

/* Vectors */
void hpgInitVector(HPGvector *vector, size_t initialCapacity);

void hpgInitStaticVector(HPGvector *vector, void *data, size_t capacity);

HPGvector *hpgNewVector(size_t initialCapacity);

void hpgDeleteVector(HPGvector *vector);

void hpgPush(HPGvector *vector, void *value);

void *hpgPop(HPGvector *vector);

void hpgInsert(HPGvector *vector, void *value, size_t index);

void *hpgVectorValue(HPGvector *vector, size_t index);

size_t hpgLength(HPGvector *vector);

bool hpgRemove(HPGvector *vector, void *el);

bool hpgRemoveNth(HPGvector *vector, size_t index);

#endif
