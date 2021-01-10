
SOURCE := src/ffo.c
INCLUDE := include/ffo.h

ffo: $(SOURCE) $(INCLUDE)
	gcc -I include -o ffo $(SOURCE)

clean:
	rm -f ffo
