import sys
import json
import openai
import os
import re

# ✅ Define a chave diretamente (pode trocar por variável de ambiente se quiser)
api_key = ""
if not api_key or not api_key.startswith("sk-"):
    print("Erro: chave de API inválida ou não definida.")
    sys.exit(1)

openai.api_key = api_key

# ✅ Verifica se a categoria foi passada por argumento
if len(sys.argv) < 2:
    print("Erro: categoria não informada.")
    sys.exit(1)

categoria = sys.argv[1]

# ✅ Prompt para gerar a pergunta
prompt = f"""
Gere uma pergunta de múltipla escolha sobre o tema '{categoria}' com 4 alternativas.
A resposta deve estar formatada exatamente neste JSON:
{{
  "pergunta": "texto da pergunta",
  "alternativas": ["A", "B", "C", "D"],
  "correta": 0
}}
A chave "correta" deve conter um número de 0 a 3.
"""

try:
    response = openai.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[{"role": "user", "content": prompt}],
        temperature=0.8
    )

    content = response.choices[0].message.content
    print("Resposta da IA:")
    print(content)

    # ✅ Extrai o JSON da resposta
    match = re.search(r'\{.*\}', content, re.DOTALL)
    if not match:
        raise ValueError("JSON não encontrado na resposta")

    dados = json.loads(match.group())

    # ✅ Salva no arquivo
    with open("pergunta.json", "w", encoding="utf-8") as f:
        json.dump(dados, f, ensure_ascii=False, indent=4)

    print("Pergunta gerada com sucesso!")

except Exception as e:
    print("Erro ao obter resposta da IA:", e)
    with open("pergunta.json", "w") as f:
        f.write("")  # Garante que o arquivo exista (mesmo vazio) para ser detectado no C
    sys.exit(1)