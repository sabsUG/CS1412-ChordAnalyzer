#ifndef TRIE_H
#define TRIE_H

/* Trie node for the chords dictionary (base-12 words: 0–9, A, B). */
typedef struct trie_node {
    struct trie_node *children[12];
    char *meaning;  /* concatenated meanings separated by ';' */
} TrieNode;

/* Create an empty trie node. */
TrieNode *trie_create_node(void);

/* Insert a word and its meanings (extra-credit trie implementation). */
void trie_insert(TrieNode *root, const char *word, const char *meaning);

/* Return meanings for the longest prefix of word that exists in the trie. */
char *trie_search_longest_prefix(TrieNode *root, const char *word);

#endif
