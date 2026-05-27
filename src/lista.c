/*
 * lista.c
 * Controla a lista lateral de detentas.
 * Trata busca, selecao visual, exibicao de detalhes e exclusao de registros.
 */

#include "lista.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "arquivo.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static void limpar_detalhes(AppState *state)
{
    gtk_label_set_text(GTK_LABEL(state->detail_nome), "Selecione uma Detenta");
    gtk_label_set_text(GTK_LABEL(state->detail_apelido), "");
    gtk_label_set_text(GTK_LABEL(state->detail_serie), "");
    gtk_label_set_text(GTK_LABEL(state->detail_idade), "");
    gtk_label_set_text(GTK_LABEL(state->detail_nacionalidade), "");
    gtk_label_set_text(GTK_LABEL(state->detail_genero), "");
    gtk_label_set_text(GTK_LABEL(state->detail_altura), "");
    gtk_label_set_text(GTK_LABEL(state->detail_peso), "");
    gtk_label_set_text(GTK_LABEL(state->detail_data_nascimento), "");
    gtk_image_set_from_icon_name(GTK_IMAGE(state->detail_imagem), "image-x-generic-symbolic");
    gtk_image_set_pixel_size(GTK_IMAGE(state->detail_imagem), 220);
    gtk_label_set_text(GTK_LABEL(state->detail_crime), "");
    gtk_label_set_text(GTK_LABEL(state->detail_pena), "");
    gtk_label_set_text(GTK_LABEL(state->detail_cela), "");
    gtk_label_set_text(GTK_LABEL(state->detail_perigo), "");
    gtk_label_set_text(GTK_LABEL(state->detail_comportamento), "");
    gtk_label_set_text(GTK_LABEL(state->detail_regime), "");
    gtk_label_set_text(GTK_LABEL(state->detail_data_prisao), "");
    gtk_label_set_text(GTK_LABEL(state->detail_tempo_restante), "");
    gtk_label_set_text(GTK_LABEL(state->detail_isolamento), "");
    gtk_label_set_text(GTK_LABEL(state->detail_risco_fuga), "");
    gtk_label_set_text(GTK_LABEL(state->detail_saude), "");
    gtk_label_set_text(GTK_LABEL(state->detail_doencas), "");
    gtk_label_set_text(GTK_LABEL(state->detail_psicologico), "");
    gtk_label_set_text(GTK_LABEL(state->detail_medicacao), "");
    gtk_label_set_text(GTK_LABEL(state->detail_tentativas_fuga), "");
    gtk_label_set_text(GTK_LABEL(state->detail_historico_violencia), "");
    gtk_label_set_text(GTK_LABEL(state->detail_ultima_ocorrencia), "");
    gtk_label_set_text(GTK_LABEL(state->detail_alerta), "");
}

static void atualizar_selecao_visual(AppState *state, ItemLista *novo_item)
{
    if (state->selected_item && state->selected_item->button)
    {
        gtk_widget_remove_css_class(
            state->selected_item->button,
            "suggested-action");

        gtk_widget_remove_css_class(
            state->selected_item->button,
            "selected-detento");
    }

    if (novo_item && novo_item->button)
    {
        gtk_widget_add_css_class(
            novo_item->button,
            "suggested-action");

        gtk_widget_add_css_class(
            novo_item->button,
            "selected-detento");
    }
}

static int comparar_texto(const char *a, const char *b)
{
    char *a_normalizado =
        g_utf8_casefold(a ? a : "", -1);

    char *b_normalizado =
        g_utf8_casefold(b ? b : "", -1);

    int resultado =
        g_utf8_collate(a_normalizado, b_normalizado);

    g_free(a_normalizado);
    g_free(b_normalizado);

    return resultado;
}

static int obter_posicao_ordenada(AppState *state, ItemLista *novo_item)
{
    int posicao = 0;

    for (GList *l = state->items; l != NULL; l = l->next)
    {
        ItemLista *item = l->data;

        if (comparar(novo_item, item) < 0)
            return posicao;

        posicao++;
    }

    return posicao;
}

static void atualizar_label_formatado(GtkWidget *label, const char *formato, ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, formato);
    vsnprintf(buffer, sizeof(buffer), formato, args);
    va_end(args);

    gtk_label_set_text(GTK_LABEL(label), buffer);
}

void atualizar_item_lista(ItemLista *item)
{
    char texto[120];

    if (!item || !item->button || !item->pessoa)
        return;

    snprintf(
        texto,
        sizeof(texto),
        "%s | Série: %d",
        item->pessoa->nome,
        item->pessoa->serie);

    gtk_button_set_label(GTK_BUTTON(item->button), texto);
}

ItemLista *criar_item_lista(AppState *state, Pessoa *pessoa)
{
    ItemLista *item = g_malloc0(sizeof(ItemLista));

    item->pessoa = pessoa;
    item->state = state;

    char texto[120];
    snprintf(texto, sizeof(texto), "%s | Série: %d", pessoa->nome, pessoa->serie);

    GtkWidget *button =
        gtk_button_new_with_label(texto);

    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_halign(button, GTK_ALIGN_FILL);
    gtk_widget_set_size_request(button, -1, 50);
    gtk_widget_set_margin_top(button, 5);
    gtk_widget_set_margin_bottom(button, 5);
    gtk_widget_add_css_class(button, "flat");

    GtkWidget *child =
        gtk_widget_get_first_child(button);

    if (GTK_IS_LABEL(child))
    {
        gtk_label_set_xalign(GTK_LABEL(child), 0.0f);
        gtk_widget_set_margin_start(child, 10);
    }

    item->button = button;

    gtk_list_box_insert(
        GTK_LIST_BOX(state->list),
        button,
        obter_posicao_ordenada(state, item));

    g_signal_connect(button, "clicked", G_CALLBACK(item_clicado), item);

    return item;
}

void liberar_item_lista(gpointer data)
{
    ItemLista *item = data;

    if (!item)
        return;

    g_free(item->pessoa);
    g_free(item);
}

static void excluir_detenta_confirmada(AppState *state)
{
    if (!state->selected_item)
        return;

    ItemLista *item = state->selected_item;

    GtkWidget *row =
        gtk_widget_get_ancestor(
            item->button,
            GTK_TYPE_LIST_BOX_ROW);

    if (!row)
        return;

    state->selected_item = NULL;

    atualizar_selecao_visual(state, NULL);

    gtk_list_box_remove(GTK_LIST_BOX(state->list), row);

    state->items = g_list_remove(state->items, item);

    limpar_detalhes(state);

    gtk_widget_set_sensitive(state->button_excluir, FALSE);
    gtk_widget_set_sensitive(state->button_modificar, FALSE);

    liberar_item_lista(item);

    salvar_arquivo(state->items);
}

static void on_excluir_response(GtkDialog *dialog, int response_id, gpointer user_data)
{
    AppState *state = user_data;

    if (response_id == GTK_RESPONSE_OK)
    {
        excluir_detenta_confirmada(state);
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}

void excluir_detenta(GtkWidget *widget, gpointer data)
{
    AppState *state = data;

    if (!state->selected_item)
        return;

    GtkWidget *dialog =
        gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_root(widget)),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "Tem certeza que deseja excluir esta detenta?");

    gtk_dialog_add_buttons(
        GTK_DIALOG(dialog),
        "Cancelar", GTK_RESPONSE_CANCEL,
        "Excluir", GTK_RESPONSE_OK,
        NULL);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(on_excluir_response),
                     state);

    gtk_widget_show(dialog);
}

void pesquisar(GtkEditable *editable, gpointer data)
{
    AppState *state = data;

    const char *texto =
        gtk_editable_get_text(editable);

    if (!texto || texto[0] == '\0')
    {
        for (GList *l = state->items; l != NULL; l = l->next)
        {
            ItemLista *item = l->data;

            if (!item || !item->button)
                continue;

            GtkWidget *row =
                gtk_widget_get_parent(item->button);

            gtk_widget_set_visible(row, TRUE);
        }
        return;
    }

    char *busca = g_utf8_casefold(texto, -1);

    for (GList *l = state->items; l != NULL; l = l->next)
    {
        ItemLista *item = l->data;

        if (!item || !item->button || !item->pessoa)
            continue;

        char serie_str[50];
        snprintf(serie_str, sizeof(serie_str), "%d", item->pessoa->serie);

        char *nome = g_utf8_casefold(item->pessoa->nome, -1);
        char *apelido = g_utf8_casefold(item->pessoa->apelido, -1);
        char *crime = g_utf8_casefold(item->pessoa->crime, -1);
        char *cela = g_utf8_casefold(item->pessoa->cela, -1);

        gboolean encontrado =
            (g_strrstr(serie_str, busca) != NULL) ||
            (g_strrstr(nome, busca) != NULL) ||
            (g_strrstr(apelido, busca) != NULL) ||
            (g_strrstr(crime, busca) != NULL) ||
            (g_strrstr(cela, busca) != NULL);

        g_free(nome);
        g_free(apelido);
        g_free(crime);
        g_free(cela);

        GtkWidget *row =
            gtk_widget_get_parent(item->button);

        gtk_widget_set_visible(row, encontrado);
    }

    g_free(busca);
}

int comparar(const void *a, const void *b)
{
    ItemLista *i1 = (ItemLista *)a;
    ItemLista *i2 = (ItemLista *)b;

    return comparar_texto(
        i1 && i1->pessoa ? i1->pessoa->nome : "",
        i2 && i2->pessoa ? i2->pessoa->nome : "");
}

void sincronizar_ordem_lista(AppState *state)
{
    if (!state || !state->list)
        return;

    int posicao = 0;

    for (GList *l = state->items; l != NULL; l = l->next)
    {
        ItemLista *item = l->data;

        if (!item || !item->button)
            continue;

        GtkWidget *row =
            gtk_widget_get_ancestor(
                item->button,
                GTK_TYPE_LIST_BOX_ROW);

        if (!row)
            continue;

        g_object_ref(item->button);

        gtk_list_box_remove(
            GTK_LIST_BOX(state->list),
            row);

        gtk_list_box_insert(
            GTK_LIST_BOX(state->list),
            item->button,
            posicao);

        g_object_unref(item->button);

        posicao++;
    }
}

void item_clicado(GtkWidget *widget, gpointer data)
{
    (void)widget;

    ItemLista *item =
        (ItemLista *)data;

    AppState *state = item->state;

    Pessoa *p =
        item->pessoa;

    atualizar_label_formatado(state->detail_nome, "Nome: %s", p->nome);
    atualizar_label_formatado(state->detail_apelido, "Apelido: %s", p->apelido);
    atualizar_label_formatado(state->detail_serie, "Série: %d", p->serie);
    atualizar_label_formatado(state->detail_idade, "Idade: %d", p->idade);
    atualizar_label_formatado(state->detail_nacionalidade, "Nacionalidade: %s", p->nacionalidade);
    atualizar_label_formatado(state->detail_genero, "Gênero: %s", p->genero);
    atualizar_label_formatado(state->detail_altura, "Altura: %.2f", p->altura);
    atualizar_label_formatado(state->detail_peso, "Peso: %.2f", p->peso);
    atualizar_label_formatado(state->detail_data_nascimento, "Nascimento: %s", p->data_nascimento);

    if (p->imagem[0] != '\0' && strcmp(p->imagem, "-") != 0 && g_file_test(p->imagem, G_FILE_TEST_EXISTS))
        gtk_image_set_from_file(GTK_IMAGE(state->detail_imagem), p->imagem);
    else
        gtk_image_set_from_icon_name(GTK_IMAGE(state->detail_imagem), "image-x-generic-symbolic");

    gtk_image_set_pixel_size(GTK_IMAGE(state->detail_imagem), 220);

    atualizar_label_formatado(state->detail_crime, "Crime: %s", p->crime);
    atualizar_label_formatado(state->detail_pena, "Pena: %d anos", p->pena_anos);
    atualizar_label_formatado(state->detail_cela, "Cela: %s", p->cela);
    atualizar_label_formatado(state->detail_perigo, "Periculosidade: %s", p->perigo);
    atualizar_label_formatado(state->detail_comportamento, "Comportamento: %s", p->comportamento);
    atualizar_label_formatado(state->detail_regime, "Regime: %s", p->regime);
    atualizar_label_formatado(state->detail_data_prisao, "Data da Prisão: %s", p->data_prisao);
    atualizar_label_formatado(state->detail_tempo_restante, "Tempo Restante: %d anos", p->tempo_restante);
    atualizar_label_formatado(state->detail_isolamento, "Isolamento: %s", p->isolamento ? "SIM" : "NÃO");
    atualizar_label_formatado(state->detail_risco_fuga, "Risco de Fuga: %s", p->risco_fuga ? "SIM" : "NÃO");
    atualizar_label_formatado(state->detail_saude, "Saúde: %s", p->saude);
    atualizar_label_formatado(state->detail_doencas, "Doenças: %s", p->doencas);
    atualizar_label_formatado(state->detail_psicologico, "Psicológico: %s", p->psicologico);
    atualizar_label_formatado(state->detail_medicacao, "Medicação: %s", p->medicacao);
    atualizar_label_formatado(state->detail_tentativas_fuga, "Tentativas de Fuga: %d", p->tentativas_fuga);
    atualizar_label_formatado(state->detail_historico_violencia, "Histórico Violento: %s", p->historico_violencia);
    atualizar_label_formatado(state->detail_ultima_ocorrencia, "Última Ocorrência: %s", p->ultima_ocorrencia);
    atualizar_label_formatado(state->detail_alerta, "Alerta: %s", p->alerta);

    gtk_widget_set_sensitive(state->button_excluir, TRUE);
    gtk_widget_set_sensitive(state->button_modificar, TRUE);

    atualizar_selecao_visual(state, item);

    state->selected_item = item;
}
