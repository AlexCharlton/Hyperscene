#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

/* Vectors */
void hpsInitVector(HPSvector *vector, size_t initialCapacity){
    if (initialCapacity > 0){
	vector->data = malloc(initialCapacity * sizeof(void *));
    } else {
	vector->data = NULL;
    }
    vector->capacity = initialCapacity;
    vector->isStatic = false;
    vector->size = 0;
}

void hpsInitStaticVector(HPSvector *vector, void *data, size_t capacity){
    vector->data = data;
    vector->capacity = capacity;
    vector->isStatic = true;
    vector->size = 0;
}

HPSvector *hpsNewVector(size_t initialCapacity){
    HPSvector *vec = malloc(sizeof(HPSvector));
    hpsInitVector(vec, initialCapacity);
    return vec;
}

void hpsDeleteVector(HPSvector *vector){
    if (!vector->isStatic){
	free(vector->data);
    }
    //free(vector); // TODO: Should this be here, if so, there needs to be some other deletion routine.
}

void hpsPush(HPSvector *vector, void *value){
    if (vector->size == vector->capacity){
	if (vector->isStatic){
	    void * new = malloc(2 * vector->capacity * sizeof(void *));
	    memcpy(new, vector->data,
		   sizeof(void*) * vector->capacity);
	    vector->data = new;
	    vector->capacity *= 2;
	    vector->isStatic = false;
	} else if (vector->capacity != 0){
	    vector->data = realloc(vector->data,
				   2 * vector->capacity * sizeof(void *));
	    vector->capacity *= 2;
	} else {
	    vector->data = malloc(DEFAULT_VECTOR_SIZE * sizeof(void *));
	    vector->capacity = DEFAULT_VECTOR_SIZE;
	}

    }
    vector->data[vector->size++] = value;
}

void *hpsPop(HPSvector *vector){
    if (vector->size == 0)
	return NULL;
    return vector->data[--vector->size];
}

void hpsInsert(HPSvector *vector, void *value, size_t index){
    if (index >= vector->size) return;
    vector->data[index] = value;
}

void *hpsVectorValue(HPSvector *vector, size_t index){
    if (index >= vector->size) return NULL;
    return vector->data[index];
}

size_t hpsLength(HPSvector *vector){
    return vector->size; 
}

bool hpsRemove(HPSvector *vector, void *el){
    int i;
    for (i = 0; i < vector->size; i++){
	if (vector->data[i] == el){
	    hpsRemoveNth(vector, i);
	    return true;
	}
    }
    return false;
}

bool hpsRemoveNth(HPSvector *vector, size_t index){
    if (vector->size == 0 || index >= vector->size) return false;
    if (index <= (vector->size - 1))
	memmove(&vector->data[index], &vector->data[index + 1],
                sizeof(void*) * (vector->size - index - 1));
    --vector->size;
    return true;
}
