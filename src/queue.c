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
    queue->first = NULL;
    queue->last = NULL;
}

void queue_push(queue_t *queue, token_t *token){
    printf("QUEUE: Pushing token to queue\n\n");


    queue_item_t *new_item = (queue_item_t*)allocate_memory(sizeof(queue_item_t), "temporary token pointer", BASIC);
    

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
    if (queue != NULL || queue->first != NULL) {
        printf("QUEUE: Disposing queue\n\n");
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

void queue_print(queue_t *queue) {
    printf("QUEUE: Printing queue:");

    if (queue == NULL) {
        printf(" Queue is NULL.\n\n");
        return;
    }
    if (queue->first == NULL) {
        printf(" Queue is empty.\n\n");
        return;
    }
    queue_item_t *tmp = queue->first;
    while (tmp != NULL) {
        if (tmp->token->value.vector->array != NULL) {
            printf(" %s ", tmp->token->value.vector->array);
            tmp = tmp->next;
        }
    }
    printf("\n\n");
}

// /// CHCE TO VIC PROMYSLET ABY SE TO DALO OBECNE POUZITi
// void validate_and_insert (queue_t *queue){

// }
