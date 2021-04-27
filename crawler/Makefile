CC=g++
CFLAGS=-Wall -Wextra -g
EXEC=./crawler
LIBS=-lchilkat-9.5.0 -lresolv -lpthread
CHILKAT=/home/andre/Documentos/chilkat-9.5.0-x86_64-linux-gcc#link for chilkat lib

$(EXEC): crawler.cpp
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	$(CC) $(CFLAGS) crawler.cpp -I $(CHILKAT)/include -L $(CHILKAT)/lib $(LIBS) -o $(EXEC)

run:
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	$(EXEC) $(seed) $(threads) $(limit) $(individual_files) > log

run_default:
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	$(EXEC) seeds 50 10000 false > log

debug:
	export LD_LIBRARY_PATH=$(CHILKAT)/lib:$$LD_LIBRARY_PATH; \
	gdb $(EXEC) $(seed)

#Running example: make run seed="seeds" threads="50" limit="10000" individual_files="false"