#ifndef TRIE_H
#define TRIE_H

struct trie_node {
    struct trie_node *children[12];
    char *meaning;
};

struct trie_node *trie_create_node();
void trie_insert(struct trie_node *root, const char *word, const char *meaning);
char *trie_search_longest_prefix(struct trie_node *root, const char *word);

#endif
