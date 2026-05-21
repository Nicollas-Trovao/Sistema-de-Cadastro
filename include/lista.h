/*
 * lista.h
 * Declara as funcoes publicas da lista, busca, selecao e ordenacao.
 */

#ifndef LISTA_H
#define LISTA_H

#include <gtk/gtk.h>

#include "app_state.h"

void excluir_detenta(GtkWidget *widget, gpointer data);
void pesquisar(GtkEditable *editable, gpointer data);
void item_clicado(GtkWidget *widget, gpointer data);
ItemLista *criar_item_lista(AppState *state, Pessoa *pessoa);
void atualizar_item_lista(ItemLista *item);
void liberar_item_lista(gpointer data);

int comparar(const void *a, const void *b);
int ordenar_listbox(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer data);

#endif
