#ifndef DICTADT_H
#define DICTADT_H

typedef struct dict_type *Dict;

Dict create_dict(char *filename);

char *search(Dict, char *word);

#endif
