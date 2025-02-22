#include <stdio.h>
#include <stdlib.h>

#define NUM_LINES 4
#define NUMBERS_PER_LINE 3
#define NUM_MATRIZES 170000

// Função para verificar se há repetições na mesma linha
int verificar_linha(int linha[NUMBERS_PER_LINE]) {
    for (int i = 0; i < NUMBERS_PER_LINE; i++) {
        for (int j = i + 1; j < NUMBERS_PER_LINE; j++) {
            if (linha[i] == linha[j]) {
                return 0; // Repetição encontrada na linha
            }
        }
    }
    return 1; // Linha válida (sem repetições)
}

// Função para verificar o número máximo de repetições na matriz inteira
int verificar_repeticoes_matriz(int matriz[NUM_LINES][NUMBERS_PER_LINE]) {
    int contagem[10] = {0}; // Contador para cada dígito (0-9)

    // Conta a ocorrência de cada número na matriz
    for (int i = 0; i < NUM_LINES; i++) {
        for (int j = 0; j < NUMBERS_PER_LINE; j++) {
            contagem[matriz[i][j]]++;
        }
    }

    // Verifica se algum número aparece mais de 2 vezes
    for (int i = 0; i < 10; i++) {
        if (contagem[i] > 2) {
            return 0; // Número repetido mais de 2 vezes
        }
    }

    return 1; // Matriz válida (no máximo 2 repetições)
}

// Função para validar uma matriz
int validar_matriz(int matriz[NUM_LINES][NUMBERS_PER_LINE]) {
    // Verifica cada linha
    for (int i = 0; i < NUM_LINES; i++) {
        if (!verificar_linha(matriz[i])) {
            return 0; // Linha inválida
        }
    }

    // Verifica repetições na matriz inteira
    if (!verificar_repeticoes_matriz(matriz)) {
        return 0; // Matriz inválida (mais de 2 repetições)
    }

    return 1; // Matriz válida
}

int main() {
    FILE *arquivo = fopen("matrizes.txt", "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    int matriz[NUM_LINES][NUMBERS_PER_LINE];
    int matrizes_validas = 0;
    int matrizes_invalidas = 0;

    // Lê e valida cada matriz
    for (int m = 0; m < NUM_MATRIZES; m++) {
        // Ignora o cabeçalho "Matriz X:"
        fscanf(arquivo, "Matriz %*d:\n");

        // Lê os valores da matriz
        for (int i = 0; i < NUM_LINES; i++) {
            for (int j = 0; j < NUMBERS_PER_LINE; j++) {
                fscanf(arquivo, "%d", &matriz[i][j]);
            }
        }

        // Valida a matriz
        if (validar_matriz(matriz)) {
            matrizes_validas++;
        } else {
            matrizes_invalidas++;
        }
    }

    fclose(arquivo);

    // Exibe o resultado da validação
    printf("Matrizes válidas: %d\n", matrizes_validas);
    printf("Matrizes inválidas: %d\n", matrizes_invalidas);

    return 0;
}