#ifndef ANALEX_H
#define ANALEX_H

// --- 1. DEFINIÇÃO DOS TIPOS DE ÁTOMOS (Tokens) ---
typedef enum {
    // Palavras Chave (Baseadas na especificação da LPD)
    TK_PALAVRA_AND, TK_PALAVRA_BEGIN, TK_PALAVRA_CHAR, TK_PALAVRA_ELSE,
    TK_PALAVRA_END, TK_PALAVRA_FLOAT, TK_PALAVRA_FOR, TK_PALAVRA_IF,
    TK_PALAVRA_INT, TK_PALAVRA_NOT, TK_PALAVRA_OR, TK_PALAVRA_PRG,
    TK_PALAVRA_READ, TK_PALAVRA_REPEAT, TK_PALAVRA_RETURN, TK_PALAVRA_SUBROT,
    TK_PALAVRA_THEN, TK_PALAVRA_UNTIL, TK_PALAVRA_VAR, TK_PALAVRA_VOID,
    TK_PALAVRA_WHILE, TK_PALAVRA_WRITE,

    // Identificadores e Constantes
    TK_IDENTIFICADOR,
    TK_CONST_INTEIRA,
    TK_CONST_REAL,
    TK_CONST_STRING,
    TK_CONST_CARACTERE,

    // Operadores Aritméticos e Relacionais
    TK_OP_ATRIBUICAO, // <-
    TK_OP_IGUALDADE,  // ==
    TK_OP_DIFERENCA,  // !=
    TK_OP_MAIOR_IGUAL, // >=
    TK_OP_MENOR_IGUAL, // <=
    TK_OP_SOMA, TK_OP_SUBTRACAO, TK_OP_MULTIPLICACAO, TK_OP_DIVISAO,
    TK_OP_MAIOR, TK_OP_MENOR,

    // Delimitadores e Pontuação
    TK_DELIMITADOR_DOISPONTOS,    // :
    TK_DELIMITADOR_PONTO_VIRGULA, // ;
    TK_DELIMITADOR_VIRGULA,       // ,
    TK_DELIMITADOR_ABRE_PARENTESES, // (
    TK_DELIMITADOR_FECHA_PARENTESES, // )
    TK_DELIMITADOR_ABRE_COLCHETES, // [
    TK_DELIMITADOR_FECHA_COLCHETES, // ]
    TK_DELIMITADOR_PONTO,         // .

    // Especiais
    TK_EOF,
    TK_ERRO_LEXICO
} TIPO_ATOMO;

// --- 2. DEFINIÇÃO DA UNION E ESTRUTURA DE RETORNO ---
#define TAM_MAX_LEXEMA_STR 256

typedef union {
    char str_val[TAM_MAX_LEXEMA_STR]; // Para Identificadores, Palavras-chave, Strings
    int int_val;                      // Para constantes inteiras
    double real_val;                  // Para constantes reais
} VALOR_LEXEMA;

typedef struct {
    TIPO_ATOMO tipo;
    VALOR_LEXEMA lexema;
    int linha;
} TInfoAtomo;

// --- 3. INTERFACE PÚBLICA ---
void inicializar_analex(const char *nome_arquivo_fonte);
TInfoAtomo obter_atomo(void);
void fechar_analex(void);

#endif // ANALEX_H
