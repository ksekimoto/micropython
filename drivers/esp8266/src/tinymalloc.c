/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 * Copyright (c) 2019 Kentaro Sekimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <stdio.h>
#include <stddef.h>
#include "py/runtime.h"
#include "common.h"
#include "tinymalloc.h"

#ifdef USE_DBG_PRINT
//#define DEBUG_TINY_MALLOC
#endif

//#define TMEM_SIZE   20000
//char memory[TMEM_SIZE];

static struct block *freeList=(void*)NULL;
static void *memory_start = NULL;
static int memory_size = 0;

void tinymalloc_init(void *memory, size_t size) {
    memory_start = memory;
    memory_size = (int)size;
    freeList=(void*)memory;
    freeList->size = size - sizeof(struct block);
    freeList->free = 1;
    freeList->next = NULL;
}

void split(struct block *fitting_slot, size_t size) {
    struct block *new = (void *)((void *)fitting_slot + size + sizeof(struct block));
    new->size = (fitting_slot->size) - size - sizeof(struct block);
    new->free = 1;
    new->next = fitting_slot->next;
    fitting_slot->size = size;
    fitting_slot->free = 0;
    fitting_slot->next = new;
}

void *tinymalloc(size_t noOfBytes) {
    struct block *curr;
    void *result;
//    if (!(freeList->size)) {
//        tinymalloc_init();
//#if defined(DEBUG_TINY_MALLOC)
//        debug_printf("Memory Initialized\n");
//#endif
//    }
    curr = freeList;
    while ((((curr->size) < noOfBytes) || ((curr->free) == 0)) && (curr->next != NULL)) {
        curr = curr->next;
#if defined(DEBUG_TINY_MALLOC)
        debug_printf("One block checked\n");
#endif
    }
    if ((curr->size) == noOfBytes) {
        curr->free = 0;
        result = (void *)(++curr);
#if defined(DEBUG_TINY_MALLOC)
        debug_printf("Exact fitting block allocated\n");
#endif
        return result;
    } else if ((curr->size) > (noOfBytes + sizeof(struct block))) {
        split(curr, noOfBytes);
        result = (void *)(++curr);
#if defined(DEBUG_TINY_MALLOC)
        debug_printf("Fitting block allocated with a split\n");
#endif
        return result;
    } else {
        result = NULL;
#if defined(DEBUG_TINY_MALLOC)
        debug_printf("Sorry. No sufficient memory to allocate\n");
#endif
        return result;
    }
}

void merge() {
    struct block *curr;
    curr = freeList;
    while ((curr->next) != NULL) {
        if ((curr->free) && (curr->next->free)) {
            curr->size += (curr->next->size) + sizeof(struct block);
            curr->next = curr->next->next;
        }
        curr = curr->next;
    }
}

void tinyfree(void *ptr) {
    if (((void *)memory_start <= ptr) && (ptr <= (void *)(memory_start + memory_size))) {
        struct block *curr = ptr;
        --curr;
        curr->free = 1;
        merge();
    } else {
#if defined(DEBUG_TINY_MALLOC)
        debug_printf("Please provide a valid pointer allocated by tinymalloc\n");
#endif
    }
}
