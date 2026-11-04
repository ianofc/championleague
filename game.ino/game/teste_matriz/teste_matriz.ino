// Teste da matriz WS2812 do Manna NAUTA - sem botoes, sem jogo
#include "mannaNAUTA_WS2812.h"

// Se nada acender, troque o pino aqui (padrao da lib: 47)
#undef LEDS_PIN
#define LEDS_PIN 47

mannaNAUTA_WS2812 strip(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

void tudo(uint8_t r, uint8_t g, uint8_t b, const char *nome) {
  strip.setAllLedsColorData(r, g, b);
  esp_err_t e = strip.show();
  Serial.printf("%s -> show() = %s\n", nome, e == ESP_OK ? "OK" : "FALHOU");
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.printf("Pino dos LEDs: %d, quantidade: %d\n", LEDS_PIN, LEDS_COUNT);

  bool ok = strip.begin();
  Serial.printf("strip.begin() = %s\n", ok ? "OK" : "FALHOU");

  strip.setBrightness(80);
}

void loop() {
  tudo(255, 0, 0, "vermelho");
  tudo(0, 255, 0, "verde");
  tudo(0, 0, 255, "azul");
  tudo(255, 255, 255, "branco");

  // Um LED de cada vez, para ver a ordem fisica
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setAllLedsColorData(0);
    strip.setLedColorData(i, 255, 255, 255);
    strip.show();
    Serial.printf("LED %d\n", i);
    delay(250);
  }
}
