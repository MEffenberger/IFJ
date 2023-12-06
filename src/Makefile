###
 # @file Makefile
 #
 # IFJ23 compiler
 #
 # @brief Makefile for IFJ23 compiler
 #
 # @author Adam Valík <xvalik05>
 ##


EXEC = compiler
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,$(SRC))

CC = gcc
CFLAGS = -std=c11

.PHONY: all clean pack doc

.DEFAULT_GOAL := all

all: $(EXEC) 

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o $(EXEC) 

pack: 
	@make clean
	@mkdir pack
	@cp *.c *.h Makefile pack
	@cp ../doc/dokumentace.pdf pack
	@cp ../rozdeleni pack
	@cp ../rozsireni pack
	@(cd pack && zip ../../xeffen00.zip *)
	@rm -rf pack

doc:
	doxygen Doxyfile
