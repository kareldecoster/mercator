IDIR = ./include
CFLAGS = -Wall -Werror -g -I$(IDIR)
CC = gcc

ODIR = ./src/obj

_DEPS =  mercator_processing_unit.h mercator_data_acquisition_unit.h config.h#.h files
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = mercator_processing_unit.o mercator.o mercator_data_acquisition_unit.o#.o files
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_TEST_OBJ = mercator_processing_unit.o mercator_test.o mercator_data_acquisition_unit.o#.o files
TEST_OBJ = $(patsubst %,$(ODIR)/%,$(_TEST_OBJ))

_OBJ_1 = mercator_processing_unit.o mercator_data_acquisition_unit.o foreground_mercator.o#.o files
OBJ_1 = $(patsubst %,$(ODIR)/%,$(_OBJ_1))

$(ODIR)/%.o: ./src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	

all: $(OBJ)
	make mercator
	make test
	make foreground_mercator
	
mercator: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lfftw3 -lm -lpthread -lprussdrv
	
test: $(TEST_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lfftw3 -lm -lcheck -lpthread -lprussdrv `pkg-config --cflags --libs check`
	
foreground_mercator: $(OBJ_1)
	$(CC) -o $@ $^ $(CFLAGS) -lfftw3 -lm -lpthread -lprussdrv
	
clean: 
	$ rm -f $(ODIR)/*.o core $(INCDIR)/*~
	$ rm -f ./mercator
	$ rm -f ./test
	$ rm -f ./foreground_mercator
	$ clear
	
	
.PHONY: clean
