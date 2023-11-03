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


#include "queue.h"

void init_queue(queue_t *queue) {
    q->first = NULL;
    q->last = NULL;
}

void queue_push(queue_t *queue, token_t *token){
    queue_item_t *new_token = allocate_memory(sizeof(queue_item_t *), "temporary token pointer", BASIC);
    if (new_token == NULL) {
        error_exit(ERROR_INTERNAL, "Queue", "Could not allocate memory for temporary token pointer");
    }
    if (queue->first == NULL) {
        queue->first = new_token;
        queue->last = new_token;
    } else {
        queue->last->next = new_token;
        queue->last = new_token;
    }
    new_token->next = NULL;
    new_token->token = token;
}

token_t *queue_pop(queue_t *queue) {
    if (queue->first == NULL) {
        return NULL;
    }
    queue_item_t *tmp = queue->first;
    queue->first = queue->first->next;
    token_t *token = tmp->token;
    free(tmp);
    tmp = NULL;
    return token;
}

void queue_dispose(queue_t *queue) {
    while (queue->first != NULL) {
        queue_item_t *tmp = queue->first;
        queue->first = queue->first->next;
        free(tmp);
    }
    queue->last = NULL;
}

/// CHCE TO VIC PROMYSLET ABY SE TO DALO OBECNE POUZIT
void validate_and_insert (queue_t *queue){

}
