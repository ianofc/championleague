#include "mannaNAUTA_WS2812.h"

mannaNAUTA_WS2812 strip(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

// Mapeamento comprovado da sua bancada
#define BTN_AZUL      1  // LED 2 -> 1 clique = Pausa | 2 cliques = Reinicia
#define BTN_AMARELO   2  // LED 1 -> Move Esquerda
#define BTN_BRANCO    3  // LED 0 -> Move Direita

uint8_t grid[5][5];

const int NUM_TIPOS = 4;
int pieceType = 0;
int pieceRow = 0;
int pieceCol = 2;

// Controle de tempo
unsigned long lastDropTime = 0;
const unsigned long dropInterval = 520; // Ritmo equilibrado de queda

int score = 0;
bool isGameOver = false;
bool isPaused = false;

// Variáveis para detecção de clique simples vs duplo clique no Azul
bool lastBlueState = LOW;
int blueClickCount = 0;
unsigned long lastBlueClickTime = 0;
const unsigned long doubleClickWindow = 450; // Janela de 450 ms para detectar o 2º clique

// Mapeamento linear direto
int getIndex(int row, int col) {
  if (row < 0 || row > 4 || col < 0 || col > 4) return -1;
  return (row * 5) + col;
}

// Formatos das peças (1x1, 1x2, 2x1, L)
int getPieceBlocks(int type, int outR[3], int outC[3]) {
  outR[0] = 0; outC[0] = 0;

  if (type == 0) {
    return 1; // 1x1
  } else if (type == 1) {
    outR[1] = 0; outC[1] = 1; // 1x2 horizontal
    return 2;
  } else if (type == 2) {
    outR[1] = 1; outC[1] = 0; // 2x1 vertical
    return 2;
  } else if (type == 3) {
    outR[1] = 1; outC[1] = 0; // Formato L
    outR[2] = 1; outC[2] = 1;
    return 3;
  }
  return 1;
}

bool canPlace(int r, int c, int type) {
  int blkR[3], blkC[3];
  int n = getPieceBlocks(type, blkR, blkC);

  for (int i = 0; i < n; i++) {
    int targetR = r + blkR[i];
    int targetC = c + blkC[i];

    if (targetC < 0 || targetC > 4 || targetR > 4) return false;
    if (targetR >= 0 && grid[targetR][targetC] == 1) return false;
  }
  return true;
}

void spawnPiece() {
  pieceType = random(0, NUM_TIPOS);
  pieceRow = 0;
  pieceCol = (pieceType == 1 || pieceType == 3) ? 1 : 2;

  if (!canPlace(pieceRow, pieceCol, pieceType)) {
    isGameOver = true;
  }
}

void resetGame() {
  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 5; c++) {
      grid[r][c] = 0;
    }
  }
  score = 0;
  isGameOver = false;
  isPaused = false;
  blueClickCount = 0;
  lastDropTime = millis();
  spawnPiece();
}

void checkLines() {
  for (int r = 4; r >= 0; r--) {
    bool full = true;
    for (int c = 0; c < 5; c++) {
      if (grid[r][c] == 0) {
        full = false;
        break;
      }
    }
    if (full) {
      score++;

      // Linha completa pisca em verde
      for (int c = 0; c < 5; c++) {
        strip.setLedColorData(getIndex(r, c), 0, 255, 0);
      }
      strip.show();
      delay(120);

      // Desce linhas superiores
      for (int dr = r; dr > 0; dr--) {
        for (int c = 0; c < 5; c++) {
          grid[dr][c] = grid[dr - 1][c];
        }
      }
      for (int c = 0; c < 5; c++) grid[0][c] = 0;
      r++;
    }
  }
}

void triggerGameOver() {
  // Alerta em vermelho
  for (int i = 0; i < 2; i++) {
    strip.setAllLedsColor(200, 0, 0);
    delay(140);
    strip.setAllLedsColorData(0, 0, 0);
    strip.show();
    delay(100);
  }

  // Exibe o número de pontos (0 a 9)
  strip.setAllLedsColorData(0, 0, 0);
  uint8_t displayScore = (score > 9) ? 9 : (uint8_t)score;
  strip.writeNumber(displayScore, 0, 180, 255);

  unsigned long startWait = millis();
  while (millis() - startWait < 3000) {
    if (digitalRead(BTN_AZUL) == HIGH || digitalRead(BTN_AMARELO) == HIGH || digitalRead(BTN_BRANCO) == HIGH) {
      break;
    }
    delay(15);
  }

  resetGame();
}

void setup() {
  strip.begin();
  strip.setBrightness(25);
  strip.clear();
  strip.show();

  pinMode(BTN_AZUL, INPUT);
  pinMode(BTN_AMARELO, INPUT);
  pinMode(BTN_BRANCO, INPUT);

  randomSeed(analogRead(4));
  resetGame();
}

void loop() {
  if (isGameOver) {
    triggerGameOver();
    return;
  }

  unsigned long now = millis();

  // 1. Tratamento do Botão Azul (1 clique = Pausa, 2 cliques = Reiniciar)
  bool currentBlueState = (digitalRead(BTN_AZUL) == HIGH);
  if (currentBlueState && !lastBlueState) {
    blueClickCount++;
    lastBlueClickTime = now;
    delay(50); // Debounce de pressão
  }
  lastBlueState = currentBlueState;

  if (blueClickCount > 0) {
    if (blueClickCount >= 2) {
      // Duplo clique detectado -> Reinicia a partida
      strip.setAllLedsColor(220, 100, 0);
      delay(200);
      resetGame();
      return;
    } else if (now - lastBlueClickTime > doubleClickWindow) {
      // Janela expirou e foi só 1 clique -> Alterna Pausa
      isPaused = !isPaused;
      blueClickCount = 0;
      if (!isPaused) {
        lastDropTime = now; // Evita queda imediata ao despausar
      }
    }
  }

  // Se o jogo estiver pausado, interrompe a lógica e apenas mantém o desenho estático
  if (isPaused) {
    strip.setAllLedsColorData(0, 0, 0);
    
    // Desenha blocos fixados (âmbar suave)
    for (int r = 0; r < 5; r++) {
      for (int c = 0; c < 5; c++) {
        if (grid[r][c] == 1) {
          strip.setLedColorData(getIndex(r, c), 170, 60, 0);
        }
      }
    }

    // Desenha peça atual congelada (azul com brilho reduzido indicando pausa)
    int blkR[3], blkC[3];
    int n = getPieceBlocks(pieceType, blkR, blkC);
    for (int i = 0; i < n; i++) {
      int r = pieceRow + blkR[i];
      int c = pieceCol + blkC[i];
      if (r >= 0 && r < 5 && c >= 0 && c < 5) {
        strip.setLedColorData(getIndex(r, c), 0, 70, 120);
      }
    }

    strip.show();
    delay(20);
    return;
  }

  // 2. Botão Amarelo (Pino 2) -> Move Esquerda
  if (digitalRead(BTN_AMARELO) == HIGH) {
    if (canPlace(pieceRow, pieceCol - 1, pieceType)) {
      pieceCol--;
    }
    delay(130);
  }

  // 3. Botão Branco (Pino 3) -> Move Direita
  if (digitalRead(BTN_BRANCO) == HIGH) {
    if (canPlace(pieceRow, pieceCol + 1, pieceType)) {
      pieceCol++;
    }
    delay(130);
  }

  // 4. Gravidade temporal desacoplada
  if (now - lastDropTime >= dropInterval) {
    lastDropTime = now;

    if (canPlace(pieceRow + 1, pieceCol, pieceType)) {
      pieceRow++;
    } else {
      int blkR[3], blkC[3];
      int n = getPieceBlocks(pieceType, blkR, blkC);

      for (int i = 0; i < n; i++) {
        int r = pieceRow + blkR[i];
        int c = pieceCol + blkC[i];
        if (r >= 0 && r < 5 && c >= 0 && c < 5) {
          grid[r][c] = 1;
        }
      }

      checkLines();
      spawnPiece();
    }
  }

  // 5. Renderização contínua normal
  strip.setAllLedsColorData(0, 0, 0);

  // Blocos já fixados (âmbar suave)
  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 5; c++) {
      if (grid[r][c] == 1) {
        strip.setLedColorData(getIndex(r, c), 170, 60, 0);
      }
    }
  }

  // Peça ativa caindo (ciano vivo)
  int blkR[3], blkC[3];
  int n = getPieceBlocks(pieceType, blkR, blkC);
  for (int i = 0; i < n; i++) {
    int r = pieceRow + blkR[i];
    int c = pieceCol + blkC[i];
    if (r >= 0 && r < 5 && c >= 0 && c < 5) {
      strip.setLedColorData(getIndex(r, c), 0, 180, 255);
    }
  }

  strip.show();
  delay(15);
}
