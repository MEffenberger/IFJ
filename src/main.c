/**
 * @file main.c
 * 
 * IFJ23 compiler
 * 
 * @brief Main function of IFJ23 compiler
 *
 * @author Adam Valik <xvalik05>
 * @author Jmeno Prijemni <xlogin00>
 * @author Jmeno Prijemni <xlogin00>
 * @author Jmeno Prijemni <xlogin00>
 */

//#include "parser.h"
#include "error.h"

struct AllocatedPointer *allocated_pointers = NULL; // Top of the stack for allocated pointers


int main() { 
    //parser();

    free_alloc_memory(); // Free all allocated memory
    return 0;
}