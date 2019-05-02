/*
 * vector.c
 *
 * https://gist.githubusercontent.com/EmilHernvall/953968/raw/0fef1b1f826a8c3d8cfb74b2915f17d2944ec1d0/vector.c
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/runtime.h"
#include "common.h"
#include "vector.h"

#if USE_DBG_PRINT
#define DEBUG_VECTOR
#endif

#define VECTOR_INIT_SIZE    10

void vector_init(vector *v) {
#if defined(DEBUG_VECTOR)
    debug_printf("vector_init(v=%x)\r\n", v);
#endif
    v->data = NULL;
    v->size = 0;
    v->count = 0;
}

int vector_count(vector *v) {
#if defined(DEBUG_VECTOR)
    debug_printf("vector_count() ret=%d\r\n", v->count);
#endif
    return v->count;
}

void vector_add(vector *v, void *e) {
#if defined(DEBUG_VECTOR)
    debug_printf("vector_add(v=%x, e=%x)\r\n", v, e);
#endif
    if (v->size == 0) {
        v->size = VECTOR_INIT_SIZE;
        v->data = (void **)malloc(sizeof(void *) * v->size);
        memset(v->data, '\0', sizeof(void *) * v->size);
    }

    // condition to increase v->data:
    // last slot exhausted
    if (v->size == v->count) {
        v->size *= 2;
        v->data = (void **)realloc(v->data, sizeof(void *) * v->size);
    }

    v->data[v->count] = e;
    v->count++;
#if defined(DEBUG_VECTOR)
    for (int i = 0; i < v->count; i++) {
        debug_printf("v->data[%d]=%x\r\n", i, v->data[i]);
    }
#endif
}

void vector_set(vector *v, int index, void *e) {
#if defined(DEBUG_VECTOR)
    debug_printf("vector_set(v=%x, index=%d, e=%x)\r\n", v, index, e);
#endif
    if (index >= v->count) {
        return;
    }
    v->data[index] = e;
#if defined(DEBUG_VECTOR)
    for (int i = 0; i < v->count; i++) {
        debug_printf("v->data[%d]=%x\r\n", i, v->data[i]);
    }
#endif
}

void *vector_get(vector *v, int index) {
#if defined(DEBUG_VECTOR)
    for (int i = 0; i < v->count; i++) {
        debug_printf("v->data[%d]=%x\r\n", i, v->data[i]);
    }
    debug_printf("vector_get(v=%x, index=%d)\r\n", v, index);
#endif
    if (index >= v->count) {
        return 0;
    }

#if defined(DEBUG_VECTOR)
    debug_printf("v->data[%d]=%x\r\n", index, v->data[index]);
#endif
    return v->data[index];
}

void vector_delete(vector *v, int index) {
#if defined(DEBUG_VECTOR)
    debug_printf("vector_delete(v=%x, index=%d)\r\n", v, index);
    for (int i = 0; i < v->count; i++) {
        debug_printf("v->data[%d]=%x\r\n", i, v->data[i]);
    }
#endif
    if (index >= v->count) {
        return;
    }
    v->data[index] = NULL;

    int i, j;
    void **newarr = (void **)malloc(sizeof(void *) * v->count * 2);
    for (i = 0, j = 0; i < v->count; i++) {
        if (v->data[i] != NULL) {
            newarr[j] = v->data[i];
            j++;
        }
    }
    free(v->data);
    v->data = newarr;
    v->count--;
#if defined(DEBUG_VECTOR)
    for (int i = 0; i < v->count; i++) {
        debug_printf("v->data[%d]=%x\r\n", i, v->data[i]);
    }
#endif
}

void vector_free(vector *v) {
#if defined(DEBUG_VECTOR)
    debug_printf("vector_free(v=%x)\r\n", v);
#endif
    free(v->data);
}

#if 0
int main(void)
{
    vector v;
    vector_init(&v);

    vector_add(&v, "emil");
    vector_add(&v, "hannes");
    vector_add(&v, "lydia");
    vector_add(&v, "olle");
    vector_add(&v, "erik");

    int i;
    printf("first round:\n");
    for (i = 0; i < vector_count(&v); i++) {
        printf("%s\n", vector_get(&v, i));
    }

    vector_delete(&v, 1);
    vector_delete(&v, 3);

    printf("second round:\n");
    for (i = 0; i < vector_count(&v); i++) {
        printf("%s\n", vector_get(&v, i));
    }

    vector_free(&v);

    return 0;
}
#endif
