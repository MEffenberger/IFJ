/**
 * @file main.c
 * 
 * IFJ23 compiler
 * 
 * @brief Main function of IFJ23 compiler
 *
 * @author Marek Effenberger <xeffen00>
 * @author Adam Valík <xvalik05>
 * @author Samuel Hejnicek <xhejni00>
 * @author Dominik Horut <xhorut01>
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


alloc_ptr *allocated_pointers = NULL; // Top of the stack for allocated pointers
FILE *file;

int main() {
    file = fopen("code.txt", "w");

    codegen_generate_code_please();
    return parser_parse_please();
}
