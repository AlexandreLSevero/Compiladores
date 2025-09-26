// Nome: Alexandre Luppi Severo e Silva

#include <stdio.h>
#include <stdlib.h>
#include "analex.h"

// Função auxiliar para imprimir o átomo reconhecido
void imprimir_atomo(TInfoAtomo atomo) {
    printf("Linha %-4d Tipo: %-25d", atomo.linha, atomo.tipo);
    
    switch (atomo.tipo) {
        // Exemplos de Palavras-chave
        case TK_PALAVRA_PRG: printf("Lexema: prg\n"); break;
        case TK_PALAVRA_INT: printf("Lexema: int\n"); break;
        case TK_PALAVRA_BEGIN: printf("Lexema: begin\n"); break;
        case TK_PALAVRA_END: printf("Lexema: end\n"); break;
        
        // Identificadores e Constantes
        case TK_IDENTIFICADOR: printf("Lexema: %s (ID)\n", atomo.lexema.str_val); break;
        case TK_CONST_INTEIRA: printf("Lexema: %d (INT)\n", atomo.lexema.int_val); break;
        case TK_CONST_REAL: printf("Lexema: %f (FLOAT)\n", atomo.lexema.real_val); break;
        case TK_CONST_STRING: printf("Lexema: \"%s\" (STRING)\n", atomo.lexema.str_val); break;
        case TK_CONST_CARACTERE: printf("Lexema: '%c' (CHAR)\n", atomo.lexema.str_val[0]); break;

        // Operadores e Delimitadores
        case TK_OP_ATRIBUICAO: printf("Lexema: <-\n"); break;
        case TK_OP_MAIOR_IGUAL: printf("Lexema: >=\n"); break;
        case TK_OP_IGUALDADE: printf("Lexema: ==\n"); break;
        case TK_DELIMITADOR_PONTO_VIRGULA: printf("Lexema: ;\n"); break;
        case TK_DELIMITADOR_PONTO: printf("Lexema: .\n"); break;
        case TK_OP_SOMA: printf("Lexema: +\n"); break;
        case TK_OP_SUBTRACAO: printf("Lexema: -\n"); break;
        
        // Especiais
        case TK_EOF: printf("--- FIM DE ARQUIVO ---\n"); break;
        case TK_ERRO_LEXICO: printf("### ERRO LEXICO ###\n"); break;

        default: 
            // Para todos os outros, apenas imprime o tipo
            printf("Lexema: (Outro Símbolo)\n");
            break;
    }
}

int main() {
    // Crie um arquivo chamado 'teste.lpd' com o código abaixo
    const char *CODIGO_FONTE = "teste.lpd"; 
    
    // --- CÓDIGO DE TESTE ---
    /*
    prg Exemplo_01;
    begin
        write("Olá mundo!");
    end.

    */
    
    // Simulação de criação do arquivo de teste
    FILE *f_test = fopen(CODIGO_FONTE, "w");
    if (f_test) {
        fprintf(f_test, "prg Exemplo_01;\n");
        fprintf(f_test, "begin\n");
        fprintf(f_test, " write(\"Olá mundo!\");\n");
        fprintf(f_test, "end\n");
        fclose(f_test);
    } else {
        printf("Não foi possível criar o arquivo de teste!\n");
        return 1;
    }

    inicializar_analex(CODIGO_FONTE);

    TInfoAtomo atomo;
    do {
        atomo = obter_atomo();
        imprimir_atomo(atomo);
        if (atomo.tipo == TK_ERRO_LEXICO) break;
    } while (atomo.tipo != TK_EOF);

    fechar_analex();

    return 0;
}
