#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "analex.h"

// --- VARIÁVEIS GLOBAIS DE ESTADO DO SCANNER ---
static FILE *arquivo_fonte = NULL;
static int linha_atual = 1;

// --- FUNÇÕES AUXILIARES DE ENTRADA (Input Buffer) ---

// Lê o próximo caractere do arquivo
static char obter_char() {
    char c = fgetc(arquivo_fonte);
    if (c == '\n') {
        linha_atual++;
    }
    return c;
}

// Retorna o caractere lido para o buffer do arquivo (para lookahead)
static void devolver_char(char c) {
    if (c != EOF) {
        ungetc(c, arquivo_fonte);
        if (c == '\n') {
            linha_atual--;
        }
    }
}

// --- TABELA DE PALAVRAS RESERVADAS ---
// Usamos uma estrutura para mapear a string para o tipo de token
typedef struct {
    const char *lexema;
    TIPO_ATOMO tipo;
} PalavraReservada;

static PalavraReservada tabela_palavras[] = {
    {"and", TK_PALAVRA_AND}, {"begin", TK_PALAVRA_BEGIN}, {"char", TK_PALAVRA_CHAR},
    {"else", TK_PALAVRA_ELSE}, {"end", TK_PALAVRA_END}, {"float", TK_PALAVRA_FLOAT},
    {"for", TK_PALAVRA_FOR}, {"if", TK_PALAVRA_IF}, {"int", TK_PALAVRA_INT},
    {"not", TK_PALAVRA_NOT}, {"or", TK_PALAVRA_OR}, {"prg", TK_PALAVRA_PRG},
    {"read", TK_PALAVRA_READ}, {"repeat", TK_PALAVRA_REPEAT}, {"return", TK_PALAVRA_RETURN},
    {"subrot", TK_PALAVRA_SUBROT}, {"then", TK_PALAVRA_THEN}, {"until", TK_PALAVRA_UNTIL},
    {"var", TK_PALAVRA_VAR}, {"void", TK_PALAVRA_VOID}, {"while", TK_PALAVRA_WHILE},
    {"write", TK_PALAVRA_WRITE},
    {NULL, TK_ERRO_LEXICO} // Sentinela
};

// --- INICIALIZAÇÃO E FINALIZAÇÃO DO SCANNER ---

void inicializar_analex(const char *nome_arquivo_fonte) {
    arquivo_fonte = fopen(nome_arquivo_fonte, "r");
    if (!arquivo_fonte) {
        perror("Erro ao abrir arquivo fonte");
        exit(EXIT_FAILURE);
    }
    printf("AnaLex inicializado. Lendo '%s'.\n", nome_arquivo_fonte);
}

void fechar_analex(void) {
    if (arquivo_fonte) {
        fclose(arquivo_fonte);
        arquivo_fonte = NULL;
    }
    printf("AnaLex finalizado.\n");
}

// --- FUNÇÃO PRINCIPAL: obter_atomo() ---

TInfoAtomo obter_atomo(void) {
    char c;
    TInfoAtomo atomo;
    atomo.linha = linha_atual; // Começa registrando a linha atual

    // 1. IGNORAR ESPAÇOS EM BRANCO E COMENTÁRIOS
    while (1) {
        c = obter_char();
        
        // Ignorar espaços em branco
        if (isspace(c)) {
            atomo.linha = linha_atual; // Atualiza a linha caso tenha pulado várias quebras de linha
            continue;
        }

        // Ignorar Comentários { ... }
        if (c == '{') {
            int linha_inicio_comentario = linha_atual;
            while ((c = obter_char()) != '}' && c != EOF);
            
            if (c == EOF) {
                fprintf(stderr, "ERRO LEXICO (Linha %d): Comentário aberto não fechado.\n", linha_inicio_comentario);
                atomo.tipo = TK_ERRO_LEXICO;
                return atomo;
            }
            // Comentário fechado, continua o loop para ignorar espaços e novos comentários
            continue; 
        }

        // Se não for espaço ou comentário, sai do loop de limpeza
        break;
    }

    // Se o caractere for EOF após pular espaços, retorna FIM DE ARQUIVO
    if (c == EOF) {
        atomo.tipo = TK_EOF;
        return atomo;
    }
    
    // --- 2. RECONHECIMENTO DE IDENTIFICADORES E PALAVRAS-CHAVE ---
    if (isalpha(c)) {
        char buffer[TAM_MAX_LEXEMA_STR];
        int i = 0;
        
        // Lê enquanto for letra, número ou underscore (padrão comum)
        while (isalnum(c) || c == '_') {
            if (i < TAM_MAX_LEXEMA_STR - 1) {
                buffer[i++] = c;
            } else {
                // Erro: Identificador muito longo (LPD diz 1 a 255)
                buffer[TAM_MAX_LEXEMA_STR - 1] = '\0'; // Trunca e continua para reconhecer o resto
                fprintf(stderr, "AVISO LEXICO (Linha %d): Identificador '%s...' truncado.\n", atomo.linha, buffer);
            }
            c = obter_char();
        }
        
        devolver_char(c); // Devolve o caractere que não é mais parte do lexema
        buffer[i] = '\0';
        strcpy(atomo.lexema.str_val, buffer);

        // Checa se é uma Palavra Reservada
        for (i = 0; tabela_palavras[i].lexema != NULL; i++) {
            if (strcmp(buffer, tabela_palavras[i].lexema) == 0) {
                atomo.tipo = tabela_palavras[i].tipo;
                return atomo;
            }
        }

        // Não é palavra reservada, é um identificador
        atomo.tipo = TK_IDENTIFICADOR;
        return atomo;
    }

    // --- 3. RECONHECIMENTO DE CONSTANTES NUMÉRICAS ---
    if (isdigit(c)) {
        char buffer[TAM_MAX_LEXEMA_STR];
        int i = 0;
        int is_float = 0;

        while (isdigit(c) || c == '.') {
            if (c == '.') {
                if (is_float) { 
                    // Se já encontrou um '.', é erro léxico
                    fprintf(stderr, "ERRO LEXICO (Linha %d): Número com múltiplos pontos decimais.\n", atomo.linha);
                    devolver_char(c);
                    atomo.tipo = TK_ERRO_LEXICO;
                    return atomo;
                }
                is_float = 1;
            }
            
            if (i < TAM_MAX_LEXEMA_STR - 1) {
                buffer[i++] = c;
            }
            c = obter_char();
        }
        
        devolver_char(c); // Devolve o caractere que não é mais um dígito/ponto
        buffer[i] = '\0';
        
        if (is_float) {
            atomo.tipo = TK_CONST_REAL;
            atomo.lexema.real_val = strtod(buffer, NULL); // Converte para double
        } else {
            atomo.tipo = TK_CONST_INTEIRA;
            atomo.lexema.int_val = atoi(buffer); // Converte para inteiro
        }
        return atomo;
    }

    // --- 4. RECONHECIMENTO DE OPERADORES E DELIMITADORES (Multi/Simples) ---
    char prox_c;
    switch (c) {
        // Operadores de 2 ou 1 Caractere
        case '<':
            prox_c = obter_char();
            if (prox_c == '-') {
                atomo.tipo = TK_OP_ATRIBUICAO; // <-
            } else if (prox_c == '=') {
                atomo.tipo = TK_OP_MENOR_IGUAL; // <=
            } else {
                devolver_char(prox_c); // Devolve o caractere lido
                atomo.tipo = TK_OP_MENOR; // <
            }
            return atomo;

        case '>':
            prox_c = obter_char();
            if (prox_c == '=') {
                atomo.tipo = TK_OP_MAIOR_IGUAL; // >=
            } else {
                devolver_char(prox_c); // Devolve o caractere lido
                atomo.tipo = TK_OP_MAIOR; // >
            }
            return atomo;
            
        case '=':
            prox_c = obter_char();
            if (prox_c == '=') {
                atomo.tipo = TK_OP_IGUALDADE; // ==
            } else {
                // O manual não especifica o que é um '=' sozinho. 
                // Assumimos que é erro ou devolvemos para o analisador sintático.
                // Como não está na lista de operadores válidos, tratamos como erro léxico.
                fprintf(stderr, "ERRO LEXICO (Linha %d): Símbolo '=' inválido. Use '==' para igualdade ou '<-' para atribuição.\n", atomo.linha);
                devolver_char(prox_c);
                atomo.tipo = TK_ERRO_LEXICO;
            }
            return atomo;
            
        case '!':
            prox_c = obter_char();
            if (prox_c == '=') {
                atomo.tipo = TK_OP_DIFERENCA; // !=
            } else {
                // '!' sozinho não é um token válido na LPD
                fprintf(stderr, "ERRO LEXICO (Linha %d): Símbolo '!' inválido. Use '!=' para diferença.\n", atomo.linha);
                devolver_char(prox_c);
                atomo.tipo = TK_ERRO_LEXICO;
            }
            return atomo;

        // Operadores/Delimitadores de 1 Caractere
        case '+': atomo.tipo = TK_OP_SOMA; return atomo;
        case '-': atomo.tipo = TK_OP_SUBTRACAO; return atomo;
        case '*': atomo.tipo = TK_OP_MULTIPLICACAO; return atomo;
        case '/': atomo.tipo = TK_OP_DIVISAO; return atomo;
        case '(': atomo.tipo = TK_DELIMITADOR_ABRE_PARENTESES; return atomo;
        case ')': atomo.tipo = TK_DELIMITADOR_FECHA_PARENTESES; return atomo;
        case '[': atomo.tipo = TK_DELIMITADOR_ABRE_COLCHETES; return atomo;
        case ']': atomo.tipo = TK_DELIMITADOR_FECHA_COLCHETES; return atomo;
        case ';': atomo.tipo = TK_DELIMITADOR_PONTO_VIRGULA; return atomo;
        case ',': atomo.tipo = TK_DELIMITADOR_VIRGULA; return atomo;
        case ':': atomo.tipo = TK_DELIMITADOR_DOISPONTOS; return atomo;
        case '.': atomo.tipo = TK_DELIMITADOR_PONTO; return atomo;

        // --- 5. RECONHECIMENTO DE CONSTANTES STRING/CARACTERE ---
        case '"': 
            // Implementação simplificada de String (ignora escapes)
            {
                int i = 0;
                while ((c = obter_char()) != '"' && c != EOF && c != '\n') {
                    if (i < TAM_MAX_LEXEMA_STR - 1) {
                         atomo.lexema.str_val[i++] = c;
                    }
                }
                atomo.lexema.str_val[i] = '\0';
                
                if (c != '"') {
                    fprintf(stderr, "ERRO LEXICO (Linha %d): String aberta não fechada.\n", atomo.linha);
                    atomo.tipo = TK_ERRO_LEXICO;
                    return atomo;
                }
                atomo.tipo = TK_CONST_STRING;
                return atomo;
            }

        case '\'':
            // Implementação de Caractere Simples
            {
                c = obter_char(); // O caractere em si
                prox_c = obter_char(); // A aspa de fechamento

                if (prox_c != '\'') {
                    // Erro: Múltiplos caracteres ou aspa de fechamento faltando
                    fprintf(stderr, "ERRO LEXICO (Linha %d): Constante caractere mal formada ou ausente.\n", atomo.linha);
                    devolver_char(prox_c); // Devolve o que foi lido a mais
                    devolver_char(c);
                    atomo.tipo = TK_ERRO_LEXICO;
                    return atomo;
                }
                atomo.lexema.str_val[0] = c;
                atomo.lexema.str_val[1] = '\0';
                atomo.tipo = TK_CONST_CARACTERE;
                return atomo;
            }

        default:
            // Caractere não reconhecido (Erro Léxico)
            fprintf(stderr, "ERRO LEXICO (Linha %d): Caractere inválido: '%c' (ASCII %d).\n", atomo.linha, c, (int)c);
            atomo.tipo = TK_ERRO_LEXICO;
            return atomo;
    }
}
