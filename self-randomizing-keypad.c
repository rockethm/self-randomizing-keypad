/**
 * @file self-randomizing-keypad.c
 * @brief Sistema de teclado com randomização de dígitos para entrada segura de senha
 * @author Andre de Oliveira Melo
 * @date Fevereiro 2025
 * 
 * O sistema implementa um teclado de segurança onde os dígitos são
 * randomizados a cada uso. O usuário navega entre linhas com um joystick
 * e seleciona uma linha que contenha o dígito desejado da senha.
 */

 #include <stdio.h>              // Biblioteca padrão
 #include "pico/stdlib.h"        // Biblioteca padrão do Pico
 #include "hardware/timer.h"     // Para usar o timer/alarm
 #include "ssd1306/ssd1306.h"    // Para usar o display OLED
 #include "hardware/i2c.h"       // Para comunicação I2C
 #include "hardware/adc.h"       // Para leitura do joystick via ADC
 #include "pico/rand.h"          // Para geração de números aleatórios
 #include "hardware/pwm.h"       // Para controle PWM (LEDs e buzzer)
 #include "hardware/clocks.h"    // Para configuração de clock
 
 /** 
  * @defgroup PINS Definições de Pinos
  * @{
  */
 #define BUZZER_PIN 21
 #define LED_PIN_GREEN 11
 #define LED_PIN_RED 13
 #define JOYSTICK_X 26
 #define JOYSTICK_Y 27
 #define BUTTON_R 6
 /**
  * @}
  */
 
 /** 
  * @defgroup ADC_CHANNELS Canais ADC 
  * @{
  */
 #define ADC_CHANNEL_0 0  // Canal para leitura X do joystick
 #define ADC_CHANNEL_1 1  // Canal para leitura Y do joystick
 /**
  * @}
  */
 
 /** 
  * @defgroup PWM_CONFIG Configuração do PWM
  * @{
  */
 #define PWM_PERIOD 2000
 #define PWM_DIVIDER 16.0
 #define PWM_LED_LEVEL 100
 /**
  * @}
  */
 
 /** 
  * @defgroup KEYPAD_CONFIG Configuração do Teclado
  * @{
  */
 #define NUM_LINES 4           // Número de linhas no teclado
 #define NUMBERS_PER_LINE 3    // Número de dígitos por linha
 #define PIN_LENGTH 6          // Tamanho da senha
 #define DEBOUNCE_TIME_MS 200  // Tempo de debounce em milissegundos
 /**
  * @}
  */
 
 /**
  * @brief Estrutura do display OLED
  */
 ssd1306_t disp;
 
 /**
  * @brief Variáveis globais do sistema
  */
 static volatile uint8_t linha_atual = 0;        // Linha selecionada atualmente
 static volatile bool button_pressed = false;    // Flag para botão pressionado
 static uint8_t char_count = 0;                  // Contador de caracteres digitados
 static char senha_display[PIN_LENGTH + 1];      // String para exibir asteriscos da senha
 static absolute_time_t last_button_time = {0};  // Timestamp do último pressionamento de botão
 
 /**
  * @brief Arrays para armazenamento das configurações do teclado
  */
 static int numeros[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};              // Dígitos possíveis
 static uint8_t linhas_selecionadas[PIN_LENGTH];                      // Linhas selecionadas pelo usuário
 static int matriz_digitos[NUM_LINES][NUMBERS_PER_LINE];              // Matriz de dígitos nas linhas
 
 /**
  * @brief Protótipos de funções
  */
 // Funções de inicialização
 void inicializar_display(void);
 void inicializar_joystick(void);
 void inicializar_pwm_led(uint led_pin);
 void inicializar_pwm_buzzer(uint pin);
 
 // Funções de interface
 void mostrar_mensagem(char *str, uint32_t x, uint32_t y, bool limpar_tela);
 void mostrar_selecao(uint8_t linha);
 void definir_linhas(void);
 
 // Funções de entrada
 void verificar_joystick(void);
 void ler_joystick_x(uint16_t *eixo_x);
 static void manipulador_interrupcao_gpio(uint gpio, uint32_t evento);
 
 // Funções de áudio e feedback
 void emitir_beep(uint pin, uint frequencia, uint duracao_ms);
 void tocar_melodia(bool resultado);
 
 // Funções de processamento
 void embaralhar_array(int *array, size_t n);
 void verificar_senha(uint8_t *linhas_selecionadas);
 
 /**
  * @brief Inicializa o display OLED via I2C
  */
 void inicializar_display(void) {
     i2c_init(i2c1, 400000);  // Inicializa I2C a 400kHz
     
     // Configura pinos I2C
     gpio_set_function(14, GPIO_FUNC_I2C);  // SDA
     gpio_set_function(15, GPIO_FUNC_I2C);  // SCL
     gpio_pull_up(14);
     gpio_pull_up(15);
 
     // Inicializa display OLED
     disp.external_vcc = false;
     ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
     ssd1306_clear(&disp);
 }
 
 /**
  * @brief Inicializa o ADC para leitura do joystick
  */
 void inicializar_joystick(void) {
     adc_init();
     adc_gpio_init(JOYSTICK_X);
 }
 
 /**
  * @brief Inicializa o PWM para controle dos LEDs
  * 
  * @param led_pin Pino do LED a ser inicializado
  */
 void inicializar_pwm_led(uint led_pin) {
     uint slice = pwm_gpio_to_slice_num(led_pin);
     gpio_set_function(led_pin, GPIO_FUNC_PWM);
     
     // Configura PWM
     pwm_config config = pwm_get_default_config();
     pwm_config_set_clkdiv(&config, PWM_DIVIDER);
     pwm_config_set_wrap(&config, PWM_PERIOD);
     
     pwm_init(slice, &config, true);
     pwm_set_gpio_level(led_pin, 0); // Inicialmente desligado
 }
 
 /**
  * @brief Inicializa o PWM para o buzzer
  * 
  * @param pin Pino do buzzer
  */
 void inicializar_pwm_buzzer(uint pin) {
     gpio_set_function(pin, GPIO_FUNC_PWM);
     uint slice_num = pwm_gpio_to_slice_num(pin);
     
     pwm_config config = pwm_get_default_config();
     pwm_init(slice_num, &config, false);
     
     pwm_set_gpio_level(pin, 0);
 }
 
 /**
  * @brief Emite um beep com frequência e duração especificadas
  * 
  * @param pin Pino do buzzer
  * @param frequencia Frequência do beep em Hz
  * @param duracao_ms Duração do beep em milissegundos
  */
 void emitir_beep(uint pin, uint frequencia, uint duracao_ms) {
     uint slice_num = pwm_gpio_to_slice_num(pin);
     
     // Configura PWM para frequência desejada
     pwm_config config = pwm_get_default_config();
     pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequencia * 4096));
     pwm_init(slice_num, &config, true);
     
     // 50% duty cycle
     pwm_set_gpio_level(pin, 2048);
     
     sleep_ms(duracao_ms);
     
     // Desativa saída
     pwm_set_gpio_level(pin, 0);
 }
 
 /**
  * @brief Toca melodia de acordo com o resultado da validação da senha
  * 
  * @param resultado true para senha correta, false para incorreta
  */
 void tocar_melodia(bool resultado) {
     if (resultado) {
         // Acende LED verde
         pwm_set_gpio_level(LED_PIN_GREEN, PWM_LED_LEVEL);
         
         // Melodia para senha correta
         emitir_beep(BUZZER_PIN, 9956, 125);
         emitir_beep(BUZZER_PIN, 11178, 125);
         emitir_beep(BUZZER_PIN, 5916, 125);
         emitir_beep(BUZZER_PIN, 11178, 125);
         emitir_beep(BUZZER_PIN, 5916, 125);
         emitir_beep(BUZZER_PIN, 6641, 125);
         emitir_beep(BUZZER_PIN, 5916, 125);
         emitir_beep(BUZZER_PIN, 6641, 125);
         emitir_beep(BUZZER_PIN, 7457, 125);
         emitir_beep(BUZZER_PIN, 6641, 125);
         emitir_beep(BUZZER_PIN, 7457, 125);
         emitir_beep(BUZZER_PIN, 7457, 125);
         
         // Apaga LED verde
         pwm_set_gpio_level(LED_PIN_GREEN, 0);
     } else {
         // Acende LED vermelho
         pwm_set_gpio_level(LED_PIN_RED, PWM_LED_LEVEL);
         
         // Melodia para senha incorreta
         emitir_beep(BUZZER_PIN, 3136, 500);
         emitir_beep(BUZZER_PIN, 2092, 1000);
         
         // Apaga LED vermelho
         pwm_set_gpio_level(LED_PIN_RED, 0);
     }
 }
 
 /**
  * @brief Embaralha um array usando o gerador de números aleatórios do Pico
  * 
  * @param array Array a ser embaralhado
  * @param n Tamanho do array
  */
 void embaralhar_array(int *array, size_t n) {
     if (n > 1) {
         for (size_t i = 0; i < n - 1; i++) {
             size_t j = i + get_rand_32() % (n - i);  // Índice aleatório
             // Troca os elementos
             int t = array[j];
             array[j] = array[i];
             array[i] = t;
         }
     }
 }
 
 /**
  * @brief Lê o valor do eixo X do joystick via ADC
  * 
  * @param eixo_x Ponteiro para armazenar valor lido (0-4095)
  */
 void ler_joystick_x(uint16_t *eixo_x) {
     adc_select_input(ADC_CHANNEL_0);
     sleep_us(2);  // Pequeno delay para estabilização
     *eixo_x = adc_read();
 }
 
 /**
  * @brief Mostra uma mensagem no display OLED
  * 
  * @param str String a ser mostrada
  * @param x Posição X no display
  * @param y Posição Y no display
  * @param limpar_tela Se true, limpa o display antes de desenhar
  */
 void mostrar_mensagem(char *str, uint32_t x, uint32_t y, bool limpar_tela) {
     if (limpar_tela) {
         ssd1306_clear(&disp);
     }
     sleep_ms(50);  // Pequeno delay para estabilização
     ssd1306_draw_string(&disp, x, y, 1, str);
     ssd1306_show(&disp);
 }
 
 /**
  * @brief Mostra um indicador de seleção para a linha atual
  * 
  * @param linha Índice da linha selecionada (0-3)
  */
 void mostrar_selecao(uint8_t linha) {
     uint32_t width = 3;
     uint32_t height = 5;
     uint32_t x = 20;
     uint32_t y;
     
     // Determina posição Y baseada na linha
     switch (linha) {
         case 0: y = 5; break;
         case 1: y = 20; break;
         case 2: y = 35; break;
         case 3: y = 50; break;
         default: y = 5; break;
     }
     
     sleep_ms(50);
     
     // Desenha quadrado de seleção
     ssd1306_draw_square(&disp, x, y, width, height);
     ssd1306_show(&disp);
 }
 
 /**
  * @brief Define e mostra as linhas de números randomizados no display
  */
 void definir_linhas(void) {
     embaralhar_array(numeros, 10);  // Embaralha array de números
     
     int numeros_usados[12];
     
     // Preenche os primeiros 10 slots com os números embaralhados
     for (int i = 0; i < 10; i++) {
         numeros_usados[i] = numeros[i];
     }
     
     // Adiciona 2 duplicatas aleatórias
     for (int i = 10; i < 12; i++) {
         numeros_usados[i] = numeros[get_rand_32() % 10];
     }
     
     // Distribui os números pelas linhas, garantindo que não há duplicatas na mesma linha
     for (int i = 0; i < NUM_LINES; i++) {
         for (int j = 0; j < NUMBERS_PER_LINE; j++) {
             int index = i * NUMBERS_PER_LINE + j;
             matriz_digitos[i][j] = numeros_usados[index];
             
             // Verifica duplicatas na linha atual
             for (int k = 0; k < j; k++) {
                 if (matriz_digitos[i][j] == matriz_digitos[i][k]) {
                     // Se encontrar duplicata, escolhe outro número
                     index = (index + 1) % 12;
                     matriz_digitos[i][j] = numeros_usados[index];
                     k = -1;  // Reinicia a verificação
                 }
             }
         }
     }
     
     // Mostra as linhas no display
     char buffer[10];
     
     // Linha 0
     sprintf(buffer, "%d %d %d", matriz_digitos[0][0], matriz_digitos[0][1], matriz_digitos[0][2]);
     mostrar_mensagem(buffer, 30, 5, true);
     
     // Linha 1
     sprintf(buffer, "%d %d %d", matriz_digitos[1][0], matriz_digitos[1][1], matriz_digitos[1][2]);
     mostrar_mensagem(buffer, 30, 20, false);
     
     // Linha 2
     sprintf(buffer, "%d %d %d", matriz_digitos[2][0], matriz_digitos[2][1], matriz_digitos[2][2]);
     mostrar_mensagem(buffer, 30, 35, false);
     
     // Linha 3
     sprintf(buffer, "%d %d %d", matriz_digitos[3][0], matriz_digitos[3][1], matriz_digitos[3][2]);
     mostrar_mensagem(buffer, 30, 50, false);
     
     // Reinicia array de linhas selecionadas
     for (int i = 0; i < PIN_LENGTH; i++) {
         linhas_selecionadas[i] = 0;
     }
 }
 
 /**
  * @brief Verifica entrada do joystick e atualiza linha selecionada
  */
 void verificar_joystick(void) {
     uint16_t valor_x = 0;
     ler_joystick_x(&valor_x);
     
     // Baseado no valor lido, move para cima ou para baixo
     if (valor_x < 1500 && linha_atual != 3) {
         linha_atual++;
         ssd1306_clear_square(&disp, 17, 1, 8, 60);
     } else if (valor_x > 2600 && linha_atual != 0) {
         linha_atual--;
         ssd1306_clear_square(&disp, 17, 1, 8, 60);
     }
     
     mostrar_selecao(linha_atual);
 }
 
 /**
  * @brief Manipulador de interrupção para o botão
  * 
  * @param gpio Pino GPIO que gerou a interrupção
  * @param evento Tipo de evento que causou a interrupção
  */
 static void manipulador_interrupcao_gpio(uint gpio, uint32_t evento) {
     absolute_time_t tempo_atual = get_absolute_time();
     
     // Verifica debounce
     if (absolute_time_diff_us(last_button_time, tempo_atual) > DEBOUNCE_TIME_MS * 1000) {
         button_pressed = true;
         last_button_time = tempo_atual;
     }
 }
 
 /**
  * @brief Verifica se a senha digitada está correta
  * 
  * @param linhas_selecionadas Array com as linhas selecionadas pelo usuário
  */
 void verificar_senha(uint8_t *linhas_selecionadas) {
     // Senha correta codificada (exemplo: 123456)
     uint8_t senha_correta[PIN_LENGTH] = {1, 2, 3, 4, 5, 6};
     
     bool senha_valida = true;
     
     // Verifica cada dígito da senha
     for (int i = 0; i < PIN_LENGTH; i++) {
         bool digito_encontrado = false;
         
         // Verifica se o dígito correto está presente na linha selecionada
         for (int j = 0; j < NUMBERS_PER_LINE; j++) {
             if (matriz_digitos[linhas_selecionadas[i]][j] == senha_correta[i]) {
                 digito_encontrado = true;
                 break;
             }
         }
         
         if (!digito_encontrado) {
             senha_valida = false;
             break;
         }
     }
     
     // Mostra resultado
     if (senha_valida) {
         mostrar_mensagem("SENHA CORRETA", 20, 5, true);
     } else {
         mostrar_mensagem("SENHA INCORRETA", 20, 5, true);
     }
     
     // Toca melodia de acordo com resultado
     tocar_melodia(senha_valida);
     
     // Aguarda e reinicia o sistema
     sleep_ms(2500);
     definir_linhas();
 }

  /**
  * @brief Função de inicialização do sistema e dispositivos
  */
 void srk_init(){
     // Inicialização do sistema
     stdio_init_all();
     inicializar_display();
     inicializar_joystick();
     
     // Configura botão com pull-up e interrupção
     gpio_init(BUTTON_R);
     gpio_set_dir(BUTTON_R, GPIO_IN);
     gpio_pull_up(BUTTON_R);
     gpio_set_irq_enabled_with_callback(BUTTON_R, GPIO_IRQ_EDGE_FALL, true, &manipulador_interrupcao_gpio);
     
     // Configura LEDs
     gpio_init(LED_PIN_GREEN);
     gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
     gpio_init(LED_PIN_RED);
     gpio_set_dir(LED_PIN_RED, GPIO_OUT);
     
     // Inicializa PWM para LEDs
     inicializar_pwm_led(LED_PIN_GREEN);
     inicializar_pwm_led(LED_PIN_RED);
     
     // Configura buzzer
     gpio_init(BUZZER_PIN);
     gpio_set_dir(BUZZER_PIN, GPIO_OUT);
     inicializar_pwm_buzzer(BUZZER_PIN);
     
     // Inicializa teclado randomizado
     definir_linhas();
     
     // Inicializa string de exibição da senha
     senha_display[0] = '\0';
 }
 
 /**
  * @brief Função principal
  */
 int main() {

    // Inicialização do sistema e dispositivos
    srk_init();
     
     // Loop principal
     while (true) {
         // Verifica joystick para navegação
         verificar_joystick();
         
         // Processa botão pressionado
         if (button_pressed) {
             if (char_count < PIN_LENGTH) {
                 // Armazena a linha selecionada
                 linhas_selecionadas[char_count] = linha_atual;
                 
                 // Atualiza string de exibição (asteriscos)
                 senha_display[char_count] = '*';
                 senha_display[char_count + 1] = '\0';
                 char_count++;
             }
             
             // Atualiza display
             ssd1306_clear_square(&disp, 80, 27, 48, 8);
             mostrar_mensagem(senha_display, 80, 27, false);
             button_pressed = false;
             
             // Se completou a senha, verifica
             if (char_count == PIN_LENGTH) {
                 verificar_senha(linhas_selecionadas);
                 char_count = 0;
                 senha_display[0] = '\0';
             }
         }
         
         sleep_ms(50);  // Pequeno delay para melhor controle
     }
 }