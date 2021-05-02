CC=g++
CFLAGS=-Wall -Wextra -g
EXEC=./crawler
LIBS=-lchilkat-9.5.0 -lresolv -lpthread
CHILKAT=/home/andre/Documentos/chilkat-9.5.0-x86_64-linux-gcc#link for chilkat lib
INCLUDE=include
SRC=src
BIN=bin

$(EXEC): $(SRC)/crawler.cpp $(BIN)/urlCrawler.o $(BIN)/log.o
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	$(CC) $(CFLAGS) -I $(INCLUDE)/ $(SRC)/crawler.cpp $(BIN)/urlCrawler.o $(BIN)/log.o -I $(CHILKAT)/include -L $(CHILKAT)/lib $(LIBS) -o $(EXEC)

$(BIN)/urlCrawler.o: $(INCLUDE)/urlCrawler.hpp $(SRC)/urlCrawler.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE)/ -c $(SRC)/urlCrawler.cpp -o $(BIN)/urlCrawler.o $(LIBS)

$(BIN)/log.o: $(INCLUDE)/log.hpp $(SRC)/log.cpp
	$(CC) $(CFLAGS) -I $(INCLUDE)/ -c $(SRC)/log.cpp -o $(BIN)/log.o $(LIBS)



run:
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	$(EXEC) $(seed) $(threads) $(limit) $(individual_files) > data/log

run_default:
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	$(EXEC) seeds 50 10000 false > data/log

debug:
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	gdb $(EXEC) $(seed)

clean:
	rm -rf $(BIN)/*; \
	touch $(BIN)/.gitignore

#Running example: make run seed="seeds" threads="50" limit="10000" individual_files="false"