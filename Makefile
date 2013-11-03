ceptr: clean test

test: src/*.h src/*.c
	gcc -g -o ceptr_spec src/*.h src/*.c && ./ceptr_spec

clean:
	rm -rf ceptr_spec *.o ceptr_spec.dSYM
