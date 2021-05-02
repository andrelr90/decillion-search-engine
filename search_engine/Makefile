CC=g++
CFLAGS=-Wall -Wextra #-fsanitize=address,undefined -fno-omit-frame-pointer -g
EXEC=search_engine_backend.out
LIBS=`pkg-config --cflags --libs gumbo` -lfcgi++ -lfcgi
BIN=bin
SRC=src
INCLUDE=include

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(patsubst %/,%,$(dir $(mkfile_path)))

$(EXEC): $(INCLUDE)/ $(SRC)/tp8.cpp $(BIN)/htmlParser.o $(BIN)/invertedList.o $(BIN)/queryProcessor.o
	$(CC) $(CFLAGS) -I $(INCLUDE)/ $(SRC)/tp8.cpp $(BIN)/htmlParser.o $(BIN)/invertedList.o $(BIN)/queryProcessor.o -o $(EXEC) $(LIBS)

$(BIN)/htmlParser.o: $(INCLUDE)/htmlParser.hpp $(SRC)/htmlParser.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE)/ -c $(SRC)/htmlParser.cpp -o $(BIN)/htmlParser.o $(LIBS)

$(BIN)/invertedList.o: $(INCLUDE)/invertedList.hpp $(SRC)/invertedList.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE)/ -c $(SRC)/invertedList.cpp -o $(BIN)/invertedList.o $(LIBS)

$(BIN)/queryProcessor.o: $(INCLUDE)/queryProcessor.hpp $(SRC)/queryProcessor.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE)/ -c $(SRC)/queryProcessor.cpp -o $(BIN)/queryProcessor.o $(LIBS)




run:
	spawn-fcgi -p 8000 -n $(EXEC)

config:
	sudo nginx -c $(current_dir)/conf

stop:
	sudo systemctl stop nginx

start:
	sudo systemctl start nginx

info:
	ps ax | grep nginx # you can use "sudo kill process ID" to kill it if stop doesn't work

status:
	sudo systemctl status nginx #q to quit



clean:
	rm -rf $(BIN)/*; \
	touch $(BIN)/.gitignore