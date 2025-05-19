#include <stdlib.h>
#include <string.h>
#include "categories.h"

static const char *DEFAULT_CATEGORIES[] = {
  "História","Geografia","Ciências","Esportes","Arte", "Entretenimento"
};
static const int DEFAULT_COUNT = sizeof(DEFAULT_CATEGORIES)/sizeof(DEFAULT_CATEGORIES[0]);

Node *initCategories(void) {
  Node *head = NULL, *tail = NULL;
  for (int i = 0; i < DEFAULT_COUNT; i++) {
    Node *n = malloc(sizeof(Node));
    n->categoria = strdup(DEFAULT_CATEGORIES[i]);
    n->next = NULL;
    if (!head) head = n;
    else       tail->next = n;
    tail = n;
  }
  if (tail) tail->next = head;
  return head;
}

char *spinCategory(Node *head) {
  int n = countCategories(head);
  if (n == 0) return NULL;
  int steps = rand() % n;
  Node *cur = head;
  for (int i = 0; i < steps; i++) cur = cur->next;
  return cur->categoria;
}

int countCategories(Node *head) {
  if (!head) return 0;
  int cnt = 1;
  Node *cur = head->next;
  while (cur != head) {
    cnt++;
    cur = cur->next;
  }
  return cnt;
}

const char *getCategoryByIndex(Node *head, int index) {
  int n = countCategories(head);
  if (n == 0) return NULL;
  index %= n;
  Node *cur = head;
  for (int i = 0; i < index; i++) cur = cur->next;
  return cur->categoria;
}

void destroyCategories(Node *head) {
  if (!head) return;
  Node *cur = head->next;
  while (cur != head) {
    Node *next = cur->next;
    free(cur->categoria);
    free(cur);
    cur = next;
  }
  free(head->categoria);
  free(head);
}
