# Gira-Gira Roletinha

**Gira-Gira Roletinha** é um quiz game divertido e educativo, desenvolvido para colocar seus conhecimentos à prova em diferentes categorias. Antes de cada pergunta, uma roleta gira para definir aleatoriamente o tema, trazendo uma experiência dinâmica e desafiadora a cada jogada.

## 🎮 Como funciona

1. Ao iniciar o jogo, o jogador vê a roleta virtual com as categorias:
   - Esportes
   - Ciência
   - Geografia
   - Entretenimento
   - História
   - Artes
2. O jogador pressiona uma tecla para girar a roleta.
3. Quando a roleta parar, uma categoria é selecionada aleatoriamente.
4. O jogo apresenta uma pergunta relacionada à categoria sorteada.
5. O jogador seleciona a resposta correta entre as opções.
   - Se acertar, ganha pontos e continua para a próxima rodada.
   - Se errar, o jogo termina e o placar final é registrado no ranking.
6. O ranking armazena o nome do jogador e sua pontuação máxima para futuras consultas.

---

## 🛠️ Tecnologias

- Linguagem: **C**
- Biblioteca Gráfica e de Entrada/Saída: **[raylib](https://www.raylib.com/)**
- Compilação: **GCC**

---

## 🚀 Como executar

1. **Pré-requisitos**:
   - Se for Windows ter MSYS2 instalado (https://www.msys2.org/)
   - No MSYS2 MinGW 64-bit shell, instale as dependências:
   ```bash
   pacman -Syu            # atualiza pacotes
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-raylib
   ```

2. **Compilação e execução**:
   ```bash
   gcc -o gira.exe main.c -Iinclude -Llib \
       -lraylib -lopengl32 -lgdi32 -lwinmm -lm
   ./gira.exe
