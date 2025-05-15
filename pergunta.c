// gerar_pergunta.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

#define API_URL "https://api.openai.com/v1/chat/completions"

struct string {
  char *ptr;
  size_t len;
};

static void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(1);
  s->ptr[0] = '\0';
}

static size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len + 1);
  memcpy(s->ptr + s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size * nmemb;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Uso: %s <categoria>\n", argv[0]);
    return 1;
  }
  const char *categoria = argv[1];
  const char *api_key = getenv("OPENAI_API_KEY");
  if (!api_key) {
    fprintf(stderr, "Erro: defina a variável de ambiente OPENAI_API_KEY\n");
    return 1;
  }

  json_object *root = json_object_new_object();
  json_object_object_add(root, "model", json_object_new_string("gpt-3.5-turbo"));
  json_object *messages = json_object_new_array();
  json_object *msg = json_object_new_object();
  char prompt[512];
  snprintf(prompt, sizeof(prompt),
  "Gere uma pergunta de múltipla escolha sobre '%s' com 4 alternativas. "
  "Retorne apenas o JSON: {\"pergunta\":\"...\",\"alternativas\":[\"A\",\"B\",\"C\",\"D\"],\"correta\":0}.",
  categoria);
  json_object_object_add(msg, "role",    json_object_new_string("user"));
  json_object_object_add(msg, "content", json_object_new_string(prompt));
  json_object_array_add(messages, msg);
  json_object_object_add(root, "messages",    messages);
  json_object_object_add(root, "temperature", json_object_new_double(0.8));

  const char *post_data = json_object_to_json_string(root);

  CURL *curl = curl_easy_init();
  struct string response;
  init_string(&response);

  struct curl_slist *headers = NULL;
  char auth_header[256];
  snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, auth_header);

  curl_easy_setopt(curl, CURLOPT_URL,        API_URL);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,    &response);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl falhou: %s\n", curl_easy_strerror(res));
    return 1;
  }
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  json_object_put(root);

  json_object *resp_json   = json_tokener_parse(response.ptr);
  json_object *choices     = NULL;
  json_object_object_get_ex(resp_json, "choices", &choices);
  json_object *choice      = json_object_array_get_idx(choices, 0);
  json_object *message_obj = NULL;
  json_object_object_get_ex(choice, "message", &message_obj);
  json_object *content_obj = NULL;
  json_object_object_get_ex(message_obj, "content", &content_obj);
  const char *content_str = json_object_get_string(content_obj);

  FILE *f = fopen("pergunta.json", "w");
  if (!f) { perror("fopen"); return 1; }
  fprintf(f, "%s\n", content_str);
  fclose(f);

  free(response.ptr);
  return 0;
}
