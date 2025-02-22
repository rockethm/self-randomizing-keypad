#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_LINES 4
#define NUMBERS_PER_LINE 3
#define NUM_MATRIZES 170000

void embaralhar_array(int *array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() % (n - i);  // Índice aleatório
            // Troca os elementos
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void gerar_matriz(int matriz[NUM_LINES][NUMBERS_PER_LINE]) {
    int numeros[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int numeros_usados[12];

    // Embaralha os números
    embaralhar_array(numeros, 10);

    // Preenche os primeiros 10 slots com os números embaralhados
    for (int i = 0; i < 10; i++) {
        numeros_usados[i] = numeros[i];
    }

    // Adiciona 2 duplicatas aleatórias
    for (int i = 10; i < 12; i++) {
        numeros_usados[i] = numeros[rand() % 10];
    }

    // Distribui os números pelas linhas, garantindo que não há duplicatas na mesma linha
    for (int i = 0; i < NUM_LINES; i++) {
        for (int j = 0; j < NUMBERS_PER_LINE; j++) {
            int index = i * NUMBERS_PER_LINE + j;
            matriz[i][j] = numeros_usados[index];

            // Verifica duplicatas na linha atual
            for (int k = 0; k < j; k++) {
                if (matriz[i][j] == matriz[i][k]) {
                    // Se encontrar duplicata, escolhe outro número
                    index = (index + 1) % 12;
                    matriz[i][j] = numeros_usados[index];
                    k = -1;  // Reinicia a verificação
                }
            }
        }
    }
}

void salvar_matrizes(FILE *arquivo, int matrizes[NUM_MATRIZES][NUM_LINES][NUMBERS_PER_LINE]) {
    for (int m = 0; m < NUM_MATRIZES; m++) {
        fprintf(arquivo, "Matriz %d:\n", m + 1);
        for (int i = 0; i < NUM_LINES; i++) {
            for (int j = 0; j < NUMBERS_PER_LINE; j++) {
                fprintf(arquivo, "%d ", matrizes[m][i][j]);
            }
            fprintf(arquivo, "\n");
        }
        fprintf(arquivo, "\n");
    }
}

int main() {
    srand(time(NULL));  // Inicializa a semente para geração de números aleatórios

    int matrizes[NUM_MATRIZES][NUM_LINES][NUMBERS_PER_LINE];

    // Gera 100 matrizes
    for (int i = 0; i < NUM_MATRIZES; i++) {
        gerar_matriz(matrizes[i]);
    }

    // Salva as matrizes em um arquivo
    FILE *arquivo = fopen("matrizes.txt", "w");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    salvar_matrizes(arquivo, matrizes);
    fclose(arquivo);

    printf("Matrizes geradas e salvas em 'matrizes.txt'.\n");

    return 0;
}