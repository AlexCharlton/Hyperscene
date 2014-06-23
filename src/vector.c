#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

/* Vectors */
void hpgInitVector(HPGvector *vector, size_t initialCapacity){
    if (initialCapacity > 0){
	vector->data = malloc(initialCapacity * sizeof(void *));
    } else {
	vector->data = NULL;
    }
    vector->capacity = initialCapacity;
    vector->isStatic = false;
    vector->size = 0;
}

void hpgInitStaticVector(HPGvector *vector, void *data, size_t capacity){
    vector->data = data;
    vector->capacity = capacity;
    vector->isStatic = true;
    vector->size = 0;
}

HPGvector *hpgNewVector(size_t initialCapacity){
    HPGvector *vec = malloc(sizeof(HPGvector));
    hpgInitVector(vec, initialCapacity);
    return vec;
}

void hpgDeleteVector(HPGvector *vector){
    if (!vector->isStatic)
	free(vector->data);
    //free(vector); // TODO: Should this be here, if so, there needs to be some other deletion routine.
}

void hpgPush(HPGvector *vector, void *value){
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

void *hpgPop(HPGvector *vector){
    if (vector->size == 0)
	return NULL;
    return vector->data[--vector->size];
}

void hpgInsert(HPGvector *vector, void *value, size_t index){
    if (index >= vector->size) return;
    vector->data[index] = value;
}

void *hpgVectorValue(HPGvector *vector, size_t index){
    if (index >= vector->size) return NULL;
    return vector->data[index];
}

size_t hpgLength(HPGvector *vector){
    return vector->size; 
}

bool hpgRemove(HPGvector *vector, void *el){
    int i;
    for (i = 0; i < vector->size; i++){
	if (vector->data[i] == el){
	    hpgRemoveNth(vector, i);
	    return true;
	}
    }
    return false;
}

bool hpgRemoveNth(HPGvector *vector, size_t index){
    if (vector->size == 0 || index >= vector->size) return false;
    if (index <= (vector->size - 1))
	memcpy(&vector->data[index], &vector->data[index + 1],
	       sizeof(void*) * (vector->size - index - 1));
    --vector->size;
    return true;
}
