/*
 * arquivo.h
 * Declara as funcoes de carregamento e salvamento do arquivo CSV.
 */

#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <gtk/gtk.h>

#include "app_state.h"

void carregar_arquivo(AppState *state);
void salvar_arquivo(GList *items);

#endif
