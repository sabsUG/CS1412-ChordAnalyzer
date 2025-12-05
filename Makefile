polynizer: polynizer.o dictADT.o trie.o read_file.o process_matrix.o
	gcc -Wall -Wextra -std=c11 -o polynizer polynizer.o dictADT.o trie.o read_file.o process_matrix.o

polynizer.o: polynizer.c dictADT.h trie.h read_file.h process_matrix.h
	gcc -Wall -Wextra -std=c11 -c polynizer.c

dictADT.o: dictADT.c dictADT.h trie.h
	gcc -Wall -Wextra -std=c11 -c dictADT.c

trie.o: trie.c trie.h
	gcc -Wall -Wextra -std=c11 -c trie.c

read_file.o: read_file.c read_file.h
	gcc -Wall -Wextra -std=c11 -c read_file.c

process_matrix.o: process_matrix.c process_matrix.h
	gcc -Wall -Wextra -std=c11 -c process_matrix.c
