#include "cheat.h"
#include "src/memory.h"

/* Vectors */
CHEAT_TEST(vector_push_pop,
           HPSvector *vector = hpsNewVector(2);
           cheat_assert(hpsLength(vector) == 0);
           cheat_assert(vector->capacity == 2);
           hpsPush(vector, (void *) 1);
           hpsPush(vector, (void *) 2);
           hpsPush(vector, (void *) 3);
           hpsPush(vector, (void *) 4);
           hpsPush(vector, (void *) 5); // [1, 2, 3, 4, 5]
    
           hpsRemove(vector, (void *) 2); // [1, 3, 4, 5]

           cheat_assert(hpsLength(vector) == 4);
           cheat_assert(vector->capacity == 8); // capacity was doubled twice

           cheat_assert(hpsVectorValue(vector, 0) == (void *) 1);
           cheat_assert(hpsVectorValue(vector, 1) == (void *) 3);
           cheat_assert(hpsVectorValue(vector, 2) == (void *) 4);

           cheat_assert(hpsPop(vector) == (void *) 5); // [1, 3, 4]
           cheat_assert(hpsLength(vector) == 3);

           hpsInsert(vector, (void *) 5, 2); // [1, 3, 5]
           cheat_assert(hpsPop(vector) == (void *) 5); // [1, 3]

           cheat_assert(hpsVectorValue(vector, 0) == (void *) 1);
           cheat_assert(hpsPop(vector) == (void *) 3); // [1]
           cheat_assert(hpsPop(vector) == (void *) 1); // []
           cheat_assert(hpsPop(vector) == NULL);
           cheat_assert(hpsLength(vector) == 0);
           cheat_assert(hpsVectorValue(vector, 0) == NULL);

           hpsDeleteVector(vector);
    )

CHEAT_TEST(zero_length_vector,
           HPSvector *vector = hpsNewVector(0);
           cheat_assert(hpsLength(vector) == 0);
           cheat_assert(vector->capacity == 0);
           hpsPush(vector, (void *) 1);
           cheat_assert(vector->capacity == DEFAULT_VECTOR_SIZE);
           cheat_assert(hpsPop(vector) == (void *) 1);
           cheat_assert(hpsPop(vector) == NULL);
           cheat_assert(hpsLength(vector) == 0);
           hpsDeleteVector(vector);
    )


CHEAT_TEST(pool,
           HPSpool pool = hpsMakePool(sizeof(int), 2, "test pool");
           struct pool *data = (struct pool*) pool;
           cheat_assert(data->nBlocks == 2);
           cheat_assert(data->blockSize == sizeof(void*));
           int *first = (int *) hpsAllocateFrom(pool);
           int *savedFirst = first;
           int *second = (int *) hpsAllocateFrom(pool);
           *second = 2;
           hpsDeleteFrom((void *)first, pool);
           first = (int *) hpsAllocateFrom(pool);
           *first = 1;
           cheat_assert(savedFirst == first);
           int *third = (int *) hpsAllocateFrom(pool);
           *third = 3;
           cheat_assert(*first = 1);
           cheat_assert(*second = 2);
           cheat_assert(*third = 3);
    )
