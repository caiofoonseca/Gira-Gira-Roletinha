#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <stdbool.h>
#include <unistd.h>
#include <json-c/json.h>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600
#define MAX_QUESTIONS 100

typedef enum {
  STATE_MENU,
  STATE_NAME_INPUT,
  STATE_ROULETTE,
  STATE_PLAYING,
  STATE_GAMEOVER,
  STATE_RANKING,
  STATE_EXIT
} GameState;


static char *askedQuestions[MAX_QUESTIONS];
static int   askedCount = 0;

typedef struct Node {
  const char *categoria;
  struct Node *prox;
} Node;

typedef struct {
  const char *pergunta;
  const char *alternativas[4];
  int respostaCorreta;
} Pergunta;

typedef struct {
  char nome[64];
  int pontos;
} Jogador;

void InitBackground(void);
void DrawBackground(void);
void UnloadBackground(void);
void freePergunta(Pergunta *p);
int compararRanking(const void *a, const void *b);
bool nomeExiste(const char *nome);
int encontrarPosicaoRanking(const char *nome);
Node* initCategories();
const char* spinCategory(Node *head);
Pergunta gerarPerguntaComIA(const char *categoria);

static Texture2D bgTexture;
static Vector2   bgPosition;
static float     bgScale;

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gira-Gira Roletinha");
  SetTargetFPS(60);
  InitBackground();
  srand((unsigned)time(NULL));

  GameState state = STATE_MENU;

  bool nomeDuplicado = false;
  bool nomeConfirmado = false;
  bool aguardandoResposta = false;
  bool mostrarFeedback = false;
  bool respostaCorreta = false;
  bool cliqueLiberado = true;

  int pontos = 0;
  char nomeJogador[21] = "\0";
  int nomeLength = 0;
  int letra = 0;

  Node *categorias = initCategories();
  const char *categoriaAtual = NULL;
  Pergunta perguntaAtual = { .pergunta = NULL, .alternativas = {NULL,NULL,NULL,NULL} };

  Rectangle botaoJogar = { SCREEN_WIDTH/2 -100, 200, 200, 50 };
  Rectangle botaoRankingB = { SCREEN_WIDTH/2 -100, 270, 200, 50 };
  Rectangle botaoSair = { SCREEN_WIDTH/2 -100, 340, 200, 50 };
  Rectangle botaoCentral = { SCREEN_WIDTH/2 -100, 300, 200, 50 };
  Rectangle botaoVoltar = { SCREEN_WIDTH/2 -100, 440, 200, 50 };
  Rectangle botoesAlt[4];
  for (int i = 0; i < 4; i++)
    botoesAlt[i] = (Rectangle){ 150, 180 + i*60, 500, 40 };

  while (!WindowShouldClose() && state != STATE_EXIT) {
    Vector2 mouse = GetMousePosition();

    BeginDrawing();
    DrawBackground();

    switch (state) {
      case STATE_MENU: {
        DrawText("Gira-Gira Roletinha",
          SCREEN_WIDTH/2 - MeasureText("Gira-Gira Roletinha",30)/2,
          100, 30, DARKBLUE);

        DrawRectangleRec(botaoJogar, LIGHTGRAY);
        DrawText("JOGAR",
          botaoJogar.x + (200 - MeasureText("JOGAR",20))/2,
          botaoJogar.y + 15, 20, DARKGRAY);

        DrawRectangleRec(botaoRankingB, LIGHTGRAY);
        DrawText("RANKING",
          botaoRankingB.x + (200 - MeasureText("RANKING",20))/2,
          botaoRankingB.y + 15, 20, DARKGRAY);

        DrawRectangleRec(botaoSair, LIGHTGRAY);
        DrawText("SAIR",
          botaoSair.x + (200 - MeasureText("SAIR",20))/2,
          botaoSair.y + 15, 20, DARKGRAY);

        if (CheckCollisionPointRec(mouse, botaoJogar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
          state = STATE_NAME_INPUT;

        if (CheckCollisionPointRec(mouse, botaoRankingB) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          state = STATE_RANKING;
        }

        if (CheckCollisionPointRec(mouse, botaoSair) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          state = STATE_EXIT;
        }
        break;
      }

      case STATE_NAME_INPUT: {
        DrawText("Digite seu nome:",
          SCREEN_WIDTH/2 - 100, 150, 20, DARKGRAY);

        DrawRectangle(SCREEN_WIDTH/2 -150, 180, 300, 40, LIGHTGRAY);
        DrawText(nomeJogador,
          SCREEN_WIDTH/2 -140, 190, 20, DARKBLUE);

        letra = GetCharPressed();
        while (letra > 0) {
          if ((letra >= 32 && letra <= 125) && nomeLength < 20) {
            nomeJogador[nomeLength++] = (char)letra;
            nomeJogador[nomeLength] = '\0';
          }
          letra = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && nomeLength > 0)
          nomeJogador[--nomeLength] = '\0';

        DrawRectangleRec(botaoCentral, LIGHTGRAY);
        DrawText("CONFIRMAR",
          botaoCentral.x + (200 - MeasureText("CONFIRMAR",20))/2,
          botaoCentral.y + 15, 20, DARKGRAY);

        if (CheckCollisionPointRec(mouse, botaoCentral) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          if (nomeLength > 0) {
            if (nomeExiste(nomeJogador)) {
              nomeDuplicado = true;
            } else {
              nomeConfirmado = true;
              nomeDuplicado = false;
              state = STATE_ROULETTE;
            }
          }
        }

        if (nomeDuplicado)
          DrawText("Nome já existe! Escolha outro.",
            SCREEN_WIDTH/2 -170, 240, 20, RED);
        break;
      }

      case STATE_ROULETTE: {
        DrawText(TextFormat("Jogador: %s", nomeJogador),
          20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Pontos: %d", pontos),
          SCREEN_WIDTH-150, 20, 20, DARKGRAY);

        DrawText("Clique para girar a roleta!",
          SCREEN_WIDTH/2 - MeasureText("Clique para girar a roleta!",24)/2,
          200, 24, DARKBLUE);

        DrawRectangleRec(botaoCentral, LIGHTGRAY);
        DrawText("GIRAR ROLETA",
          botaoCentral.x + (200 - MeasureText("GIRAR ROLETA",20))/2,
          botaoCentral.y + 15, 20, DARKGRAY);

        if (cliqueLiberado &&
          CheckCollisionPointRec(mouse, botaoCentral) &&
          IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

          if (perguntaAtual.pergunta) freePergunta(&perguntaAtual);
          if (categoriaAtual) free((void*)categoriaAtual);

          categoriaAtual = strdup(spinCategory(categorias));
          perguntaAtual = gerarPerguntaComIA(categoriaAtual);

          aguardandoResposta = true;
          mostrarFeedback = false;
          respostaCorreta = false;
          cliqueLiberado = false;
          state = STATE_PLAYING;
        }
        break;
      }

      case STATE_PLAYING: {
        DrawText(TextFormat("Jogador: %s", nomeJogador),
          20, 20, 20, DARKGRAY);
        DrawText(TextFormat("Pontos: %d", pontos),
          SCREEN_WIDTH-150, 20, 20, DARKGRAY);

        const int catFontSize = 20;
        const int altFontSize = 20;

        char catText[64];
        sprintf(catText, "Categoria: %s", categoriaAtual);
        int catW = MeasureText(catText, catFontSize);
        int catX = (SCREEN_WIDTH - catW) / 2;
        int catY = 60;
        DrawText(catText, catX, catY, catFontSize, DARKBLUE);

        int qW = MeasureText(perguntaAtual.pergunta, 20);
        int qX = (SCREEN_WIDTH - qW) / 2;
        DrawText(perguntaAtual.pergunta, qX, 110, 20, BLACK);

        for (int i = 0; i < 4; i++) {
          DrawRectangleRec(botoesAlt[i], LIGHTGRAY);

          int altW = MeasureText(perguntaAtual.alternativas[i], altFontSize);
          int textX = botoesAlt[i].x + (botoesAlt[i].width - altW) / 2;
          int textY = botoesAlt[i].y + (botoesAlt[i].height - altFontSize) / 2;

          DrawText(perguntaAtual.alternativas[i], textX, textY, altFontSize, DARKGRAY);
        }

        if (aguardandoResposta && cliqueLiberado) {
          for (int i = 0; i < 4; i++) {
            if (CheckCollisionPointRec(mouse, botoesAlt[i]) &&
              IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
              respostaCorreta = (i == perguntaAtual.respostaCorreta);
              mostrarFeedback = true;
              aguardandoResposta= false;
              if (respostaCorreta) {
                pontos++;
              } else {
                FILE *f = fopen("ranking.txt","a");
                if (f) {
                  fprintf(f, "%s;%d\n", nomeJogador, pontos);
                  fclose(f);
                }
                state = STATE_GAMEOVER;
              }
              cliqueLiberado = false;
            }
          }
        }

        if (mostrarFeedback) {
          DrawText(respostaCorreta ? "Você acertou!" : "Você errou!",
            SCREEN_WIDTH/2 -100, 440, 24,
            respostaCorreta ? GREEN : RED);

          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            cliqueLiberado = true;
            if (respostaCorreta) {
              state = STATE_ROULETTE;
            }
          }
        } else {
          cliqueLiberado = true;
        }
        break;
      }

      case STATE_GAMEOVER: {
        DrawText("Fim de jogo!",
          SCREEN_WIDTH/2 - MeasureText("Fim de jogo!",30)/2,
          150, 30, RED);
        DrawText(TextFormat("Você fez %d ponto(s)", pontos),
          SCREEN_WIDTH/2 -100, 200, 20, DARKGRAY);

        DrawRectangleRec(botaoCentral, LIGHTGRAY);
        DrawText("JOGAR NOVAMENTE",
          botaoCentral.x + (200 - MeasureText("JOGAR NOVAMENTE",20))/2,
          botaoCentral.y + 15, 20, DARKGRAY);

        DrawRectangleRec(botaoVoltar, LIGHTGRAY);
        DrawText("VOLTAR AO MENU",
          botaoVoltar.x + (200 - MeasureText("VOLTAR AO MENU",20))/2,
          botaoVoltar.y + 15, 20, DARKGRAY);

        if (CheckCollisionPointRec(mouse, botaoCentral) &&
          IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          pontos = 0;
          if (perguntaAtual.pergunta) freePergunta(&perguntaAtual);
          if (categoriaAtual) free((void*)categoriaAtual);
          categoriaAtual = strdup(spinCategory(categorias));
          perguntaAtual = gerarPerguntaComIA(categoriaAtual);
          aguardandoResposta = true;
          mostrarFeedback = false;
          respostaCorreta = false;
          cliqueLiberado = false;
          state = STATE_PLAYING;
        }

        if (CheckCollisionPointRec(mouse, botaoVoltar) &&
          IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          pontos = 0;
          nomeConfirmado = false;
          nomeLength = 0;
          nomeJogador[0] = '\0';
          state = STATE_MENU;
        }
        break;
      }

      case STATE_RANKING: {
        DrawText("RANKING DE JOGADORES",
          SCREEN_WIDTH/2 - MeasureText("RANKING DE JOGADORES",30)/2,
          50, 30, DARKBLUE);

        Jogador jogadores[100];
        int total = 0;
        FILE *f = fopen("ranking.txt","r");
        if (f) {
          while (fscanf(f, "%[^;];%d\n", jogadores[total].nome, &jogadores[total].pontos) == 2) {
            total++;
          }
          fclose(f);
        }
        qsort(jogadores, total, sizeof(Jogador), compararRanking);

        for (int i = 0; i < total && i < 10; i++) {
          DrawText(TextFormat("%d. %s - %d pts", i+1,
            jogadores[i].nome, jogadores[i].pontos),
            SCREEN_WIDTH/2 -150, 100 + i*30, 20, DARKGRAY);
        }

        DrawRectangleRec(botaoVoltar, LIGHTGRAY);
        DrawText("VOLTAR AO MENU",
          botaoVoltar.x + (200 - MeasureText("VOLTAR AO MENU",20))/2,
          botaoVoltar.y + 15, 20, DARKGRAY);

        if (CheckCollisionPointRec(mouse, botaoVoltar) &&
          IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          state = STATE_MENU;
        }
        break;
      }

      default: break;
    }
    EndDrawing();
  }

  UnloadBackground();
  if (perguntaAtual.pergunta) freePergunta(&perguntaAtual);
  if (categoriaAtual) free((void*)categoriaAtual);
  CloseWindow();
  return 0;
}



void InitBackground(void) {
  bgTexture = LoadTexture("assets/bg.png");
  bgPosition = (Vector2){ 0.0f, 0.0f };
  bgScale = (float)GetScreenWidth() / bgTexture.width;
}

void DrawBackground(void) {
  DrawTextureEx(bgTexture, bgPosition, 0.0f, bgScale, WHITE);
}

void UnloadBackground(void) {
  UnloadTexture(bgTexture);
}

void freePergunta(Pergunta *p) {
  if (!p) return;

  if (p->pergunta) {
    free(p->pergunta);
    p->pergunta = NULL;
  }

  for (int i = 0; i < 4; i++) {
    if (p->alternativas[i]) {
      free(p->alternativas[i]);
      p->alternativas[i] = NULL;
    }
  }
}

int compararRanking(const void *a, const void *b) {
  return ((Jogador *)b)->pontos - ((Jogador *)a)->pontos;
}

bool nomeExiste(const char *nome) {
  FILE *arquivo = fopen("ranking.txt", "r");
  if (!arquivo) return false;

  char nomeArquivo[64];
  int pontosTemp;

  while (fscanf(arquivo, "%[^;];%d\n", nomeArquivo, &pontosTemp) == 2) {
    if (strcmp(nomeArquivo, nome) == 0) {
      fclose(arquivo);
      return true;
    }
  }
  fclose(arquivo);
  return false;
}

int encontrarPosicaoRanking(const char *nome) {
  Jogador jogadores[100];
  int total = 0;

  FILE *arquivo = fopen("ranking.txt", "r");
  if (!arquivo) return -1;

  while (fscanf(arquivo, "%[^;];%d\n", jogadores[total].nome, &jogadores[total].pontos) == 2) {
    total++;
  }
  fclose(arquivo);
  qsort(jogadores, total, sizeof(Jogador), compararRanking);

  for (int i = 0; i < total; i++) {
    if (strcmp(jogadores[i].nome, nome) == 0) {
      return i + 1;
    }
  }
  return -1;
}

Node* initCategories() {
  const char *categorias[] = {
    "Esportes", "Ciências", "Geografia", "Entretenimento", "História", "Artes"
  };

  Node *head = NULL, *atual = NULL;
  for (int i = 0; i < 6; i++) {
    Node *novo = malloc(sizeof(Node));
    novo->categoria = categorias[i];
    novo->prox = NULL;
    if (!head) head = novo;
    else atual->prox = novo;
    atual = novo;
  }
  atual->prox = head;
  return head;
}

const char* spinCategory(Node *head) {
  int passos = GetRandomValue(1, 10);
  Node *atual = head;
  for (int i = 0; i < passos; i++) atual = atual->prox;
  return atual->categoria;
}

Pergunta gerarPerguntaComIA(const char *categoria) {
  Pergunta p;
  bool dup;

  do {
    remove("pergunta.json");
    char comando[128];
    snprintf(comando, sizeof(comando),
      "./gerar_pergunta \"%s\"", categoria);
    (void)system(comando);

    FILE *fp = fopen("pergunta.json", "r");
    if (!fp) {
      p.pergunta = strdup("Erro ao carregar pergunta");
      for (int i = 0; i < 4; i++) p.alternativas[i] = strdup("N/A");
      p.respostaCorreta = 0;
      return p;
    }
    char buffer[2048];
    size_t n = fread(buffer,1,sizeof(buffer)-1,fp);
    buffer[n] = '\0';
    fclose(fp);

    struct json_object *root = json_tokener_parse(buffer);
    struct json_object *jperg, *jalts, *jcor;
    json_object_object_get_ex(root, "pergunta",     &jperg);
    json_object_object_get_ex(root, "alternativas", &jalts);
    json_object_object_get_ex(root, "correta",      &jcor);

    const char *strp = json_object_get_string(jperg);
    p.pergunta = strdup(strp);
    for (int i = 0; i < 4; i++) {
      const char *alt = json_object_get_string(
        json_object_array_get_idx(jalts, i));
      p.alternativas[i] = strdup(alt);
    }
    p.respostaCorreta = json_object_get_int(jcor);
    json_object_put(root);

    dup = false;
    for (int i = 0; i < askedCount; i++) {
      if (strcmp(p.pergunta, askedQuestions[i]) == 0) {
        dup = true;
        freePergunta(&p);
        break;
      }
    }
  } while (dup && askedCount < MAX_QUESTIONS);

  askedQuestions[askedCount++] = strdup(p.pergunta);
  return p;
}
