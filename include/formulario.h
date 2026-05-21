/*
 * formulario.h
 * Declara os callbacks publicos para abrir o formulario de criacao/edicao.
 */

#ifndef FORMULARIO_H
#define FORMULARIO_H

#include <gtk/gtk.h>

typedef struct AppState AppState;

void abrir_janela_novo(GtkWidget *widget, gpointer data);
void abrir_janela_modificar(GtkWidget *widget, gpointer data);
void preparar_janela_novo(AppState *state);

#endif
