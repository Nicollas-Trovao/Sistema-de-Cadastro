/*
 * janela.h
 * Declara o callback de ativacao que cria e apresenta a janela principal.
 */

#ifndef JANELA_H
#define JANELA_H

#include <gtk/gtk.h>

typedef struct AppState AppState;

void janela_activate(GtkApplication *app, gpointer user_data);
void app_state_iniciar_carregamento(AppState *state, const char *mensagem);
void app_state_finalizar_carregamento(AppState *state);

#endif
