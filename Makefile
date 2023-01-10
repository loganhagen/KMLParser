UNAME := $(shell uname)
CC = gcc
CFLAGS = -Wall -std=gnu99 -g
LDFLAGS= -L.
INC = include/
SRC = src/
BIN = bin/
PARSER_SRC_FILES = $(wildcard src/KML*.c)
PARSER_OBJ_FILES = $(patsubst src/KML%.c,bin/KML%.o,$(PARSER_SRC_FILES))

ifeq ($(UNAME), Linux)
	XML_PATH = /usr/include/libxml2
endif
ifeq ($(UNAME), Darwin)
	XML_PATH = /System/Volumes/Data/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/libxml2
endif

parser: $(BIN)libkmlparser.so

$(BIN)libkmlparser.so: $(PARSER_OBJ_FILES) $(BIN)LinkedListAPI.o
	gcc -shared -o $(BIN)libkmlparser.so $(PARSER_OBJ_FILES) $(BIN)LinkedListAPI.o -lxml2 -lm

#Compiles all files named KML*.c in src/ into object files, places all coresponding KML*.o files in bin/
$(BIN)KML%.o: $(SRC)KML%.c $(INC)LinkedListAPI.h $(INC)KML*.h
	gcc $(CFLAGS) -I$(XML_PATH) -I$(INC) -c -fpic $< -o $@

$(BIN)liblist.so: $(BIN)LinkedListAPI.o
	$(CC) -shared -o $(BIN)liblist.so $(BIN)LinkedListAPI.o

$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic -I$(INC) $(SRC)LinkedListAPI.c -o $(BIN)LinkedListAPI.o

clean:
	rm -rf $(BIN)StructListDemo $(BIN)xmlExample $(BIN)*.o $(BIN)*.so