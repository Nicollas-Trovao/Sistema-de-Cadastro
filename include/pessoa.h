/*
 * pessoa.h
 * Define a estrutura Pessoa, que representa todos os dados de uma detenta.
 */

#ifndef PESSOA_H
#define PESSOA_H

#include <gtk/gtk.h>

typedef struct
{
    char nome[50];
    char apelido[50];

    int serie;
    int idade;

    char nacionalidade[30];
    char genero[20];

    float altura;
    float peso;

    char data_nascimento[20];
    char imagem[500];

    char crime[500];
    int pena_anos;

    char cela[20];
    char perigo[20];
    char comportamento[50];

    char regime[30];
    char data_prisao[20];

    int tempo_restante;

    gboolean isolamento;
    gboolean risco_fuga;

    char saude[50];
    char doencas[200];
    char psicologico[50];
    char medicacao[100];

    int tentativas_fuga;

    char historico_violencia[100];
    char ultima_ocorrencia[100];
    char alerta[100];

} Pessoa;

#endif
