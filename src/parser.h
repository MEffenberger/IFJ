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

#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H

#include "error.h"
#include "scanner.h"
#include "symtable.h"
#include "forest.h"

#define MAX_TOKENS 7


extern token_t *current_token; // Pointer to the current token
extern token_t *token_buffer[MAX_TOKENS]; // Buffer for tokens
extern token_buffer_cnt; // Index of the last token in the buffer


#endif //IFJ_PARSER_H
