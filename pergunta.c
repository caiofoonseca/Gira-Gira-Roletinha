// pergunta.c
#include <curl/curl.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pergunta.h"

#define API_URL "https://api.openai.com/v1/chat/completions"

typedef struct {
  char *ptr;
  size_t len;
} String;

static void init_string(String *s) {
  s->len = 0;
  s->ptr = malloc(1);
  s->ptr[0] = '\0';
}

static size_t writefunc(void *ptr, size_t size, size_t nmemb, String *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len + 1);
  memcpy(s->ptr + s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size * nmemb;
}

static char *askedQuestions[MAX_QUESTIONS];
static int askedCount = 0;

Pergunta gerarPerguntaComIA(const char *categoria) {
  Pergunta p = {0};
  bool dup;

  do {
    json_object *root = json_object_new_object();
    json_object_object_add(root, "model", json_object_new_string("gpt-3.5-turbo"));
    json_object *messages = json_object_new_array();
    json_object *msg = json_object_new_object();
    char prompt[512];
    snprintf(prompt, sizeof(prompt),
      "Gere uma pergunta de múltipla escolha sobre '%s' com 4 alternativas. "
      "Retorne apenas o JSON: {\"pergunta\":\"...\",\"alternativas\":[\"A\",\"B\",\"C\",\"D\"],\"correta\":0}.",
      categoria);
    json_object_object_add(msg, "role", json_object_new_string("user"));
    json_object_object_add(msg, "content", json_object_new_string(prompt));
    json_object_array_add(messages, msg);
    json_object_object_add(root, "messages", messages);
    json_object_object_add(root, "temperature", json_object_new_double(0.8));

    const char *post_data = json_object_to_json_string(root);

    CURL *curl = curl_easy_init();
    String response;
    init_string(&response);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", getenv("OPENAI_API_KEY"));
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    json_object_put(root);

    if (res != CURLE_OK) {
      p.pergunta = strdup("Erro: falha de requisição");
      p.respostaCorreta = 0;
      for (int i = 0; i < 4; i++) p.alternativas[i] = strdup("N/A");
      free(response.ptr);
      return p;
    }

    json_object *resp_json = json_tokener_parse(response.ptr);
    free(response.ptr);
    json_object *choices = NULL;
    json_object_object_get_ex(resp_json, "choices", &choices);
    json_object *choice = json_object_array_get_idx(choices, 0);
    json_object *message_obj = NULL;
    json_object_object_get_ex(choice, "message", &message_obj);
    json_object *content_obj = NULL;
    json_object_object_get_ex(message_obj, "content", &content_obj);
    const char *content_str = json_object_get_string(content_obj);

    json_object_put(resp_json);

    json_object *root2 = json_tokener_parse(content_str);
    json_object *jperg = NULL, *jalts = NULL, *jcor = NULL;
    json_object_object_get_ex(root2, "pergunta", &jperg);
    json_object_object_get_ex(root2, "alternativas", &jalts);
    json_object_object_get_ex(root2, "correta", &jcor);

    p.pergunta = strdup(json_object_get_string(jperg));
    p.respostaCorreta = json_object_get_int(jcor);
    for (int i = 0; i < 4; i++) {
      p.alternativas[i] = strdup(
        json_object_get_string(
        json_object_array_get_idx(jalts, i)
        )
      );
    }

    json_object_put(root2);

    dup = false;
    for (int i = 0; i < askedCount; i++) {
      if (strcmp(askedQuestions[i], p.pergunta) == 0) {
        dup = true;
        freePergunta(&p);
        break;
      }
    }
  } while (dup && askedCount < MAX_QUESTIONS);

  askedQuestions[askedCount++] = strdup(p.pergunta);
  return p;
}

void freePergunta(Pergunta *p) {
  if (!p) return;
  if (p->pergunta) free(p->pergunta);
  for (int i = 0; i < 4; i++) {
    if (p->alternativas[i]) free(p->alternativas[i]);
  }
  p->pergunta = NULL;
  for (int i = 0; i < 4; i++) p->alternativas[i] = NULL;
  p->respostaCorreta = 0;
}