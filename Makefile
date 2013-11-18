ceptr: clean specs

specs: ceptr_specs
	./ceptr_specs

ceptr_specs: src/*.h src/*.c
	gcc -g -o ceptr_specs src/*.c

clean:
	-rm -rf ceptr_specs *.o ceptr_spec.dSYM
