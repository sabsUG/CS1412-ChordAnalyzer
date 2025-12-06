#ifndef DICTADT_H
#define DICTADT_H

/* Dictionary type, implemented on top of a trie (project extra credit). */
typedef struct dict_type *Dict;

/* Build dictionary from file with lines "WORD.MEANINGS". */
Dict create_dict(char *filename);

/* Meanings for longest prefix of word found in the dictionary. */
char *search(Dict d, char *word);

#endif