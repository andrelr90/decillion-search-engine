CC=g++
CFLAGS=-Wall -Wextra# -fsanitize=address,undefined -fno-omit-frame-pointer -g
EXEC=./indexer
LIBS=`pkg-config --cflags --libs gumbo`
BIN=bin
SRC=src
INCLUDE=include

$(EXEC): $(INCLUDE)/ $(SRC)/main.cpp $(BIN)/htmlParser.o $(BIN)/invertedList.o
	$(CC) $(CFLAGS) -I $(INCLUDE)/ $(SRC)/main.cpp $(BIN)/htmlParser.o $(BIN)/invertedList.o -o $(EXEC) $(LIBS)

$(BIN)/htmlParser.o: $(INCLUDE)/htmlParser.hpp $(SRC)/htmlParser.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE)/ -c $(SRC)/htmlParser.cpp -o $(BIN)/htmlParser.o $(LIBS)

$(BIN)/invertedList.o: $(INCLUDE)/invertedList.hpp $(SRC)/invertedList.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE)/ -c $(SRC)/invertedList.cpp -o $(BIN)/invertedList.o $(LIBS)

run:
	$(EXEC)

clean:
	rm -rf $(BIN)/*; \
	touch $(BIN)/.gitignore
