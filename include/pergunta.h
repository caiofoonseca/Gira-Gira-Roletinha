#ifndef PERGUNTA_H
#define PERGUNTA_H

#include <stdbool.h>
#define MAX_QUESTIONS 100

typedef struct {
  char *pergunta;
  char *alternativas[4];
  int respostaCorreta;
} Pergunta;

Pergunta gerarPerguntaComIA(const char *categoria);
void freePergunta(Pergunta *p);

#endif
