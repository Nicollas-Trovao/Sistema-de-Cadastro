/*
 * Sistema simples de cadastro de detentas com GTK.
 * O programa mantém os dados em memória e sincroniza os registros
 * com o arquivo detentos.csv. Use "make" para compilar o projeto.
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

#include "janela.h"

// ------------------------------------------------------------
// FUNÇÃO PRINCIPAL
// ------------------------------------------------------------
int main(int argc, char **argv)
{
    GtkApplication *app;

    int status;

    srand(time(NULL));

    app = gtk_application_new(
        "com.exemplo.app",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(janela_activate),
        NULL);

    status = g_application_run(
        G_APPLICATION(app),
        argc,
        argv);

    g_object_unref(app);

    return status;
}
// ------------------------------------------------------------

