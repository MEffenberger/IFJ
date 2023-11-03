/**
 * @file error.h
 *
 * IFJ23 compiler
 *
 * @brief Error handling header file
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */


#ifndef IFJ_QUEUE_H
#define IFJ_QUEUE_H

#include "scanner.h"
#include "error.h"

typedef struct queue_item {
    token_t *token;
    struct queue_item *next;
} queue_item_t;

typedef struct queue {
    queue_item_t *first;
    queue_item_t *last;
} queue_t;



void init_queue(queue_t *queue);
void queue_push(queue_t *queue, token_t *token);
token_t *queue_pop(queue_t *queue);
void queue_dispose(queue_t *queue);
void validate_and_insert (queue_t *queue);


#endif //IFJ_QUEUE_H
