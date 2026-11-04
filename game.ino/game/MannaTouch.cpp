#include "MannaTouch.h"

#define LIMIAR_TOUCH 26000

int touchButton(int pino) {

  int leitura = touchRead(pino);

  if (leitura > LIMIAR_TOUCH) {
    return 1;
  }

  return 0;
}
