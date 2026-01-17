# Tarefa_CnnRP2040_Parte2
---

# MNIST TinyML no RP2040 com Display SSD1306 e Entrada via USB Serial

## Objetivo

Este projeto tem como objetivo **executar a inferência de um modelo CNN treinado no dataset MNIST diretamente no Raspberry Pi Pico W (RP2040)** utilizando **TensorFlow Lite Micro**, permitindo:

- **Inferência local (TinyML)**
- **Visualização da imagem MNIST (28×28) em um display OLED SSD1306**
- **Envio dinâmico de amostras via monitor serial USB**

O projeto integra **Machine Learning embarcado, comunicação serial USB, I2C e renderização gráfica**, simulando um fluxo real de teste e validação de modelos TinyML.

## Funcionalidades

- Inferência de modelo CNN **quantizado (INT8)** treinado com MNIST  
- Execução 100% local no RP2040 (TensorFlow Lite Micro)  
- Comunicação I2C a **400 kHz** com display SSD1306  
- Exibição da imagem MNIST **28×28 monocromática**  
- **Recebimento de amostras MNIST via USB (monitor serial)**  
- Logs e debug via `printf` (USB CDC)  
- Compatibilidade com:
  - Amostra fixa (`mnist_sample.h`)
  - Amostras dinâmicas enviadas pelo PC  

## Fluxo de Execução

### Inicialização
- Inicializa RP2040, I2C, SSD1306 e TensorFlow Lite Micro

### Envio da Amostra via USB
- Envio de **784 valores (0–255)** separados por espaço
- A amostra é armazenada, inferida e desenhada no display

### Inferência
- Quantização INT8
- Execução do modelo CNN
- Resultado exibido no terminal
- Imagem exibida no display

## Utilitário `matrix2line.c`

Ferramenta auxiliar que:
- Lê a matriz em `mnist_sample.h`
- Serializa a imagem 28×28
- Imprime em linha única pronta para envio via USB serial

### Uso
```bash
gcc matrix2line.c -o matrix2line
./matrix2line
```

## Compilação

```bash
mkdir build
cd build
cmake ..
make
```

## Organização do Projeto

- `cnn_mnist.c`
- `matrix2line.c`
- `mnist_sample.h`
- `lib/ssd1306.*`
- `pico-tflmicro/`

## Considerações Finais

Projeto completo de **TinyML embarcado**, integrando inferência, comunicação USB, I2C e visualização gráfica.
