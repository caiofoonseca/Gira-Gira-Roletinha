#ifndef CATEGORIES_H
#define CATEGORIES_H

// Nó para lista circular de categorias
typedef struct Node {
    char *categoria;
    struct Node *next;
} Node;

Node *initCategories(void);

char *spinCategory(Node *head);

int countCategories(Node *head);

const char *getCategoryByIndex(Node *head, int index);

void destroyCategories(Node *head);

#endif