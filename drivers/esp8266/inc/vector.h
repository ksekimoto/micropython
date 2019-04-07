/*
 * vector.h
 *
 * https://gist.githubusercontent.com/EmilHernvall/953968/raw/0fef1b1f826a8c3d8cfb74b2915f17d2944ec1d0/vector.h
 *
 */

#ifndef DRIVERS_ESP8266_INC_VECTOR_H_
#define DRIVERS_ESP8266_INC_VECTOR_H_

typedef struct vector_ {
    void** data;
    int size;
    int count;
} vector;

void vector_init(vector*);
int vector_count(vector*);
void vector_add(vector*, void*);
void vector_set(vector*, int, void*);
void *vector_get(vector*, int);
void vector_delete(vector*, int);
void vector_free(vector*);

#endif /* DRIVERS_ESP8266_INC_VECTOR_H_ */
