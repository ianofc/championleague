# 🚀 MannaNAUTA Tetris — Sistema Embarcado de Entretenimento 5x5

**Implementação síncrona de alto desempenho do clássico jogo Tetris para a matriz de LEDs 5x5 do astronauta MannaNAUTA (ESP32-S3) com controle tátil MannaControl.**

---

## 📖 Sumário

* [Visão Geral](https://www.google.com/search?q=%23-vis%C3%A3o-geral)
* [Os 4 Módulos do Sistema](https://www.google.com/search?q=%23-os-4-m%C3%B3dulos-do-sistema)
* [Motor de Renderização WS2812](https://www.google.com/search?q=%231-%EF%B8%8F-motor-de-renderiza%C3%A7%C3%A3o-ws2812)
* [Lógica de Jogo & Gravidade](https://www.google.com/search?q=%232-%EF%B8%8F-l%C3%B3gica-de-jogo--gravidade)
* [Mapeamento de Entrada MannaControl](https://www.google.com/search?q=%233-%EF%B8%8F-mapeamento-de-entrada-mannacontrol)
* [Fim de Jogo & Placar Numérico](https://www.google.com/search?q=%234--fim-de-jogo--placar-num%C3%A9rico)


* [Arquitetura & Especificações de Hardware](https://www.google.com/search?q=%23-arquitetura--especifica%C3%A7%C3%B5es-de-hardware)
* [Esquema de Pinagem e Ligações](https://www.google.com/search?q=%23-esquema-de-pinagem-e-liga%C3%A7%C3%B5es)
* [Estrutura de Pastas](https://www.google.com/search?q=%23-estrutura-de-pastas)
* [Como Compilar e Gravar](https://www.google.com/search?q=%23-como-compilar-e-gravar)
* [Diretrizes de Manutenção do Projeto](https://www.google.com/search?q=%23-diretrizes-de-manuten%C3%A7%C3%A3o-do-projeto)

---

## 🌟 Visão Geral

O **MannaNAUTA Tetris** é um jogo embarcado autônomo desenvolvido para o ecossistema educacional e científico **Manna Team** (Kit Ground Control).

O projeto adapta a mecânica essencial do Tetris tradicional para uma grade ultracompacta de **5x5 LEDs RGB**, priorizando execução síncrona estável, proteção contra aquecimento e desacoplamento do rádio Wi-Fi para evitar surtos de consumo e reinicializações por subfrequência de tensão (*brownout*). O firmware gerencia quedas aleatórias de poliminós, detecção de colisões, limpeza de linhas completas e controle de estados por meio da interface externa **MannaControl**.

---

## 🕹️ Os 4 Módulos do Sistema

### 1. 💡 Motor de Renderização WS2812

O núcleo gráfico responsável por traduzir o buffer matricial em sinais temporizados compatíveis com WS2812.

* **Mapeamento Linear:** Conversão matricial contínua via fórmula `(linha * 5) + coluna`, compatível com a biblioteca proprietária `mannaNAUTA_WS2812`.
* **Gerenciamento de Corrente:** Brilho calibrado a ~10% (nível 25 de 255) para manter o circuito integrado e reguladores totalmente frios.
* **Buffer Limpo:** Limpeza lógica de quadro (`setAllLedsColorData(0, 0, 0)`) antes do envio, eliminando cintilação (*flickering*).

### 2. ⚙️ Lógica de Jogo & Gravidade

O cérebro computacional responsável pelas regras e física dos blocos.

* **Temporização Desacoplada:** Controle de queda contínuo via `millis()`, garantindo velocidade estável (~520 ms por passo) independentemente do ritmo dos toques do usuário.
* **Variedade de Formatos:** Sorteio pseudoaleatório entre 4 peças adaptadas: Bloco 1x1, Duplo Horizontal 1x2, Duplo Vertical 2x1 e Peça em L (3 blocos).
* **Varredura de Linhas:** Checagem automática do solo ao topo com animação de confirmação em verde e translação vertical descendente dos blocos empilhados.

### 3. 🎮 Mapeamento de Entrada MannaControl

Interfaceamento direto com a placa de botões periférica via registradores digitais.

* **Navegação Lateral:** Botão Amarelo (desloca à esquerda) e Botão Branco (desloca à direita) com rotinas de *debounce* dedicadas.
* **Controle Duplo no Botão Azul:**
* **1 Clique:** Alterna entre pausa e retomada imediata (peça congelada em tom suave).
* **2 Cliques Rápidos:** Reinicialização voluntária (*soft reset*) do tabuleiro sem travar a memória flash.


* **Isolamento de Strapping:** Desacoplamento físico do pino GPIO 0 para assegurar boot limpo da controladora sem forçar o modo download da ROM.

### 4. 🏆 Fim de Jogo & Placar Numérico

Gerenciador de estado para transições de derrota e pontuação.

* **Alerta de Colisão:** Pulso em vermelho nos 25 LEDs ao saturar o topo do tabuleiro.
* **Placar Integrado:** Renderização automática do dígito decimal de pontuação (linhas completadas) direto na matriz usando o gerador de caracteres nativo `writeNumber()`.
* **Reinício Fluido:** Espera programada de 3 segundos com interceptação imediata ao tocar em qualquer botão.

---

## 🛠️ Arquitetura & Especificações de Hardware

A stack técnica utiliza C++ embarcado para controle determinístico e baixa pegada de memória.

* **Plataforma:** MannaNAUTA (MCU Espressif ESP32-S3 Dual-Core Xtensa LX7 @ 240 MHz)
* **Matriz de Vídeo:** 25 LEDs RGB endereçáveis WS2812B em grade 5x5
* **Driver de Transmissão:** Camada RMT / Bitbang nativo com latch de reset calibrado
* **Alimentação Operacional:** 3.3V DC via barramento regulado da placa principal
* **Bibliotecas:** `mannaNAUTA_WS2812.h` (Manna Team)

---

## 🔌 Esquema de Pinagem e Ligações

As conexões entre a placa **MannaControl** e a barra central de pinos (**Conector J2**) do **MannaNAUTA** são dispostas da seguinte forma:

| Linha / Função | Pino MannaControl | Pino no MannaNAUTA (J2) | Comportamento no Jogo |
| --- | --- | --- | --- |
| **Alimentação** | VCC | **3V3** | Alimentação lógica de 3.3V |
| **Referência** | GND | **GND** | Referência de aterramento comum |
| **Botão Azul** | BT2 (LED 2) | **Pino 01** | 1 Clique: Pausa / 2 Cliques: Reinicia |
| **Botão Amarelo** | BT1 (LED 1) | **Pino 02** | Move a peça para a Esquerda |
| **Botão Branco** | BT0 (LED 0) | **Pino 03** | Move a peça para a Direita |
| **Botão Vermelho** | BT3 (LED 3) | *Desconectado* | Isolado fisicamente (hardware inoperante) |

---

## 📁 Estrutura de Pastas

```text
mannanauta-tetris/
├── game/
│   ├── game.ino                 # Loop principal, mecânica do Tetris e rotinas de IO
│   ├── mannaNAUTA_WS2812.cpp    # Driver da matriz de LEDs e mapa de caracteres
│   └── mannaNAUTA_WS2812.h      # Protótipos e constantes de tipo GRB e pinagem
├── docs/                        # Imagens de montagem, fiação e registros de teste
└── README.md                    # Documentação técnica do projeto

```

---

## 💻 Como Compilar e Gravar

### Pré-requisitos

* [Arduino IDE](https://www.arduino.cc/en/software) (versão 1.8.19 ou 2.x)
* Pacote de placas **ESP32 by Espressif Systems** (Core estável 2.0.x recomendado)

### Parâmetros da Placa no Arduino IDE

Em **Ferramentas (Tools)**, ajuste rigorosamente as seguintes configurações:

* **Placa:** `ESP32S3 Dev Module`
* **USB CDC On Boot:** `Enabled`
* **CPU Frequency:** `240MHz (WiFi)`
* **Flash Mode:** `QIO 80MHz`
* **Flash Size:** `4MB (32Mb)`
* **Upload Speed:** `115200`
* **Porta:** Selecione a porta correspondente (`COMx` no Windows ou `/dev/ttyACM*` no Linux)

### Procedimento de Upload

1. Conecte o MannaNAUTA ao computador via cabo USB direto (evite hubs sem alimentação externa).
2. Abra o arquivo `game/game.ino` no Arduino IDE (certifique-se de que os arquivos `.h` e `.cpp` estão na mesma aba/pasta).
3. Clique no botão **Carregar (Upload)** e aguarde a mensagem final:
```text
Writing at 0x00010000... (100 %)
Leaving...
Hard resetting via RTS pin...

```


4. Caso a matriz não inicie imediatamente após o término, dê um clique breve no botão físico **RESET** do MannaNAUTA.

---

## 📌 Diretrizes de Manutenção do Projeto

> [!IMPORTANT]
> **Estabilidade Elétrica e Execução Offline:**
> O código foi estruturado propositalmente para rodar de maneira autônoma e sem instanciar pilhas Wi-Fi (`WiFi.h` ou `WebServer.h`). Evite reintroduzir transmissões sem fio simultâneas à animação dos LEDs para impedir quedas de tensão na linha de alimentação USB. O pino físico **GPIO 0** deve permanecer desocupado para garantir reinicializações consistentes sem retenção no bootloader interno.

---
