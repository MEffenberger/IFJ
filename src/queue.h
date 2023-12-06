/**
 * @file queue.h
 *
 * IFJ23 compiler
 *
 * @brief Queue implementation for tokens
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */


#ifndef IFJ_QUEUE_H
#define IFJ_QUEUE_H

#include <stdbool.h>
#include "scanner.h"

// Structure for queue item
typedef struct queue_item {
    token_t *token;
    struct queue_item *next;
} queue_item_t;

// Structure for queue
typedef struct queue {
    queue_item_t *first;
    queue_item_t *last;
} queue_t;

/**
 * @brief Initializes the queue
 * 
 * @param queue Pointer to the queue
 */
void init_queue(queue_t *queue);

/**
 * @brief Pushes a token to the queue
 * 
 * @param queue Pointer to the queue
 * @param token Pointer to the token
 */
void queue_push(queue_t *queue, token_t *token);

/**
 * @brief Cleaning of the queue
 * 
 * @param queue Pointer to the queue
 */
void queue_dispose(queue_t *queue);

#endif //IFJ_QUEUE_H
