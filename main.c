#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <stdbool.h>
#include <unistd.h>
#include <json-c/json.h>
#include <categories.h>
#include <math.h>
#include <pergunta.h>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600

typedef enum {
  STATE_MENU,
  STATE_NAME_INPUT,
  STATE_ROULETTE,
  STATE_PLAYING,
  STATE_GAMEOVER,
  STATE_RANKING,
  STATE_EXIT
} GameState;

typedef struct {
  char nome[64];
  int pontos;
} Jogador;

void InitBackground(void);
void DrawBackground(void);
int compararRanking(const void *a, const void *b);
bool nomeExiste(const char *nome);
int encontrarPosicaoRanking(const char *nome);
static void swapJogador(Jogador *a, Jogador *b);
static int partition(Jogador arr[], int low, int high);
void quickSortJogadores(Jogador arr[], int low, int high);

static Texture2D bgTexture;
static Vector2 bgPosition;
static float bgScale;
static Texture2D wheelTex;
static Texture2D pointerTex;

Rectangle botaoJogar, botaoRankingB, botaoSair;
Rectangle botaoCentral, botaoVoltar;
Rectangle botoesAlt[4];

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gira-Gira Roletinha");
  SetTargetFPS(60);
  InitBackground();
  srand((unsigned)time(NULL));
  Node *categorias = initCategories();
  wheelTex   = LoadTexture("assets/roleta.png");

  GameState state = STATE_MENU;

  bool nomeDuplicado = false;
  bool nomeConfirmado = false;
  bool aguardandoResposta = false;
  bool mostrarFeedback = false;
  bool respostaCorreta = false;
  bool cliqueLiberado = true;

  float wheelRotation = 0;
  float wheelSpeed = 0;
  bool wheelSpinning = false;
  int pontos = 0;
  char nomeJogador[21] = "\0";
  int nomeLength = 0;
  int letra = 0;

  const float ALT_SCALE = 0.6f;
  const int BTN_HEIGHT = 50;
  const int SPACING_Y = 20;
  const int TOP_Y = 120;

  float altWidth = SCREEN_WIDTH * ALT_SCALE;
  for (int i = 0; i < 4; i++) {
    botoesAlt[i].width = altWidth;
    botoesAlt[i].height = BTN_HEIGHT;
    botoesAlt[i].x = (SCREEN_WIDTH - altWidth) / 2.0f;
    botoesAlt[i].y = TOP_Y + i * (BTN_HEIGHT + SPACING_Y);
  }

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

    if (wheelSpinning) {
      float dt = GetFrameTime();
      wheelRotation += wheelSpeed * dt;
      wheelSpeed = fmaxf(0, wheelSpeed - 200*dt);
      if (wheelSpeed <= 0) {
        wheelSpinning = false;
        categoriaAtual = spinCategory(categorias);
      }
    }

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
          SCREEN_WIDTH - 150, 20, 20, DARKGRAY);

        if (!wheelSpinning && CheckCollisionPointRec(mouse, botaoCentral) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
          wheelSpinning = true;
          wheelSpeed = 400 + rand() % 200;
        }

        if (wheelSpinning) {
          float dt = GetFrameTime();
          wheelRotation += wheelSpeed * dt;
          wheelSpeed = fmaxf(0, wheelSpeed - 200 * dt);

          if (wheelSpeed == 0) {
            wheelSpinning = false;

            int nSeg  = countCategories(categorias);
            float seg = 360.0f / nSeg;
            float ang = fmodf(wheelRotation, 360.0f);
            if (ang < 0) ang += 360.0f;
            int idx = (int)((ang + seg * 0.5f) / seg) % nSeg;

            if (categoriaAtual) free((void*)categoriaAtual);
            categoriaAtual = strdup(getCategoryByIndex(categorias, idx));

            perguntaAtual = gerarPerguntaComIA(categoriaAtual);
            aguardandoResposta = true;
            cliqueLiberado = true;

            state = STATE_PLAYING;
          }
        }

        {
          float scale = 0.6f;
          Vector2 center = { SCREEN_WIDTH*0.82f, SCREEN_HEIGHT*1.0f };
          Rectangle source = { 0, 0,
            (float)wheelTex.width,
            (float)wheelTex.height };

          Rectangle dest = {
            center.x - (wheelTex.width  * scale) * 0.5f,
            center.y - (wheelTex.height * scale) * 0.5f,
            wheelTex.width  * scale,
            wheelTex.height * scale
          };

          Vector2 origin = { dest.width * 0.5f,
            dest.height * 0.5f };

          DrawTexturePro(
            wheelTex,
            source,
            dest,
            origin,
            wheelRotation,
            WHITE
          );

          Vector2 ptrPos = {
            center.x - pointerTex.width * 0.5f,
            center.y - dest.height    * 0.5f - pointerTex.height
          };
          DrawTexture(pointerTex, ptrPos.x, ptrPos.y, WHITE);

        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
          cliqueLiberado = true;
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
        if (total > 0) quickSortJogadores(jogadores, 0, total - 1);

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

  destroyCategories(categorias);
  UnloadTexture(bgTexture);
  UnloadTexture(wheelTex);
  UnloadTexture(pointerTex);
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

static void swapJogador(Jogador *a, Jogador *b) {
  Jogador tmp = *a;
  *a = *b;
  *b = tmp;
}

static int partition(Jogador arr[], int low, int high) {
  int pivot = arr[high].pontos;
  int i = low - 1;
  for (int j = low; j < high; j++) {
    if (arr[j].pontos > pivot) {
      i++;
      swapJogador(&arr[i], &arr[j]);
    }
  }
  swapJogador(&arr[i+1], &arr[high]);
  return i + 1;
}

void quickSortJogadores(Jogador arr[], int low, int high) {
  if (low < high) {
    int pi = partition(arr, low, high);
    quickSortJogadores(arr, low, pi - 1);
    quickSortJogadores(arr, pi + 1, high);
  }
}
