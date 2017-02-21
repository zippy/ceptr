all: clean ceptr test
.PHONY: all

CEPTR_SRC_FILES := $(wildcard src/*.c)
CEPTR_SRCH_FILES := $(wildcard src/*.h)
SPECS_SRC_FILES := $(wildcard spec/*.c)
SPECS_SRCH_FILES := $(wildcard src/*.h src/*.c)
SPECS_SRC_FILES := $(filter-out src/ceptr.c, $(SPECS_SRC_FILES))

ceptr: $(CEPTR_SRCH_FILES) $(CEPTR_SRC_FILES:.c=.o)
	gcc -pthread -g -o ceptr $(CEPTR_SRC_FILES:.c=.o)

test: ceptr_specs
	./ceptr_specs

ceptr_specs: $(SPECS_SRC_FILES) $(SPECS_SRCH_FILES)
	gcc -pthread -g -o ceptr_specs $(SPECS_SRC_FILES:.c=.o)

clean:
	-rm -rf ceptr_specs *.o ceptr_spec.dSYM ceptr ceptr.dSYM #src/base_defs.c src/base_defs.h

#.PHONY: bdefs
#bdefs: src/base_defs
#	perl src/base_defs.pl
