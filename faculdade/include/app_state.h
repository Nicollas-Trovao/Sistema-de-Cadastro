/*
 * app_state.h
 * Define as estruturas compartilhadas pela interface.
 * AppState guarda os widgets principais e FormData guarda os campos do formulario.
 */

#ifndef APP_STATE_H
#define APP_STATE_H

#include <gtk/gtk.h>

#include "pessoa.h"

#define FORM_DOENCAS_TOTAL 6

typedef struct ItemLista ItemLista;
typedef struct AppState AppState;
typedef struct FormData FormData;

// Item exibido na lista lateral.
struct ItemLista
{
    GtkWidget *button;
    Pessoa *pessoa;
    AppState *state;
};

// Estado compartilhado pela janela principal.
struct AppState
{
    GList *items;
    ItemLista *selected_item;
    int carregamentos_pendentes;
    GtkWidget *header;
    GtkWidget *main_content;
    GtkWidget *loading_label;
    guint data_timer_id;
    GtkWidget *list;
    GtkWidget *button_modificar;
    GtkWidget *button_excluir;
    FormData *form_adicionar;
    GtkWidget *detail_nome;
    GtkWidget *detail_apelido;
    GtkWidget *detail_serie;
    GtkWidget *detail_idade;
    GtkWidget *detail_nacionalidade;
    GtkWidget *detail_genero;
    GtkWidget *detail_altura;
    GtkWidget *detail_peso;
    GtkWidget *detail_data_nascimento;
    GtkWidget *detail_imagem;
    GtkWidget *detail_crime;
    GtkWidget *detail_pena;
    GtkWidget *detail_cela;
    GtkWidget *detail_perigo;
    GtkWidget *detail_comportamento;
    GtkWidget *detail_regime;
    GtkWidget *detail_data_prisao;
    GtkWidget *detail_tempo_restante;
    GtkWidget *detail_isolamento;
    GtkWidget *detail_risco_fuga;
    GtkWidget *detail_saude;
    GtkWidget *detail_doencas;
    GtkWidget *detail_psicologico;
    GtkWidget *detail_medicacao;
    GtkWidget *detail_tentativas_fuga;
    GtkWidget *detail_historico_violencia;
    GtkWidget *detail_ultima_ocorrencia;
    GtkWidget *detail_alerta;
};

// Dados compartilhados pelo formulario de criacao/edicao.
struct FormData
{
    GtkWidget *window;

    // ------------------------------------------------------------
    // DADOS PESSOAIS
    // ------------------------------------------------------------
    GtkWidget *entry_nome;
    GtkWidget *entry_apelido;

    GtkWidget *label_serie;

    GtkWidget *entry_idade;
    GtkWidget *entry_nacionalidade;
    GtkWidget *entry_genero;

    GtkWidget *entry_altura;
    GtkWidget *entry_peso;

    GtkWidget *entry_data_nascimento;
    GtkWidget *entry_imagem;
    GtkWidget *preview_imagem;

    // ------------------------------------------------------------
    // CRIME
    // ------------------------------------------------------------
    GtkWidget *entry_crime;
    GtkWidget *entry_pena;

    // ------------------------------------------------------------
    // SISTEMA PRISIONAL
    // ------------------------------------------------------------
    GtkWidget *entry_cela;
    GtkWidget *entry_regime;

    GtkWidget *entry_perigo;
    GtkWidget *entry_comportamento;

    GtkWidget *entry_data_prisao;
    GtkWidget *entry_tempo_restante;

    GtkWidget *check_isolamento;
    GtkWidget *check_risco_fuga;

    // ------------------------------------------------------------
    // SAUDE
    // ------------------------------------------------------------
    GtkWidget *entry_saude;
    GtkWidget *check_doencas[FORM_DOENCAS_TOTAL];

    GtkWidget *entry_psicologico;
    GtkWidget *entry_medicacao;

    // ------------------------------------------------------------
    // SEGURANCA
    // ------------------------------------------------------------
    GtkWidget *entry_tentativas_fuga;

    GtkWidget *entry_historico_violencia;
    GtkWidget *entry_ultima_ocorrencia;

    GtkWidget *entry_alerta;

    // ------------------------------------------------------------
    // CONTROLE
    // ------------------------------------------------------------
    int serie_temp;
    GtkWidget *list;
    AppState *state;
    ItemLista *item_editando;
    gboolean modo_edicao;
};

#endif
