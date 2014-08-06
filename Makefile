ceptr: clean specs

specs: ceptr_specs
	./ceptr_specs

ceptr_specs: src/*.h src/*.c spec/*.c
	gcc -pthread -g -o ceptr_specs spec/*.c src/*.c

clean:
	-rm -rf ceptr_specs *.o ceptr_spec.dSYM
