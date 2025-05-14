#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <stdbool.h>
#include <unistd.h> // Para usar system() no Windows
#include <json-c/json.h> // Biblioteca externa para ler JSON

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 500

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
    remove("pergunta.json"); // Apaga o arquivo anterior

    Pergunta p;
    char comando[128];
    snprintf(comando, sizeof(comando),
    "\"C:\\Users\\Caio Fonseca\\AppData\\Local\\Programs\\Python\\Python313\\python.exe\" \"C:\\Users\\Caio Fonseca\\Gira-Gira_Roletinha\\gerar_pergunta.py\" \"%s\"", categoria);
    system(comando);

    FILE *fp = fopen("C:\\Users\\Caio Fonseca\\Gira-Gira_Roletinha\\pergunta.json", "r");
    if (!fp) {
        p.pergunta = "Erro ao carregar pergunta";
        for (int i = 0; i < 4; i++) p.alternativas[i] = "N/A";
        p.respostaCorreta = 0;
        return p;
    }

    char buffer[2048];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), fp);
    if (bytesRead < sizeof(buffer)) buffer[bytesRead] = '\0';
    else buffer[sizeof(buffer) - 1] = '\0';

    // Verifica se o arquivo estava vazio
    if (bytesRead == 0) {
        p.pergunta = "Erro ao carregar pergunta";
        for (int i = 0; i < 4; i++) p.alternativas[i] = "N/A";
        p.respostaCorreta = 0;
        printf("DEBUG: Pergunta lida: %s\n", p.pergunta);
        return p;
    }
    fclose(fp);

    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(buffer);

    struct json_object *pergunta, *alternativas, *correta;
    json_object_object_get_ex(parsed_json, "pergunta", &pergunta);
    json_object_object_get_ex(parsed_json, "alternativas", &alternativas);
    json_object_object_get_ex(parsed_json, "correta", &correta);

    const char *perguntaStr = json_object_get_string(pergunta);
    p.pergunta = malloc(strlen(perguntaStr) + 1);
    strcpy((char *)p.pergunta, perguntaStr);

        for (int i = 0; i < 4; i++) {
            const char *altStr = json_object_get_string(json_object_array_get_idx(alternativas, i));
            p.alternativas[i] = malloc(strlen(altStr) + 1);
            strcpy((char *)p.alternativas[i], altStr);
    }


    p.respostaCorreta = json_object_get_int(correta);
    return p;
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gira-Gira Roletinha");
    SetTargetFPS(60);
    srand(time(NULL));

    bool nomeDuplicado = false;

    Rectangle botoesAlternativas[4];
    for (int i = 0; i < 4; i++)
        botoesAlternativas[i] = (Rectangle){150, 180 + i * 60, 500, 40};

    Rectangle botaoCentral = { SCREEN_WIDTH/2 - 100, 300, 200, 50 };

    bool telaMenu = true, telaNome = false, telaRoleta = false, jogoIniciado = false;
    bool fimDeJogo = false, telaRanking = false;
    bool aguardandoResposta = false, mostrarFeedback = false;
    bool respostaCorreta = false, nomeConfirmado = false, cliqueLiberado = true;

    int pontos = 0;
    char nomeJogador[21] = "\0";
    int nomeLength = 0;
    int letra = 0;

    Node *categorias = initCategories();
    const char *categoriaAtual = NULL;
    Pergunta perguntaAtual;
    perguntaAtual.pergunta = NULL;
    for (int i = 0; i < 4; i++){
        perguntaAtual.alternativas[i] = NULL;
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (telaMenu) {
            DrawText("Gira-Gira Roletinha", SCREEN_WIDTH/2 - MeasureText("Gira-Gira Roletinha", 30)/2, 100, 30, DARKBLUE);

            Rectangle botaoJogar = { SCREEN_WIDTH/2 - 100, 200, 200, 50 };
            Rectangle botaoRanking = { SCREEN_WIDTH/2 - 100, 270, 200, 50 };
            Rectangle botaoSair = { SCREEN_WIDTH/2 - 100, 340, 200, 50 };

            DrawRectangleRec(botaoJogar, LIGHTGRAY);
            DrawText("JOGAR", botaoJogar.x + (200 - MeasureText("JOGAR", 20))/2, botaoJogar.y + 15, 20, DARKGRAY);

            DrawRectangleRec(botaoRanking, LIGHTGRAY);
            DrawText("RANKING", botaoRanking.x + (200 - MeasureText("RANKING", 20))/2, botaoRanking.y + 15, 20, DARKGRAY);

            DrawRectangleRec(botaoSair, LIGHTGRAY);
            DrawText("SAIR", botaoSair.x + (200 - MeasureText("SAIR", 20))/2, botaoSair.y + 15, 20, DARKGRAY);

            if (CheckCollisionPointRec(GetMousePosition(), botaoJogar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                telaMenu = false;
                telaNome = true;
            }

            if (CheckCollisionPointRec(GetMousePosition(), botaoRanking) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                telaMenu = false;
                telaNome = false;
                nomeConfirmado = false;
                telaRoleta = false;
                jogoIniciado = false;
                fimDeJogo = false;
                mostrarFeedback = false;
                cliqueLiberado = true;
                aguardandoResposta = false;

                // Abrir ranking
                telaRanking = true;
            }

            if (CheckCollisionPointRec(GetMousePosition(), botaoSair) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                CloseWindow();
            }
        }

        else if (telaNome && !nomeConfirmado) {
            DrawText("Digite seu nome:", SCREEN_WIDTH/2 - 100, 150, 20, DARKGRAY);
            DrawRectangle(SCREEN_WIDTH/2 - 150, 180, 300, 40, LIGHTGRAY);
            DrawText(nomeJogador, SCREEN_WIDTH/2 - 140, 190, 20, DARKBLUE);

            letra = GetCharPressed();
            while (letra > 0) {
                if ((letra >= 32) && (letra <= 125) && (nomeLength < 20)) {
                    nomeJogador[nomeLength++] = (char)letra;
                    nomeJogador[nomeLength] = '\0';
                }
                letra = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && nomeLength > 0) {
                nomeJogador[--nomeLength] = '\0';
            }

            DrawRectangleRec(botaoCentral, LIGHTGRAY);
            DrawText("CONFIRMAR", botaoCentral.x + (200 - MeasureText("CONFIRMAR", 20))/2, botaoCentral.y + 15, 20, DARKGRAY);

            if (CheckCollisionPointRec(GetMousePosition(), botaoCentral) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (nomeLength > 0) {
                    if (nomeExiste(nomeJogador)) {
                        nomeDuplicado = true;
                    } else {
                        nomeConfirmado = true;
                        nomeDuplicado = false;
                        telaNome = false;
                        telaRoleta = true;
                    }
                }
            }

            if (nomeDuplicado) {
                DrawText("Nome já existe no ranking! Escolha outro.", SCREEN_WIDTH/2 - 170, 240, 20, RED);
            }
        }

        else if (telaRoleta) {
            DrawText(TextFormat("Jogador: %s", nomeJogador), 20, 20, 20, DARKGRAY);
            DrawText(TextFormat("Pontos: %d", pontos), SCREEN_WIDTH - 150, 20, 20, DARKGRAY);

            DrawText("Clique para girar a roleta!", SCREEN_WIDTH/2 - MeasureText("Clique para girar a roleta!", 24)/2, 200, 24, DARKBLUE);
            DrawRectangleRec(botaoCentral, LIGHTGRAY);
            DrawText("GIRAR ROLETA", botaoCentral.x + (200 - MeasureText("GIRAR ROLETA", 20))/2, botaoCentral.y + 15, 20, DARKGRAY);

            if (cliqueLiberado && CheckCollisionPointRec(GetMousePosition(), botaoCentral) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (perguntaAtual.pergunta != NULL) {
                free((void *)perguntaAtual.pergunta);
                for (int i = 0; i < 4; i++) {
                    if (perguntaAtual.alternativas[i] != NULL)
                        free((void *)perguntaAtual.alternativas[i]);
                }
            }
            if (categoriaAtual) free((void *)categoriaAtual);
            categoriaAtual = strdup(spinCategory(categorias));
            perguntaAtual = gerarPerguntaComIA(categoriaAtual);
                aguardandoResposta = true;
                mostrarFeedback = false;
                respostaCorreta = false;
                telaRoleta = false;
                jogoIniciado = true;
                cliqueLiberado = false;
            }
        }

          else if (jogoIniciado && !fimDeJogo) {
            DrawText(TextFormat("Jogador: %s", nomeJogador), 20, 20, 20, DARKGRAY);
            DrawText(TextFormat("Pontos: %d", pontos), SCREEN_WIDTH - 150, 20, 20, DARKGRAY);
            DrawText(TextFormat("Categoria: %s", categoriaAtual), SCREEN_WIDTH/2 - 100, 60, 20, DARKBLUE);
            DrawText(TextFormat("DEBUG: %s", perguntaAtual.pergunta), 100, 80, 10, RED);
            DrawText(perguntaAtual.pergunta, 100, 110, 20, BLACK);

            for (int i = 0; i < 4; i++) {
                DrawRectangleRec(botoesAlternativas[i], LIGHTGRAY);
                DrawText(perguntaAtual.alternativas[i],
                    botoesAlternativas[i].x + (500 - MeasureText(perguntaAtual.alternativas[i], 20))/2,
                    botoesAlternativas[i].y + 10, 20, DARKGRAY);
            }

            if (aguardandoResposta && cliqueLiberado) {
                for (int i = 0; i < 4; i++) {
                    if (CheckCollisionPointRec(GetMousePosition(), botoesAlternativas[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        respostaCorreta = (i == perguntaAtual.respostaCorreta);
                        mostrarFeedback = true;
                        aguardandoResposta = false;
                        if (respostaCorreta) pontos++;
                        else {
                            fimDeJogo = true;

                            // Salvar pontuação no ranking
                            FILE *arquivo = fopen("ranking.txt", "a");
                            if (arquivo != NULL) {
                                fprintf(arquivo, "%s;%d\n", nomeJogador, pontos);
                                fclose(arquivo);
                            }
                        }
                        cliqueLiberado = false;
                    }
                }
            }

            if (mostrarFeedback) {
            DrawText(respostaCorreta ? "Você acertou!" : "Você errou!", SCREEN_WIDTH/2 - 100, 440, 24, respostaCorreta ? GREEN : RED);

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
               if (respostaCorreta) {
                    if (perguntaAtual.pergunta != NULL) {
                        free((void *)perguntaAtual.pergunta);
                        for (int i = 0; i < 4; i++) {
                            if (perguntaAtual.alternativas[i] != NULL)
                                free((void *)perguntaAtual.alternativas[i]);
                        }
                    }
                    if (categoriaAtual) free((void *)categoriaAtual);
                    categoriaAtual = strdup(spinCategory(categorias));
                    perguntaAtual = gerarPerguntaComIA(categoriaAtual);


                    jogoIniciado = false;
                    telaRoleta = true;
                    aguardandoResposta = false;
                    mostrarFeedback = false;
                    respostaCorreta = false;
                    cliqueLiberado = true;
                } else {
                cliqueLiberado = true;
                }
            }
        }

            if (!mostrarFeedback) cliqueLiberado = true;
        }

        else if (fimDeJogo) {
            DrawText("Fim de jogo!", SCREEN_WIDTH/2 - MeasureText("Fim de jogo!", 30)/2, 150, 30, RED);
            DrawText(TextFormat("Você fez %d ponto(s)", pontos), SCREEN_WIDTH/2 - 100, 200, 20, DARKGRAY);

            DrawRectangleRec(botaoCentral, LIGHTGRAY);
            DrawText("JOGAR NOVAMENTE", botaoCentral.x + (200 - MeasureText("JOGAR NOVAMENTE", 20))/2, botaoCentral.y + 15, 20, DARKGRAY);

            Rectangle botaoVoltar = { SCREEN_WIDTH/2 - 100, 360, 200, 50 };
            DrawRectangleRec(botaoVoltar, LIGHTGRAY);
            DrawText("VOLTAR AO MENU", botaoVoltar.x + (200 - MeasureText("VOLTAR AO MENU", 20))/2, botaoVoltar.y + 15, 20, DARKGRAY);

            if (CheckCollisionPointRec(GetMousePosition(), botaoCentral) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                pontos = 0;
                fimDeJogo = false;

                if (perguntaAtual.pergunta != NULL) {
                    free((void *)perguntaAtual.pergunta);
                    for (int i = 0; i < 4; i++) {
                        if (perguntaAtual.alternativas[i] != NULL)
                            free((void *)perguntaAtual.alternativas[i]);
                    }
                }
                if (categoriaAtual) free((void *)categoriaAtual);

                categoriaAtual = strdup(spinCategory(categorias));
                perguntaAtual = gerarPerguntaComIA(categoriaAtual);

                jogoIniciado = true;
                aguardandoResposta = true;
                mostrarFeedback = false;
                respostaCorreta = false;
                cliqueLiberado = true;
            }

            if (CheckCollisionPointRec(GetMousePosition(), botaoVoltar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                pontos = 0;
                fimDeJogo = false;
                telaMenu = true;
                telaNome = false;
                nomeConfirmado = false;
                nomeLength = 0;
                nomeJogador[0] = '\0';
            }
        }

        else if (telaRanking) {
            DrawText("RANKING DE JOGADORES", SCREEN_WIDTH/2 - MeasureText("RANKING DE JOGADORES", 30)/2, 50, 30, DARKBLUE);

            Jogador jogadores[100];
            int totalJogadores = 0;

            FILE *arquivo = fopen("ranking.txt", "r");
            if (arquivo != NULL) {
                while (fscanf(arquivo, "%[^;];%d\n", jogadores[totalJogadores].nome, &jogadores[totalJogadores].pontos) == 2) {
                    totalJogadores++;
                }
                fclose(arquivo);
            }

            qsort(jogadores, totalJogadores, sizeof(Jogador), compararRanking);

            for (int i = 0; i < totalJogadores && i < 10; i++) {
                DrawText(TextFormat("%d. %s - %d pts", i+1, jogadores[i].nome, jogadores[i].pontos),
                         SCREEN_WIDTH/2 - 150, 100 + i * 30, 20, DARKGRAY);
            }

            Rectangle botaoVoltar = { SCREEN_WIDTH/2 - 100, 440, 200, 50 };
            DrawRectangleRec(botaoVoltar, LIGHTGRAY);
            DrawText("VOLTAR AO MENU", botaoVoltar.x + (200 - MeasureText("VOLTAR AO MENU", 20))/2, botaoVoltar.y + 15, 20, DARKGRAY);

            if (CheckCollisionPointRec(GetMousePosition(), botaoVoltar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                telaRanking = false;
                telaMenu = true;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    if (perguntaAtual.pergunta != NULL) free((void *)perguntaAtual.pergunta);
        for (int i = 0; i < 4; i++) {
        if (perguntaAtual.alternativas[i] != NULL)
        free((void *)perguntaAtual.alternativas[i]);
    }
    return 0;
} 