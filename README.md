# Self-Randomizing Keypad

Sistema de fechadura eletrônica com randomização automática de dígitos para prevenir a visualização da senha por terceiros. Desenvolvido com a BitDogLab (Raspberry Pi Pico W).

![Demonstração do Self-Randomizing Keypad](https://github.com/rockethm/self-randomizing-keypad/raw/main/assets/demo.gif)

## Funcionalidades

- Posições dos dígitos se randomizam automaticamente para prevenir bisbilhotagem
- Display com 4 linhas e 3 dígitos por linha
- Navegação intuitiva utilizando joystick
- Feedback visual através de display OLED e LEDs indicadores
- Feedback sonoro com melodias diferentes para sucesso/falha
- Suporte para senha de 6 dígitos
- Interface I2C para display OLED
- Entrada segura de senha através da seleção de linhas ao invés de dígitos diretos

## Requisitos de Hardware

- Raspberry Pi Pico W
- Display OLED SSD1306 (128x64 pixels, I2C)
- Joystick Analógico
- 2x LEDs (Verde e Vermelho) (ou um LED RGB)
- Botão
- Buzzer Piezoelétrico
- Placa de Desenvolvimento BitDogLab (Opcional, contém todos acima)

## Configuração de Pinos

| Função | Pino | Tipo | Descrição |
|--------|------|------|-----------|
| Display SDA | GPIO 14 | I2C | Linha de dados do display OLED |
| Display SCL | GPIO 15 | I2C | Linha de clock do display OLED |
| Joystick X | GPIO 26 | ADC | Entrada analógica eixo X |
| Joystick Y | GPIO 27 | ADC | Entrada analógica eixo Y (não utilizado) |
| Botão | GPIO 6 | Entrada | Botão de seleção com interrupção |
| LED Verde | GPIO 11 | PWM | Indicador de sucesso |
| LED Vermelho | GPIO 13 | PWM | Indicador de falha |
| Buzzer | GPIO 21 | PWM | Feedback sonoro |

## Compilação e Instalação
*se você quiser apenas rodar o projeto na BitDogLab (ou na Pico W com as conexões indicadas) basta trasnferir o arquivo `.uf2` na pasta build para a placa no modo bootloader*

1. Clone o repositório:
```bash
git clone https://github.com/rockethm/self-randomizing-keypad.git
cd self-randomizing-keypad
```
2. Adicione a extensão da Raspberry Pi Pico no VSCode
3. Importe o projeto pela extensão, selecionando a Pico W e habilitando a extensão CMakeTools
4. Selecione o Kit Pico na extensão CMakeTools
5. Compile o projeto
6. Conecte seu Raspberry Pi Pico W em modo bootloader e copie o arquivo `.uf2` gerado para ele.

## Como Usar

1. O sistema exibe 4 linhas com 3 dígitos aleatórios em cada
2. Use o joystick para mover para cima/baixo entre as linhas
3. Pressione o botão para selecionar a linha que contém o dígito desejado da sua senha
4. Um asterisco (*) aparece para cada dígito inserido
5. Após inserir todos os 6 dígitos:
   - Senha correta: LED Verde + melodia de sucesso
   - Senha incorreta: LED Vermelho + melodia de falha
6. O sistema reinicia automaticamente e randomiza os dígitos para a próxima tentativa

## Estrutura do Projeto

```
self-randomizing-keypad/
├── src/
│   ├── self-randomizing-keypad.c
│   └── ssd1306/
├── include/
│   └── ssd1306/
├── validation/
│   ├── matrix_generator.c
│   └── matrix_validator.c
├── CMakeLists.txt
└── README.md
```

## Recursos de Segurança

- Dígitos são randomizados a cada uso
- Múltiplos dígitos por linha previnem observação direta
- Seleção por linha ao invés de dígito adiciona uma camada extra de ofuscação
- Dois números duplicados por matriz aumentam a dificuldade de adivinhação
- Nenhum dígito se repete na mesma linha

## Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## Agradecimentos

- Equipes Embarcatech e HBR pela capacitação
- BitDogLab pela placa de desenvolvimento e suporte
- Raspberry Pi Foundation pela excelente documentação do RP2040

## Autor

Andre de Oliveira Melo

## Links

- [Repositório do Projeto](https://github.com/rockethm/self-randomizing-keypad)
- [Vídeo de Demonstração](https://youtu.be/u4Ev4Xev3Qc?si=ciYzQw0AWk5lCFQp)
