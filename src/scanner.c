/**
 * @file scanner.c
 * 
 * IFJ23 compiler
 * 
 * @brief Scanner implementation
 *
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

#define ASCII_BEGIN 31
#define DEFAULT_TOKEN_VAL 1000

keyword_t compare_keyword(vector* v){
    if(vector_str_cmp(v, "Double")){
        return KW_DOUBLE;
    } else if (vector_str_cmp(v, "else")){
        return KW_ELSE;
    } else if (vector_str_cmp(v, "func")){
        return KW_FUNC;
    } else if (vector_str_cmp(v, "if")){
        return KW_IF;
    } else if (vector_str_cmp(v, "Int")){
        return KW_INT;
    } else if (vector_str_cmp(v, "let")){
        return KW_LET;
    } else if (vector_str_cmp(v, "nil")){
        return KW_NIL;
    } else if (vector_str_cmp(v, "return")){
        return KW_RETURN;
    } else if (vector_str_cmp(v, "String")){
        return KW_STRING;
    } else if (vector_str_cmp(v, "var")){
        return KW_VAR;
    } else if (vector_str_cmp(v, "while")){
        return KW_WHILE;

    } else if (vector_str_cmp(v, "readString")){
        return KW_RD_STR;
    } else if (vector_str_cmp(v, "readInt")){
        return KW_RD_INT;
    } else if (vector_str_cmp(v, "readDouble")){
        return KW_RD_DBL;
    } else if (vector_str_cmp(v, "write")){
        return KW_WRT;
    } else if (vector_str_cmp(v, "Int2Double")){
        return KW_INT_2_DBL;
    } else if (vector_str_cmp(v, "Double2Int")){
        return KW_DBL_2_INT;
    } else if (vector_str_cmp(v, "length")){
        return KW_LENGHT;
    } else if (vector_str_cmp(v, "substring")){
        return KW_SUBSTR;
    } else if (vector_str_cmp(v, "ord")){
        return KW_ORD;
    } else if (vector_str_cmp(v, "chr")){
        return KW_CHR;
    } else {
        return DEFAULT_TOKEN_VAL;
    }
}

bool check_indent(int* cnt_array, int size){
    for(int i = 0; i < size ; i++){
        if(cnt_array[i] < cnt_array[size]){
            return false;
        }
    }
    return true;
}

void destroy_token(token_t* token){
    if(token->value.vector){
        vector_dispose(token->value.vector);
    }
    free(token);
    token = NULL;
}



token_t* get_me_token(){
    token_t* token = malloc(sizeof(token_t));
    //push_alloc_ptr(token, TOKEN);
    char readchar, next_char;
    automat_state_t a_state = S_START;
    vector* buffer = vector_init();
    token->value.vector = NULL;
    token->prev_was_eol = false;
    token->value.integer = 0;
    token->value.type_double = 0.0;
    token->value.keyword = DEFAULT_TOKEN_VAL;
    char hex[8] = {0};
    int hex_counter = 0;
    int cnt_open = 0;
    int cnt_close = 0;

    int cnt_array_size = 0;
    int cnt_array_alloc_size = 8;
    int* cnt_array = NULL;
    bool is_multiline = false;


    while ((readchar = (char) getc(stdin))){

        switch(a_state)
        {
            case (S_START):

                    if(readchar == ':'){
                        token->type = TOKEN_COLON;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == ','){
                        token->type = TOKEN_COMMA;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '?'){
                        a_state = S_QM;
                        vector_dispose(buffer);
                        //break;

                    } else if(readchar == '('){
                        a_state = S_START;
                        token->type = TOKEN_LPAR;
                        vector_dispose(buffer);
                        
                        return token;
                    } else if(readchar == ')'){
                        a_state = S_START;
                        token->type = TOKEN_RPAR;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '{'){
                        a_state = S_START;
                        token->type = TOKEN_LEFT_BRACKET;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '}'){
                        a_state = S_START;
                        token->type = TOKEN_RIGHT_BRACKET;
                        vector_dispose(buffer);
                        return token;

                    }else if(readchar == '>'){
                        token->type = TOKEN_GREAT;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;
                            token->type = TOKEN_GREAT_EQ;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '<'){
                        token->type = TOKEN_LESS;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;
                            token->type = TOKEN_LESS_EQ;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '='){
                        token->type = TOKEN_EQ;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;
                            token->type = TOKEN_EQEQ;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '!'){
                        token->type = TOKEN_EXCLAM;

                        if((next_char = (char) getc(stdin)) == '='){
                            a_state = S_START;

                            token->type = TOKEN_EXCLAMEQ;
                            vector_dispose(buffer);
                             return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;    

                    } else if(readchar == '*'){
                        a_state = S_START;
                        token->type = TOKEN_MULTIPLY;
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == '+'){
                        a_state = S_START;
                        token->type = TOKEN_PLUS;
                        vector_dispose(buffer);
                        return token;
                    
                    } else if(readchar == '/'){
                        next_char = (char) getc(stdin);
                        if(next_char == '*'){
                            cnt_open++;
                            a_state = S_NESTED_COM;
                            break;
                        } else if(next_char == '/'){
                            a_state = S_SL_COM;
                            break;
                        } else {
                            ungetc(next_char, stdin);
                            token->type = TOKEN_DIVIDE;
                            vector_dispose(buffer);
                            return token;
                        }
                    } else if(readchar == '_'){
                        a_state = S_START;
                        token->type = TOKEN_UNDERSCORE;
                        vector_append(buffer, readchar);
                        token->value.vector = buffer;

                        if((next_char = (char) getc(stdin)) == '_' || isalpha(next_char) || isdigit(next_char)){
                            a_state = S_ID;

                            vector_append(buffer, readchar);
                            vector_append(buffer, next_char);
                            break;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        return token;     

                    } else if(readchar == '-'){
                        token->type = TOKEN_MINUS;

                        if((next_char = (char) getc(stdin)) == '>'){
                            a_state = S_START;

                            token->type = TOKEN_RET_TYPE;
                            vector_dispose(buffer);
                            return token;
                        } else {
                            ungetc(next_char, stdin);
                        }
                        vector_dispose(buffer);
                        return token;

                    } else if(readchar == ' '){
                        continue;
                        
                    } else if(readchar == '\t'){
                        continue;
                        
                    } else if(readchar == '\n'){
                        token->type = TOKEN_EOL;
                        vector_dispose(buffer);
                        return token;

                    } else if((int)readchar == EOF){
                        token->type = TOKEN_EOF;
                        vector_dispose(buffer);
                        return token;

                    } else if(isalpha(readchar)){
                        a_state = S_ID;
                        vector_append(buffer, readchar);
                        break;
                    } else if(isdigit(readchar)){
                        a_state = S_NUM;
                        vector_append(buffer, readchar);
                        break;
                    } else if(readchar == '"'){
                        next_char = (char) getc(stdin);
                        if(next_char == '"'){
                            a_state = S_STR_EMPTY;
                            break;
                        } else {
                            ungetc(next_char, stdin);
                            a_state = S_START_QUOTES;
                            break;
                        }
                    }

            case(S_QM):
                if((next_char = (char) getc(stdin)) == '?'){
                    a_state = S_START;
                    token->type = TOKEN_DOUBLE_QM;
                    return token;
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_ID):
                if(isalpha(readchar) || isdigit(readchar) || readchar == '_'){
                    vector_append(buffer, readchar);
                    break;

                } else if(readchar == '?'){
                    keyword_t key = compare_keyword(buffer);
                    if(key < 3){
                        vector_append(buffer, readchar);
                        token->type = TOKEN_KEYWORD_QM;
                        token->value.keyword = key;
                        token->value.vector = buffer;
                        return token;
                    } else if (key > 3 && key != DEFAULT_TOKEN_VAL){
                        ungetc(readchar, stdin);
                        a_state = S_QM;
                        token->type = TOKEN_KEYWORD;
                        token->value.keyword = key;
                        token->value.vector = buffer;
                        return token;
                    } else  {
                        ungetc(readchar, stdin);
                        a_state = S_QM;
                        token->type = TOKEN_ID;
                        token->value.vector = buffer;
                        return token;
                    }
                } else {
                    ungetc(readchar, stdin);
                    keyword_t key = compare_keyword(buffer);
                    if(key != DEFAULT_TOKEN_VAL){
                        token->type = TOKEN_KEYWORD;
                        token->value.keyword = key;
                        token->value.vector = buffer;
                        return token;
                    } else {
                        token->type = TOKEN_ID;
                        token->value.vector = buffer;
                        return token;
                    }
                }
            
            case(S_NUM):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    break;
                } else if(readchar == '.'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_DOT;
                    break;
                } else if(readchar == 'e' || readchar == 'E'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_E;
                    break;
                } else {
                    ungetc(readchar, stdin);
                    token->type = TOKEN_NUM;
                    if(sscanf(buffer->array, "%d", &token->value.integer) != EOF){
                        vector_dispose(buffer);
                        return token;
                    } else {
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    }
                }

            case(S_NUM_DOT):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    a_state = S_DEC;
                    break;
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            
            case(S_DEC):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    break;
                } else if(readchar == 'e' || readchar == 'E'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_E;
                    break;
                } else {
                    ungetc(readchar, stdin);
                    token->type = TOKEN_DEC;
                    if(sscanf(buffer->array, "%lf", &token->value.type_double) != EOF){
                        vector_dispose(buffer);
                        return token;
                    } else {
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    }
                }
            case(S_NUM_E):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    a_state = S_EXP;
                    break;
                } else if(readchar == '+' || readchar == '-'){
                    vector_append(buffer, readchar);
                    a_state = S_NUM_E_SIGN;
                    break;
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_NUM_E_SIGN):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    a_state = S_EXP;
                    break; 
                } else {
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_EXP):
                if(isdigit(readchar)){
                    vector_append(buffer, readchar);
                    break;
                } else {
                    ungetc(readchar, stdin);
                    token->type = TOKEN_EXP;
                    if(sscanf(buffer->array, "%lf", &token->value.type_double) != EOF){
                        vector_dispose(buffer);
                        return token;
                    } else {
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    }

                }
            case(S_START_QUOTES):
                if(readchar > ASCII_BEGIN && readchar != '\n' && readchar != '\\' && readchar != '"'){
                    vector_append(buffer, readchar);
                    break;
                } else if(readchar == '"'){
                    a_state = S_END_QUOTES;
                    token->type = TOKEN_STRING;
                    token->value.vector = buffer;
                    return token;
                } else if(readchar == '\\'){
                    a_state = S_START_ESC_SENTENCE;
                    //vector_append(buffer, readchar);
                    break;
                } else {
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_START_ESC_SENTENCE):
                if(readchar == '"' || readchar == '\\'){
                    if(is_multiline){
                        a_state = S_IS_MULTILINE;
                    } else {
                        a_state= S_START_QUOTES;
                    } 
                    vector_append(buffer, readchar);
                    break;
                } else if(readchar == 'n'){
                    if(is_multiline){
                        a_state = S_IS_MULTILINE;
                    } else {
                        a_state= S_START_QUOTES;
                    } 
                    vector_append(buffer, '\n');
                    break;
                } else if(readchar == 't'){
                    if(is_multiline){
                        a_state = S_IS_MULTILINE;
                    } else {
                        a_state= S_START_QUOTES;
                    } 
                    vector_append(buffer, '\t');
                    break;
                } else if(readchar == 'r'){
                    int buffer_size = vector_size(buffer);
                    if(buffer_size == 0){
                    if(is_multiline){
                        a_state = S_IS_MULTILINE;
                    } else {
                        a_state= S_START_QUOTES;
                    } 
                        break;
                    } else if(buffer_size != 0){
                        next_char = (char) getc(stdin);
                        if(next_char == '"'){
                            ungetc(next_char, stdin);
                            if(is_multiline){
                                a_state = S_IS_MULTILINE;
                            } else {
                                a_state= S_START_QUOTES;
                            } 
                            break;
                        } else {
                            ungetc(next_char, stdin);
                            if(is_multiline){
                                a_state = S_IS_MULTILINE;
                            } else {
                                a_state= S_START_QUOTES;
                            } 
                            vector_append(buffer, '\n');
                            break;
                        }
                    }
                } else if(readchar == 'u'){
                    a_state = S_START_HEX;
                    //vector_append(buffer, readchar);
                    break;
                } else {
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_START_HEX):
                if(readchar == '{'){
                    a_state = S_LEFT_BRACKET;
                    break;
                } else {
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_LEFT_BRACKET):
                if(isdigit(readchar) || isalpha(readchar)){
                    hex[hex_counter] = readchar;
                    hex_counter++;
                    a_state = S_FIRST_HEX;
                    break;
                } else {
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            case(S_FIRST_HEX):
                if(isdigit(readchar) || isalpha(readchar)){

                    if(hex_counter == 8){
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    }

                    hex[hex_counter] = readchar;
                    hex_counter++;
                    a_state = S_FIRST_HEX;
                    break;
                } else if(readchar == '}'){
                    int hex_num = 0;
                    if(sscanf(hex, "%x", &hex_num) != EOF){
                        char* c = (char*) &hex_num;
                        vector_append(buffer, c[0]);
                        if(is_multiline){
                            a_state = S_IS_MULTILINE;
                        } else {
                            a_state = S_START_QUOTES;
                        }
                        break;
                    } else {
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    }

                } else {
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }
            
            case(S_SL_COM):
                if(readchar == '\n'){
                    a_state = S_START;
                    break;
                } else if((int) readchar == EOF){
                    a_state = S_START;
                    break;
                } else {
                    break;
                }
            case(S_NESTED_COM):

                if(((int) readchar == EOF) && (cnt_open != cnt_close)){
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                }

                if(readchar == '/'){
                    next_char = (char) getc (stdin);
                    if(next_char == '*'){
                        cnt_open++;
                        break;
                    } else {
                        ungetc(next_char, stdin);
                        break;
                    }
                } else if(readchar == '*'){
                    next_char = (char) getc (stdin);
                    if(next_char == '/'){
                        a_state = S_NESTED_END;
                        cnt_close++;
                        break;
                    } else {
                        ungetc(next_char, stdin);
                        break;
                    }
                } else {
                    break;
                }

            case(S_NESTED_END):
                if(cnt_open == cnt_close){
                    a_state = S_START;
                    break;
                } else if(readchar == '*'){
                    next_char = (char) getc (stdin);
                    if(next_char == '/'){
                        cnt_close++;
                        break;
                    } else {
                        ungetc(next_char, stdin);
                        a_state = S_NESTED_COM;
                        break;
                    }
                } else {
                    a_state = S_NESTED_COM;
                    break;
                }
            case(S_STR_EMPTY):
                if(readchar != '"'){
                    //Syntax error nemuze byt nic po dvou uvozovkach?
                    ungetc(readchar, stdin);
                    a_state = S_START;
                    token->type = TOKEN_STRING;
                    token->value.vector = buffer;
                    return token;
                } else {
                    next_char = (char) getc(stdin);
                    if(next_char == '\n'){
                        a_state = S_START_MULTILINE;
                        is_multiline = true;
                        cnt_array = malloc(8*sizeof(int));
                        for(int i =0; i < 8; i++){
                            cnt_array[i] = 0;
                        }
                        break;
                    } else {
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                    }
                }

            case(S_START_MULTILINE):
                
                if(cnt_array_size + 1 == cnt_array_alloc_size){
                    int *new_vec = realloc(cnt_array, cnt_array_alloc_size * 2 * sizeof(int));
                    if(!(new_vec)){
                        return false;
                    }
                    cnt_array = new_vec;
                    cnt_array_alloc_size *= 2;
                    for(int i = cnt_array_size +1; i < cnt_array_alloc_size; i++){
                        cnt_array[i] = 0;
                    }
                }

                if(readchar == ' '){
                    cnt_array[cnt_array_size]++;
                    vector_append(buffer, readchar);
                    break;
                } else if(readchar == '"'){
                    vector_append(buffer, readchar);
                    next_char = (char) getc (stdin);
                    if(next_char == '"'){
                        vector_append(buffer, next_char);
                        a_state = S_END_MULTILINE;
                        break;
                    } else {
                        free(cnt_array);
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    }
                } else if ((int) readchar == EOF) {
                    free(cnt_array);
                    vector_dispose(buffer);
                    free(token);
                    token = NULL;
                    exit(ERROR_LEX);
                } else if(readchar == '\\'){
                    a_state = S_START_ESC_SENTENCE;
                    break;
                } else {
                    a_state = S_IS_MULTILINE;
                    vector_append(buffer, readchar);
                    break;
                }

            case(S_IS_MULTILINE):

                if(readchar == '\n'){
                    vector_append(buffer,'\n');
                    a_state = S_START_MULTILINE;
                    cnt_array_size++;
                    break;
                } else if(readchar == '"'){
                    vector_append(buffer, readchar);
                    next_char = (char) getc (stdin);
                    if(next_char == '"'){
                        vector_append(buffer, next_char);
                        a_state = S_FAKE_END_MULTILINE;
                        break;
                    } else {
                        free(cnt_array);
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    }
                } else if(readchar == '\\'){
                    a_state = S_START_ESC_SENTENCE;
                    break;
                } else {
                    vector_append(buffer, readchar);
                    break;
                }

            case(S_END_MULTILINE):
                if(readchar == '"'){
                    next_char = (char) getc(stdin);
                    if(next_char == '"'){
                        free(cnt_array);
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                    } else {
                        ungetc(next_char, stdin);
                        if(check_indent(cnt_array, cnt_array_size)){
                            buffer->array[buffer->size-1] = '\0';
                            buffer->array[buffer->size-2] = '\0';
                            a_state = S_START;
                            token->type = TOKEN_ML_STRING;
                            token->value.vector = buffer;
                            free(cnt_array);
                            is_multiline=false;
                            return token;
                        } else {
                            free(cnt_array);
                            vector_dispose(buffer);
                            free(token);
                            token = NULL;
                            exit(ERROR_LEX);
                        }
                    }
                } else if(readchar == '\n'){
                    a_state = S_START_MULTILINE;
                    cnt_array_size++;
                    vector_append(buffer, '\n');
                    break;
                } else {
                    a_state = S_IS_MULTILINE;
                    vector_append(buffer, readchar);
                    break;
                }
            
            case(S_FAKE_END_MULTILINE):
                if(readchar == '"'){
                        free(cnt_array);
                        vector_dispose(buffer);
                        free(token);
                        token = NULL;
                        exit(ERROR_LEX);
                } else if(readchar == '\n'){
                    a_state = S_START_MULTILINE;
                    cnt_array_size++;
                    vector_append(buffer, '\n');
                    break;
                } else {
                    a_state = S_IS_MULTILINE;
                    vector_append(buffer, readchar);
                    break;
                }                
            default:
                if((int) readchar == EOF){
                token->type = TOKEN_EOF;
                vector_dispose(buffer);
                return token;
            }

        }
    }
    return NULL;
}