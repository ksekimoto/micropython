/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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

/*
 * http://tharikasblogs.blogspot.com/p/include-include-include-mymalloc.html
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "py/runtime.h"
#include "rz_buf.h"

#define RZ_RAM_ALLOC_SIZE   0x00280000
static __attribute((section("NC_BSS"),aligned(32))) uint8_t rz_buf[RZ_RAM_ALLOC_SIZE];

#define ALIGN_4BYTE

static struct block *freeList = (struct block *)NULL;
static void *memory_start = (void *)NULL;
static size_t memory_size = 0;

static void rz_malloc_init_sub(void *memory, size_t size) {
    memory_start = memory;
    memory_size = size;
    memset(memory, 0, (size_t)size);
    freeList = (struct block *)memory;
    freeList->size = size - sizeof(struct block);
    freeList->free = 1;
    freeList->next = (struct block *)NULL;
}

void rz_malloc_init(void) {
    rz_malloc_init_sub((void *)&rz_buf, (size_t)RZ_RAM_ALLOC_SIZE);
}

static void split(struct block *fitting_slot, size_t size) {
    size_t chunk_size = size + sizeof(struct block);
    struct block *new = (struct block *)((char *)fitting_slot + chunk_size);
    new->size = fitting_slot->size - chunk_size;
    new->free = 1;
    new->next = fitting_slot->next;
    fitting_slot->size = size;
    fitting_slot->free = 0;
    fitting_slot->next = new;
}

void *rz_malloc(size_t numbytes) {
    struct block *curr;
    void *result;
    #if defined(ALIGN_4BYTE)
    numbytes = (numbytes + 3) / 4 * 4;  // 4 byte alignment
    #endif
    curr = freeList;
    while ((((curr->size) < numbytes) || ((curr->free) == 0)) && (curr->next != NULL)) {
        curr = curr->next;
    }
    if ((curr->size) == numbytes) {
        curr->free = 0;
        result = (void *)(++curr);
    } else if ((curr->size) > (numbytes + sizeof(struct block))) {
        split(curr, numbytes);
        result = (void *)(++curr);
    } else {
        result = (void *)NULL;
    }
    return result;
}

static void merge() {
    struct block *curr;
    curr = freeList;
    while ((curr != NULL) && (curr->next) != NULL) {
        if ((curr->free) && (curr->next->free)) {
            curr->size += (curr->next->size) + sizeof(struct block);
            curr->next = curr->next->next;
        }
        curr = curr->next;
    }
}

void rz_free(void *ptr) {
    if (((char *)memory_start <= (char *)ptr) && ((char *)ptr <= ((char *)memory_start + memory_size))) {
        struct block *curr = (struct block *)ptr;
        --curr;
        curr->free = 1;
        merge();
    }
}
