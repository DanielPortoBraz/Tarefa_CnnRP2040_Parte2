/**
 * @file cnn_mnist.c
 * @brief Inferência de uma CNN treinada no MNIST (modelo INT8) no Raspberry Pi Pico W (RP2040)
 *        usando TensorFlow Lite Micro + exibição da imagem no ssdlay SSD1306 (OLED).
 */

// =======================
// Bibliotecas padrão
// =======================
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"


// =======================
// Headers do projeto
// =======================
#include "tflm_wrapper.h"
#include "mnist_sample.h"
#include "string.h"
#include "hardware/i2c.h"

// =======================
// Biblioteca do display ssd
// =======================
#include "lib/ssd1306.h"

// =======================
// Definições I2C / ssdlay
// =======================
#define I2C_PORT       i2c1
#define I2C_SDA        14
#define I2C_SCL        15
#define SSD1306_ADDR   0x3C

// =======================
// Funções auxiliares
// =======================

// Retorna o índice do maior valor (argmax) em um vetor int8
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

// Quantização float32 -> int8
static int8_t quantize_f32_to_i8(float x, float scale, int zp) {
    long q = lroundf(x / scale) + zp;
    if (q < -128) q = -128;
    if (q > 127)  q = 127;
    return (int8_t)q;
}

// =======================
// Função principal
// =======================
int main() {

    // -----------------------
    // Inicialização padrão
    // -----------------------
    stdio_init_all();
    sleep_ms(5000);
    printf("\n=== MNIST CNN INT8 no Pico W ===\n");

    // -----------------------
    // Inicialização I2C (400 kHz)
    // -----------------------
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // -----------------------
    // Inicialização do ssdlay SSD1306
    // -----------------------
    ssd1306_t ssd;

    ssd1306_init(
        &ssd,
        WIDTH,
        HEIGHT,
        false,          // charge pump interno
        SSD1306_ADDR,
        I2C_PORT
    );

    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // -----------------------
    // Inicialização do TFLM
    // -----------------------
    int rc = tflm_init();
    if (rc != 0) {
        printf("tflm_init falhou: %d\n", rc);
        while (1) tight_loop_contents();
    }

    printf("Arena usada (bytes): %d\n", tflm_arena_used_bytes());

    int in_bytes = 0;
    int out_bytes = 0;

    int8_t* in  = tflm_input_ptr(&in_bytes);
    int8_t* out = tflm_output_ptr(&out_bytes);

    if (!in || !out) {
        printf("Erro: input/output nulo\n");
        while (1) tight_loop_contents();
    }

    printf("Input bytes: %d | Output bytes: %d\n", in_bytes, out_bytes);

    float in_scale = tflm_input_scale();
    int   in_zp    = tflm_input_zero_point();

    float out_scale = tflm_output_scale();
    int   out_zp    = tflm_output_zero_point();

    printf("IN : scale=%f zp=%d\n", in_scale, in_zp);
    printf("OUT: scale=%f zp=%d\n", out_scale, out_zp);

    // -----------------------
    // Preparar entrada MNIST
    // -----------------------
    for (int i = 0; i < 28 * 28; i++) {
        float x = (float)mnist_sample_28x28[i] / 255.0f;
        in[i] = quantize_f32_to_i8(x, in_scale, in_zp);
    }

    // -----------------------
    // Executar inferência
    // -----------------------
    rc = tflm_invoke();
    if (rc != 0) {
        printf("Invoke falhou: %d\n", rc);
        while (1) tight_loop_contents();
    }

    int pred = argmax_i8(out, 10);

    printf("Label esperado: %d\n", mnist_sample_label);
    printf("Predito: %d\n", pred);

    // -----------------------
    // Mostrar scores
    // -----------------------
    for (int c = 0; c < 10; c++) {
        int8_t q = out[c];
        float y = (float)(q - out_zp) * out_scale;
        printf("c%d: q=%d y~=%f\n", c, (int)q, y);
    }

    // -----------------------
    // Desenhar no OLED
    // -----------------------
    ssd1306_fill(&ssd, false);

    uint8_t x0 = (WIDTH  - 28) / 2;
    uint8_t y0 = (HEIGHT - 28) / 2;

    ssd1306_draw_mnist(&ssd, mnist_sample_28x28, x0, y0);    
    ssd1306_send_data(&ssd);

    // -----------------------
    // Loop final
    // -----------------------
    while (1) {
        sleep_ms(1000);
    }
}
