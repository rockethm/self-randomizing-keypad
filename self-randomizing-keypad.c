#include <stdio.h>              //STD
#include "pico/stdlib.h"        //STD

#include "hardware/timer.h"     //Para usar o timer/alarm
#include "ssd1306/ssd1306.h"    //Para usar o display
#include "hardware/i2c.h"       //Para usar o i2c
#include "hardware/adc.h"       //Para usar o ADC de ler o joystic
#include "pico/rand.h"          //Para usar o RNG da Pico
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define BUZZER_PIN 21

const int vrx = 26;
const int vry = 27;
const int ADC_CHANNEL_0 = 0;
const int ADC_CHANNEL_1 = 1;

const uint led_pin_green = 11;
const uint led_pin_red = 13;
const uint16_t PERIOD = 2000;
const float DIVIDER_PWM = 16.0;
uint16_t led_level = 100;

volatile uint8_t linha_atual = 0;

ssd1306_t disp;

const int bt_R = 6;
volatile bool button_pressed = false;
static uint8_t char_count = 0;
static char frase[7];
static absolute_time_t last_button_time = {0};
#define DEBOUNCE_TIME_MS 200 // Debounce time in milliseconds

#define NUM_LINES 4
#define NUMBERS_PER_LINE 3
static int numbers[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

static char pin[6];
static uint8_t selected_lines[6]; // Array to store the selected lines

// Global array to store the current lines and their digits
static int lines[NUM_LINES][NUMBERS_PER_LINE];

void init_srk(){

}

void pwm_init_led(uint led_pin) {
    uint slice = pwm_gpio_to_slice_num(led_pin);
    gpio_set_function(led_pin, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, DIVIDER_PWM);
    pwm_config_set_wrap(&config, PERIOD);
    pwm_init(slice, &config, true);
    pwm_set_gpio_level(led_pin, 0); // Inicialmente desligado
}

void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com valores padrão
    pwm_config config = pwm_get_default_config();
    pwm_init(slice_num, &config, false); // Inicializa o PWM, mas não o habilita ainda

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Função para emitir um beep com frequência e duração especificadas
void beep(uint pin, uint frequency, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Reconfigurar o PWM para a nova frequência
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true); // Reinicializa o PWM com a nova configuração

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);
}

void melodia (bool resultado){
    if (resultado) {
        pwm_set_gpio_level(led_pin_green, led_level);
        beep(BUZZER_PIN, 9956, 125);   // NOTE_A8.5 (3.5 octaves higher than A5)
        beep(BUZZER_PIN, 11178, 125);  // NOTE_B8.5 (3.5 octaves higher than B5)
        beep(BUZZER_PIN, 5916, 125);   // NOTE_C8.5 (3.5 octaves higher than C5)
        beep(BUZZER_PIN, 11178, 125);  // NOTE_B8.5 (3.5 octaves higher than B5)
        beep(BUZZER_PIN, 5916, 125);   // NOTE_C8.5 (3.5 octaves higher than C5)
        beep(BUZZER_PIN, 6641, 125);   // NOTE_D8.5 (3.5 octaves higher than D5)
        beep(BUZZER_PIN, 5916, 125);   // NOTE_C8.5 (3.5 octaves higher than C5)
        beep(BUZZER_PIN, 6641, 125);   // NOTE_D8.5 (3.5 octaves higher than D5)
        beep(BUZZER_PIN, 7457, 125);   // NOTE_E8.5 (3.5 octaves higher than E5)
        beep(BUZZER_PIN, 6641, 125);   // NOTE_D8.5 (3.5 octaves higher than D5)
        beep(BUZZER_PIN, 7457, 125);   // NOTE_E8.5 (3.5 octaves higher than E5)
        beep(BUZZER_PIN, 7457, 125);   // NOTE_E8.5 (3.5 octaves higher than E5)
        pwm_set_gpio_level(led_pin_green, 0);

    } else {
        pwm_set_gpio_level(led_pin_red, led_level);
        beep(BUZZER_PIN, 3136, 500);   // NOTE_G4 (unchanged)
        beep(BUZZER_PIN, 2092, 1000);   // NOTE_C4 (unchanged)
        pwm_set_gpio_level(led_pin_red, 0);
    }
}

void shuffle(int *array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + get_rand_32() % (n - i); // Random index
            int t = array[j]; // Swap elements
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void setup_js() {
    adc_init();
    adc_gpio_init(vrx);
}

//valores de 0 a 4095
void jsread_x(uint16_t *eixo_x) {
    adc_select_input(ADC_CHANNEL_0);
    sleep_us(2);
    *eixo_x = adc_read();
}

// Funcao para configurar o display
void setup_display() {
    i2c_init(i2c1, 400000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp);
}

//quadrado de selecao
void selected(uint8_t linha) {
    uint32_t width = 3;
    uint32_t height = 5;
    uint32_t x = 20;
    uint32_t y = 5;

    switch (linha) {
        case 0:
            y = 5;
            break;
        case 1:
            y = 20;
            break;
        case 2:
            y = 35;
            break;
        case 3:
            y = 50;
            break;
        default:
            y = 5;
            break;
    }

    sleep_ms(50);

    ssd1306_draw_square(&disp, x, y, width, height);
    ssd1306_show(&disp);
}

// Funcao para mostrar uma mensagem
void mostrar_mensagem(char *str, uint32_t x, uint32_t y, bool should_clear) {
    if (should_clear) {
        ssd1306_clear(&disp);
    }
    sleep_ms(50);
    ssd1306_draw_string(&disp, x, y, 1, str);
    ssd1306_show(&disp);
}

void definir_linhas() {
    shuffle(numbers, 10); // Shuffle the numbers array

    int used_numbers[12];

    // Fill the first 10 slots with the shuffled numbers
    for (int i = 0; i < 10; i++) {
        used_numbers[i] = numbers[i];
    }

    // Fill the remaining 2 slots with random duplicates
    for (int i = 10; i < 12; i++) {
        used_numbers[i] = numbers[get_rand_32() % 10]; // Use get_rand_32() for randomness
    }

    // Assign numbers to lines, ensuring no duplicates in the same line
    for (int i = 0; i < NUM_LINES; i++) {
        for (int j = 0; j < NUMBERS_PER_LINE; j++) {
            int index = i * NUMBERS_PER_LINE + j;
            lines[i][j] = used_numbers[index];

            // Check for duplicates in the current line
            for (int k = 0; k < j; k++) {
                if (lines[i][j] == lines[i][k]) {
                    // If a duplicate is found, pick a new number
                    index = (index + 1) % 12; // Move to the next number in used_numbers
                    lines[i][j] = used_numbers[index];
                    k = -1; // Restart the duplicate check for the new number
                }
            }
        }
    }

    // Display the lines
    char linha0[10];
    sprintf(linha0, "%d %d %d", lines[0][0], lines[0][1], lines[0][2]);
    mostrar_mensagem(linha0, 30, 5, true);

    char linha1[10];
    sprintf(linha1, "%d %d %d", lines[1][0], lines[1][1], lines[1][2]);
    mostrar_mensagem(linha1, 30, 20, false);

    char linha2[10];
    sprintf(linha2, "%d %d %d", lines[2][0], lines[2][1], lines[2][2]);
    mostrar_mensagem(linha2, 30, 35, false);

    char linha3[10];
    sprintf(linha3, "%d %d %d", lines[3][0], lines[3][1], lines[3][2]);
    mostrar_mensagem(linha3, 30, 50, false);

    // Reset the selected lines array
    for (int i = 0; i < 6; i++) {
        selected_lines[i] = 0;
    }
}

// verifica input
void check_js() {
    uint16_t valor_x = 0;
    jsread_x(&valor_x);

    if (valor_x < 1500 && linha_atual != 3) {
        linha_atual++;
        ssd1306_clear_square(&disp, 17, 1, 8, 60);
    } else if (valor_x > 2600 && linha_atual != 0) {
        linha_atual--;
        ssd1306_clear_square(&disp, 17, 1, 8, 60);
    }

    selected(linha_atual);
}


//Handler da irq
static void gpio_irq_handler(uint gpio, uint32_t evento) {
    absolute_time_t current_time = get_absolute_time();
    if (absolute_time_diff_us(last_button_time, current_time) > DEBOUNCE_TIME_MS * 1000) {
        button_pressed = true;
        last_button_time = current_time;
    }
}

void correct_pin(uint8_t *selected_lines) {
    // Hardcoded PIN (e.g., 123456)
    uint8_t correct_pin[6] = {1, 2, 3, 4, 5, 6}; // Example PIN

    bool pin_correct = true;
    for (int i = 0; i < 6; i++) {
        bool digit_match = false;
        // Check if any of the 3 digits in the selected line matches the correct PIN digit
        for (int j = 0; j < NUMBERS_PER_LINE; j++) {
            if (lines[selected_lines[i]][j] == correct_pin[i]) {
                digit_match = true;
                break;
            }
        }
        if (!digit_match) {
            pin_correct = false;
            break;
        }
    }

    if (pin_correct) {
        mostrar_mensagem("SENHA CORRETA", 20, 5, true);
    } else {
        mostrar_mensagem("SENHA INCORRETA", 20, 5, true);
    }

    melodia(pin_correct);

    sleep_ms(2500);

    definir_linhas(); // Reset the system
}

int main() {
    stdio_init_all();
    setup_display();
    setup_js();

    gpio_init(bt_R);
    gpio_set_dir(bt_R, GPIO_IN);
    gpio_pull_up(bt_R);
    gpio_set_irq_enabled_with_callback(bt_R, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(led_pin_green);
    gpio_set_dir(led_pin_green, GPIO_OUT);
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);

    // Inicializar PWM para os LEDs
    pwm_init_led(led_pin_green);
    pwm_init_led(led_pin_red);

    // Configuração do GPIO para o buzzer como saída
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    // Inicializar o PWM no pino do buzzer
    pwm_init_buzzer(BUZZER_PIN);

    definir_linhas();

    frase[0] = '\0';

    while (true) {
        check_js();
        if (button_pressed) {
            if (char_count < 6) {
                // Store the selected line
                selected_lines[char_count] = linha_atual;
                frase[char_count] = '*';
                frase[char_count + 1] = '\0';
                char_count++;
            }
            ssd1306_clear_square(&disp, 80, 27, 48, 8); // Wider area to fit 6 characters
            mostrar_mensagem(frase, 80, 27, false);
            button_pressed = false;

            // Check if 6 digits have been entered
            if (char_count == 6) {
                correct_pin(selected_lines); // Verify the PIN
                char_count = 0; // Reset the character count
                frase[0] = '\0'; // Clear the displayed string
            }
        }

        sleep_ms(50); //pequeno delay para melhor controle
    }
}