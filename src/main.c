/**
 * @file main.c
 * 
 * IFJ23 compiler
 * 
 * @brief Main function of IFJ23 compiler
 *
 * @author Adam Valik <xvalik05>
 */

//#include "parser.h"
#include "error.h"
#include "forest.h"

alloc_ptr *allocated_pointers = NULL; // Top of the stack for allocated pointers
forest_node *active = NULL; // Pointer to the active node in the forest



int main() { 
    sym_data data = {0};
    forest_node *global = forest_insert_global();
    active = global;

    //parser();

    free_alloc_memory(); // Free all allocated memory
    return 0;
}