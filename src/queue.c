/**
 * @file queue.c
 *
 * IFJ23 compiler
 *
 * @brief Queue implementation for tokens
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 */


#include "callee.h"
#include "cnt_stack.h"
#include "codegen.h"
#include "error.h"
#include "expression_parser.h"
#include "forest.h"
#include "parser.h"
#include "queue.h"
#include "scanner.h"
#include "string_vector.h"
#include "symtable.h"
#include "token_stack.h"

// initialize the queue
void init_queue(queue_t *queue) {
    queue->first = NULL;
    queue->last = NULL;
}

// add the token to the end of the queue
void queue_push(queue_t *queue, token_t *token){
    queue_item_t *new_item = (queue_item_t*)allocate_memory(sizeof(queue_item_t));

    new_item->next = NULL;
    new_item->token = token;

    if (queue->first == NULL) {
        queue->first = new_item;
        queue->last = new_item;
    } 
    else if (queue->first->next == NULL) {
        queue->first->next = new_item;
        queue->last = new_item;
    }
    else {
        queue->last->next = new_item;
        queue->last = new_item;
    }
}

void queue_dispose(queue_t *queue) {
    if (queue != NULL || queue->first != NULL) {
        queue_item_t *current = queue->first;
        while (current != NULL) {
            queue_item_t *tmp = current;
            current = current->next;
            free(tmp);
        }
        queue->first = NULL;
        queue->last = NULL;
        queue = NULL;
    }
}
