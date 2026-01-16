# Tarefa_CnnRP2040_Parte2
---

# MNIST TinyML no RP2040 com Display SSD1306

## Objetivo

Este projeto tem como objetivo **executar a inferência de um modelo CNN treinado no dataset MNIST diretamente no Raspberry Pi Pico W (RP2040)** utilizando **TensorFlow Lite Micro**, e **exibir a imagem de entrada (dígito manuscrito) em um display OLED SSD1306** por meio do **protocolo de comunicação serial I2C**.

O foco do projeto é integrar **TinyML + sistemas embarcados**, garantindo que:
- A inferência ocorra totalmente no microcontrolador
- A imagem utilizada na predição seja visualizada no display por I2C
- O fluxo seja reproduzível e didático

---

## Funcionalidades

- Inferência de um modelo CNN **quantizado (INT8)** treinado com MNIST
- Execução local no RP2040 (sem dependência de PC após gravação)
- Comunicação I2C a **400 kHz**
- Exibição da imagem MNIST (28×28) em **modo monocromático** no display SSD1306
- Debug e logs via `printf` (USB)

---

## Fluxo de Execução

1. **Carregar e executar o código padrão**
   - Inicializa o runtime TensorFlow Lite Micro
   - Configura o display SSD1306
   - Prepara os tensores de entrada e saída

2. **Enviar um número diferente para inferência**
   - A imagem MNIST é definida em um header (`mnist_sample.h`)
   - Basta substituir a amostra para testar outro dígito

3. **Exibir o número no display**
   - A matriz 28×28 (escala de cinza) é convertida para **monocromático**
   - Cada pixel é binarizado (0 ou 1) antes de ser desenhado no OLED
   - Apenas a imagem é exibida (nenhum texto no display)

---

## Compilação e Gravação

Este projeto já inclui todas as dependências necessárias e **não requer etapas adicionais de configuração do TensorFlow Lite Micro**.

### Requisitos

- Raspberry Pi Pico SDK instalado e configurado
- CMake ≥ 3.13
- Toolchain ARM (`arm-none-eabi`)
- Extensão *Raspberry Pi Pico* no VS Code (opcional)

### Passos para Compilação

```bash
mkdir build
cd build
cmake ..
make
```

## Gravação na Placa
1. Conecte o Raspberry Pi Pico W ao computador pressionando o botão BOOTSEL
2. Copie o arquivo .uf2 gerado para a unidade removível do Pico 
* O microcontrolador reiniciará automaticamente executando o firmware
---

## Observações sobre o Dataset (Colab)

- Foi adicionada uma **nova célula no Colab** para:
  - Identificar visualmente qual índice do dataset MNIST corresponde a cada dígito (0–9)
- Isso permite:
  - Escolher imagens específicas 
  - Garantir que a imagem enviada ao RP2040 seja conhecida e validada

> O notebook Colab **não faz parte do projeto final**, sendo apenas uma ferramenta auxiliar de preparação dos dados.

---

## Organização do Projeto

- `cnn_mnist.c`  
  Código principal com:
  - Inicialização do RP2040
  - Inferência com TensorFlow Lite Micro
  - Renderização da imagem no SSD1306

- `mnist_sample.h`  
  Contém:
  - A imagem MNIST (28×28)
  - O rótulo esperado

- `lib/ssd1306.*`  
  Biblioteca de controle do display OLED

- `pico-tflmicro/`  
  Runtime do TensorFlow Lite Micro para RP2040

---

## Considerações Finais

Este projeto demonstra um fluxo completo de **TinyML embarcado**, integrando:
- Machine Learning
- Processamento de sinais simples
- Sistemas embarcados
- Comunicação serial por Protocolo I2C
- Interface visual via display OLED

O sistema é ideal para estudos, validação de modelos quantizados e aprendizado prático de inferência em microcontroladores.

---
