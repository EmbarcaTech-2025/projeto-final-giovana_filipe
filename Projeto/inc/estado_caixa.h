#ifndef ESTADO_CAIXA_H
#define ESTADO_CAIXA_H

typedef enum {
    CAIXA_ABERTA,
    CAIXA_FECHADA,
    CAIXA_FECHADA_TRAVADA
} EstadoCaixa;

extern EstadoCaixa estado_caixa;

#endif // ESTADO_CAIXA_H
