/**
 * @file cnn_mnist.c
 * @brief Inferência MNIST INT8 no RP2040 com entrada dinâmica via USB (CDC)
 *        e visualização da imagem no display OLED SSD1306.
 */

// =======================
// Bibliotecas padrão
// =======================
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// =======================
// Headers do projeto
// =======================
#include "tflm_wrapper.h"

// =======================
// Biblioteca do display
// =======================
#include "lib/ssd1306.h"

// =======================
// Definições I2C / Display
// =======================
#define I2C_PORT       i2c1
#define I2C_SDA        14
#define I2C_SCL        15
#define SSD1306_ADDR   0x3C

// =======================
// Constantes MNIST
// =======================
#define MNIST_W 28
#define MNIST_H 28
#define MNIST_SIZE (MNIST_W * MNIST_H)

// Buffer serial suficiente para 784 números (até 4 chars cada)
#define SERIAL_BUFFER_SIZE 4096

// =======================
// Buffers globais
// =======================
static uint8_t mnist_rx_buffer[MNIST_SIZE];

// =======================
// Funções auxiliares
// =======================

// Argmax em vetor int8
static int argmax_i8(const int8_t* v, int n) {
    int best = 0;
    int8_t bestv = v[0];
    for (int i = 1; i < n; i++) {
        if (v[i] > bestv) {
            bestv = v[i];
            best = i;
        }
    }
    return best;
}

// Quantização float -> int8
static int8_t quantize_f32_to_i8(float x, float scale, int zp) {
    long q = lroundf(x / scale) + zp;
    if (q < -128) q = -128;
    if (q > 127)  q = 127;
    return (int8_t)q;
}

// =======================
// Leitura da imagem via USB
// =======================
bool receive_mnist_usb(uint8_t *mnist_buffer) {

    static char serial_buffer[SERIAL_BUFFER_SIZE];

    printf("Envie a amostra MNIST (784 valores 0-255 separados por espaço):\n");

    if (!fgets(serial_buffer, SERIAL_BUFFER_SIZE, stdin)) {
        printf("Erro ao ler da serial\n");
        return false;
    }

    char *ptr = serial_buffer;
    char *end;
    int count = 0;

    while (count < MNIST_SIZE) {

        // Pula espaços e quebras de linha
        while (*ptr == ' ' || *ptr == '\n' || *ptr == '\r' || *ptr == '\t') {
            ptr++;
        }

        if (*ptr == '\0') {
            printf("Amostra incompleta (%d / 784)\n", count);
            return false;
        }

        long value = strtol(ptr, &end, 10);

        if (ptr == end) {
            printf("Erro de parsing no elemento %d\n", count);
            return false;
        }

        if (value < 0)   value = 0;
        if (value > 255) value = 255;

        mnist_buffer[count++] = (uint8_t)value;
        ptr = end;
    }

    printf("Amostra MNIST recebida com sucesso!\n");
    return true;
}

// =======================
// Função principal
// =======================
int main() {

    // -----------------------
    // Inicialização básica
    // -----------------------
    stdio_init_all();
    sleep_ms(5000);
    printf("\n=== MNIST TinyML via USB no Pico W ===\n");

    // -----------------------
    // Inicialização I2C
    // -----------------------
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // -----------------------
    // Inicialização do display
    // -----------------------
    ssd1306_t disp;

    ssd1306_init(
        &disp,
        WIDTH,
        HEIGHT,
        false,
        SSD1306_ADDR,
        I2C_PORT
    );

    ssd1306_config(&disp);
    ssd1306_fill(&disp, false);
    ssd1306_send_data(&disp);

    // -----------------------
    // Inicialização do TFLM
    // -----------------------
    int rc = tflm_init();
    if (rc != 0) {
        printf("Erro tflm_init: %d\n", rc);
        while (1) tight_loop_contents();
    }

    printf("Arena usada: %d bytes\n", tflm_arena_used_bytes());

    int in_bytes = 0;
    int out_bytes = 0;

    int8_t* in  = tflm_input_ptr(&in_bytes);
    int8_t* out = tflm_output_ptr(&out_bytes);

    float in_scale = tflm_input_scale();
    int   in_zp    = tflm_input_zero_point();

    float out_scale = tflm_output_scale();
    int   out_zp    = tflm_output_zero_point();

    printf("IN : scale=%f zp=%d\n", in_scale, in_zp);
    printf("OUT: scale=%f zp=%d\n", out_scale, out_zp);

    // -----------------------
    // Loop principal
    // -----------------------
    while (1) {

        // Recebe imagem via USB
        if (!receive_mnist_usb(mnist_rx_buffer)) {
            printf("Entrada inválida, descartando amostra.\n");
            continue;
        }

        // Mostra imagem no OLED
        ssd1306_fill(&disp, false);
        ssd1306_send_data(&disp);
        uint8_t x0 = (WIDTH  - MNIST_W) / 2;
        uint8_t y0 = (HEIGHT - MNIST_H) / 2;
        ssd1306_draw_mnist(&disp, mnist_rx_buffer, x0, y0);
        ssd1306_send_data(&disp);

        // Pré-processamento + quantização
        for (int i = 0; i < MNIST_SIZE; i++) {
            float x = (float)mnist_rx_buffer[i] / 255.0f;
            in[i] = quantize_f32_to_i8(x, in_scale, in_zp);
        }

        // Inferência
        rc = tflm_invoke();
        if (rc != 0) {
            printf("Erro invoke: %d\n", rc);
            continue;
        }

        // Predição
        int pred = argmax_i8(out, 10);
        printf("Predição: %d\n", pred);

        // Scores
        for (int i = 0; i < 10; i++) {
            float y = (float)(out[i] - out_zp) * out_scale;
            printf("c%d: %.3f  ", i, y);
        }
        printf("\n");

        // Aguarda nova entrada
        printf("Pressione ENTER para enviar nova amostra...\n");
        getchar();
    }
}
