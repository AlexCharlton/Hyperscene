#ifndef HPS_MEMORY
#define HPS_MEMORY 1

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
} HPSvector;

typedef void* HPSpool;

/* Pools */
HPSpool hpsMakePool(size_t blockSize, size_t nBlocks, char name[32]);

void hpsInitPool(HPSpool pool, void *data, size_t blockSize, size_t nBlocks, char name[32]);

void hpsDeletePool(HPSpool pool);

void hpsClearPool(HPSpool pool);

void *hpsAllocateFrom(HPSpool pool);

void hpsDeleteFrom(void *block, HPSpool pool);

/* Vectors */
void hpsInitVector(HPSvector *vector, size_t initialCapacity);

void hpsInitStaticVector(HPSvector *vector, void *data, size_t capacity);

HPSvector *hpsNewVector(size_t initialCapacity);

void hpsDeleteVector(HPSvector *vector);

void hpsPush(HPSvector *vector, void *value);

void *hpsPop(HPSvector *vector);

void hpsInsert(HPSvector *vector, void *value, size_t index);

void *hpsVectorValue(HPSvector *vector, size_t index);

size_t hpsLength(HPSvector *vector);

bool hpsRemove(HPSvector *vector, void *el);

bool hpsRemoveNth(HPSvector *vector, size_t index);

#endif
