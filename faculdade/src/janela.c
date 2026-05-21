/*
 * janela.c
 * Monta a janela principal da aplicacao GTK.
 * Cria o cabecalho, busca, sidebar, painel de detalhes e callbacks gerais.
 */

#include "janela.h"

#include <time.h>

#include "app_state.h"
#include "arquivo.h"
#include "formulario.h"
#include "lista.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static GtkWidget *create_main_window(GtkApplication *app);
static void carregar_css(void);
static void liberar_app_state(gpointer data);
static void fechar_janela(GtkWidget *widget, gpointer data);
static void on_dialog_response(GtkDialog *dialog, int response_id, gpointer user_data);
static gboolean atualizar_data(gpointer data);
static GtkWidget *criar_label_detalhe(const char *texto);
static void atualizar_estado_carregamento(AppState *state);
static void executar_pre_carregamentos(AppState *state);

// ------------------------------------------------------------
// FUNÇÕES
// ------------------------------------------------------------

// ------------------------------------------------------------
// FUNCOES PRINCIPAIS
// ------------------------------------------------------------
GtkWidget *create_main_window(GtkApplication *app)
{
    GtkWidget *window;
    AppState *state = g_malloc0(sizeof(AppState));

    window =
        gtk_application_window_new(app);

    g_object_set_data_full(
        G_OBJECT(window),
        "app-state",
        state,
        liberar_app_state);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Penitenciaria Santa Olga");

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        1400,
        820);

    gtk_window_maximize(
        GTK_WINDOW(window));

    gtk_window_set_resizable(
        GTK_WINDOW(window),
        TRUE);

    gtk_widget_set_size_request(
        window,
        1200,
        700);

    // ------------------------------------------------------------
    // CONTAINER PRINCIPAL
    // ------------------------------------------------------------

    GtkWidget *main_box =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            0);

    gtk_widget_add_css_class(
        main_box,
        "app-root");

    gtk_widget_set_margin_top(main_box, 18);
    gtk_widget_set_margin_bottom(main_box, 18);
    gtk_widget_set_margin_start(main_box, 18);
    gtk_widget_set_margin_end(main_box, 18);

    gtk_window_set_child(
        GTK_WINDOW(window),
        main_box);

    GtkWidget *list =
        gtk_list_box_new();

    state->list = list;

    gtk_list_box_set_sort_func(
        GTK_LIST_BOX(list),
        ordenar_listbox,
        NULL,
        NULL);

    gtk_list_box_set_selection_mode(
        GTK_LIST_BOX(list),
        GTK_SELECTION_NONE);

    gtk_widget_set_vexpand(
        list,
        TRUE);

    // ------------------------------------------------------------
    // HEADER
    // ------------------------------------------------------------

    GtkWidget *header =
        gtk_box_new(
            GTK_ORIENTATION_HORIZONTAL,
            15);

    state->header = header;

    gtk_widget_add_css_class(
        header,
        "toolbar");

    gtk_widget_add_css_class(
        header,
        "primary-toolbar");

    gtk_widget_set_margin_top(header, 0);
    gtk_widget_set_margin_bottom(header, 14);
    gtk_widget_set_margin_start(header, 0);
    gtk_widget_set_margin_end(header, 0);

    gtk_box_append(
        GTK_BOX(main_box),
        header);

    state->loading_label =
        gtk_label_new("Carregando...");

    gtk_widget_add_css_class(
        state->loading_label,
        "caption");

    gtk_widget_set_visible(
        state->loading_label,
        FALSE);

    gtk_box_append(
        GTK_BOX(main_box),
        state->loading_label);

    // ------------------------------------------------------------
    // BUSCA
    // ------------------------------------------------------------

    GtkWidget *search =
        gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(search),
        "Pesquisar detenta...");

    gtk_widget_add_css_class(
        search,
        "pill");

    gtk_widget_add_css_class(
        search,
        "search-entry");

    gtk_widget_set_size_request(
        search,
        -1,
        45);

    gtk_widget_set_hexpand(
        search,
        TRUE);

    gtk_box_append(
        GTK_BOX(header),
        search);

    // ------------------------------------------------------------
    // BOTAO ADICIONAR
    // ------------------------------------------------------------

    GtkWidget *button_add =
        gtk_button_new_with_label(
            "Adicionar");

    gtk_widget_add_css_class(
        button_add,
        "suggested-action");

    gtk_widget_add_css_class(
        button_add,
        "toolbar-button");

    gtk_widget_set_size_request(
        button_add,
        130,
        45);

    g_signal_connect(
        button_add,
        "clicked",
        G_CALLBACK(abrir_janela_novo),
        state);

    gtk_box_append(
        GTK_BOX(header),
        button_add);

    // ------------------------------------------------------------
    // BOTAO MODIFICAR
    // ------------------------------------------------------------

    GtkWidget *button_modificar =
        gtk_button_new_with_label(
            "Modificar");

    gtk_widget_add_css_class(
        button_modificar,
        "toolbar-button");

    gtk_widget_set_size_request(
        button_modificar,
        130,
        45);

    gtk_box_append(
        GTK_BOX(header),
        button_modificar);

    g_signal_connect(
        button_modificar,
        "clicked",
        G_CALLBACK(abrir_janela_modificar),
        state);

    state->button_modificar =
        button_modificar;

    gtk_widget_set_sensitive(
        button_modificar,
        FALSE);

    // ------------------------------------------------------------
    // BOTAO EXCLUIR
    // ------------------------------------------------------------

    GtkWidget *button_excluir =
        gtk_button_new_with_label(
            "Excluir");

    gtk_widget_add_css_class(
        button_excluir,
        "destructive-action");

    gtk_widget_add_css_class(
        button_excluir,
        "toolbar-button");

    state->button_excluir =
        button_excluir;

    gtk_widget_set_sensitive(
        button_excluir,
        FALSE);

    gtk_widget_set_size_request(
        button_excluir,
        130,
        45);

    gtk_box_append(
        GTK_BOX(header),
        button_excluir);

    // ------------------------------------------------------------
    // RELÓGIO
    // ------------------------------------------------------------
    GtkWidget *data_label = gtk_label_new("");

    state->data_timer_id =
        g_timeout_add_seconds(1, atualizar_data, data_label);

    gtk_widget_add_css_class(data_label, "caption");
    gtk_widget_add_css_class(data_label, "date-chip");
    gtk_box_append(GTK_BOX(header), data_label);

    // ------------------------------------------------------------
    // BOTAO SAIR
    // ------------------------------------------------------------

    GtkWidget *button_exit =
        gtk_button_new_with_label(
            "Sair");

    gtk_widget_add_css_class(
        button_exit,
        "toolbar-button");

    gtk_widget_set_size_request(
        button_exit,
        130,
        45);

    gtk_box_append(
        GTK_BOX(header),
        button_exit);

    g_signal_connect(
        button_exit,
        "clicked",
        G_CALLBACK(fechar_janela),
        window);

    // ------------------------------------------------------------
    // SEPARADOR
    // ------------------------------------------------------------

    GtkWidget *separator =
        gtk_separator_new(
            GTK_ORIENTATION_HORIZONTAL);

    gtk_box_append(
        GTK_BOX(main_box),
        separator);

    // ------------------------------------------------------------
    // AREA PRINCIPAL
    // ------------------------------------------------------------

    GtkWidget *main_content =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 18);

    state->main_content = main_content;

    gtk_widget_set_vexpand(main_content, TRUE);

    gtk_widget_set_margin_top(main_content, 18);

    gtk_box_append(GTK_BOX(main_box), main_content);

    // ------------------------------------------------------------
    // SIDEBAR
    // ------------------------------------------------------------
    GtkWidget *sidebar_content =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    gtk_widget_set_margin_top(sidebar_content, 16);
    gtk_widget_set_margin_bottom(sidebar_content, 16);
    gtk_widget_set_margin_start(sidebar_content, 16);
    gtk_widget_set_margin_end(sidebar_content, 16);

    // TITULO
    GtkWidget *titulo_lista =
        gtk_label_new("Lista de Detentas");

    gtk_widget_add_css_class(titulo_lista, "title-3");
    gtk_label_set_xalign(GTK_LABEL(titulo_lista), 0);

    gtk_box_append(GTK_BOX(sidebar_content), titulo_lista);

    // separador
    GtkWidget *sep =
        gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    gtk_box_append(GTK_BOX(sidebar_content), sep);

    // SCROLL DA LISTA
    GtkWidget *scroll =
        gtk_scrolled_window_new();

    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE);

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC);

    // Lista dentro do scroll
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list);

    gtk_box_append(GTK_BOX(sidebar_content), scroll);

    // FRAME DA SIDEBAR
    GtkWidget *sidebar_frame =
        gtk_frame_new(NULL);

    gtk_widget_add_css_class(
        sidebar_frame,
        "panel");

    gtk_widget_set_hexpand(sidebar_frame, FALSE);
    gtk_widget_set_size_request(sidebar_frame, 340, -1);

    gtk_frame_set_child(
        GTK_FRAME(sidebar_frame),
        sidebar_content);

    // Adiciona campos ao grid no layout principal
    gtk_box_append(
        GTK_BOX(main_content),
        sidebar_frame);

    // Conecta o botao de exclusao
    g_signal_connect(
        button_excluir,
        "clicked",
        G_CALLBACK(excluir_detenta),
        state);

    // ------------------------------------------------------------
    // PAINEL DIREITO
    // ------------------------------------------------------------

    GtkWidget *details =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            18);

    gtk_widget_set_valign(
        details,
        GTK_ALIGN_START);

    gtk_widget_set_vexpand(
        details,
        FALSE);

    gtk_widget_set_margin_top(
        details,
        22);

    gtk_widget_set_margin_bottom(
        details,
        22);

    gtk_widget_set_margin_start(
        details,
        22);

    gtk_widget_set_margin_end(
        details,
        22);

    GtkWidget *details_frame =
        gtk_frame_new(NULL);

    gtk_widget_add_css_class(
        details_frame,
        "panel");

    gtk_widget_set_hexpand(
        details_frame,
        TRUE);

    GtkWidget *details_scroll =
        gtk_scrolled_window_new();

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(details_scroll),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC);

    gtk_widget_set_vexpand(
        details_scroll,
        TRUE);

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(details_scroll),
        details);

    gtk_frame_set_child(
        GTK_FRAME(details_frame),
        details_scroll);

    gtk_box_append(
        GTK_BOX(main_content),
        details_frame);

    // ------------------------------------------------------------
    // TITULO DETALHES
    // ------------------------------------------------------------

    GtkWidget *titulo_detalhes =
        gtk_label_new(
            "Informações da Detenta");

    gtk_widget_add_css_class(
        titulo_detalhes,
        "title-2");

    gtk_label_set_xalign(
        GTK_LABEL(titulo_detalhes),
        0);

    gtk_box_append(
        GTK_BOX(details),
        titulo_detalhes);

    GtkWidget *topo_detalhes =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 24);

    gtk_widget_set_hexpand(
        topo_detalhes,
        TRUE);

    gtk_widget_set_valign(
        topo_detalhes,
        GTK_ALIGN_START);

    gtk_widget_set_vexpand(
        topo_detalhes,
        FALSE);

    gtk_box_append(
        GTK_BOX(details),
        topo_detalhes);

    state->detail_imagem =
        gtk_image_new_from_icon_name("image-x-generic-symbolic");

    gtk_widget_add_css_class(
        state->detail_imagem,
        "photo-preview");

    gtk_widget_add_css_class(
        state->detail_imagem,
        "detento-photo");

    gtk_image_set_pixel_size(
        GTK_IMAGE(state->detail_imagem),
        220);

    gtk_widget_set_size_request(
        state->detail_imagem,
        240,
        260);

    gtk_widget_set_halign(
        state->detail_imagem,
        GTK_ALIGN_START);

    gtk_widget_set_valign(
        state->detail_imagem,
        GTK_ALIGN_START);

    gtk_widget_set_vexpand(
        state->detail_imagem,
        FALSE);

    gtk_box_append(
        GTK_BOX(topo_detalhes),
        state->detail_imagem);

    GtkWidget *coluna_dados =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);

    gtk_widget_set_hexpand(
        coluna_dados,
        TRUE);

    gtk_widget_set_valign(
        coluna_dados,
        GTK_ALIGN_START);

    gtk_box_append(
        GTK_BOX(topo_detalhes),
        coluna_dados);

    // ------------------------------------------------------------
    // SECAO DADOS PESSOAIS
    // ------------------------------------------------------------

    GtkWidget *sec_pessoal =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_add_css_class(
        sec_pessoal,
        "detail-section");

    gtk_widget_set_hexpand(
        sec_pessoal,
        TRUE);

    GtkWidget *titulo_pessoal =
        gtk_label_new("Dados Pessoais");

    gtk_widget_add_css_class(
        titulo_pessoal,
        "title-4");

    gtk_label_set_xalign(
        GTK_LABEL(titulo_pessoal),
        0);

    gtk_box_append(
        GTK_BOX(sec_pessoal),
        titulo_pessoal);

    GtkWidget *grid_pessoal =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid_pessoal),
        8);

    gtk_grid_set_column_spacing(
        GTK_GRID(grid_pessoal),
        20);

    state->detail_nome = criar_label_detalhe("Nome: -");
    state->detail_apelido = criar_label_detalhe("Apelido: -");
    state->detail_serie = criar_label_detalhe("Série: -");
    state->detail_idade = criar_label_detalhe("Idade: -");
    state->detail_nacionalidade = criar_label_detalhe("Nacionalidade: -");
    state->detail_genero = criar_label_detalhe("Gênero: -");
    state->detail_altura = criar_label_detalhe("Altura: -");
    state->detail_peso = criar_label_detalhe("Peso: -");
    state->detail_data_nascimento = criar_label_detalhe("Nascimento: -");

    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_nome, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_apelido, 2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_serie, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_idade, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_data_nascimento, 2, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_genero, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_nacionalidade, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_altura, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_pessoal), state->detail_peso, 1, 3, 1, 1);

    gtk_box_append(
        GTK_BOX(sec_pessoal),
        grid_pessoal);

    gtk_box_append(
        GTK_BOX(coluna_dados),
        sec_pessoal);

    GtkWidget *grid_secoes =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid_secoes),
        22);

    gtk_grid_set_column_spacing(
        GTK_GRID(grid_secoes),
        70);

    gtk_widget_set_vexpand(
        grid_secoes,
        FALSE);

    gtk_widget_set_halign(
        grid_secoes,
        GTK_ALIGN_START);

    gtk_widget_set_valign(
        grid_secoes,
        GTK_ALIGN_START);

    gtk_box_append(
        GTK_BOX(coluna_dados),
        grid_secoes);

    // ------------------------------------------------------------
    // SECAO PRISIONAL
    // ------------------------------------------------------------

    GtkWidget *sec_prisional =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_add_css_class(
        sec_prisional,
        "detail-section");

    gtk_widget_set_hexpand(
        sec_prisional,
        TRUE);

    GtkWidget *titulo_prisional =
        gtk_label_new("Sistema Prisional");

    gtk_widget_add_css_class(
        titulo_prisional,
        "title-4");

    gtk_label_set_xalign(
        GTK_LABEL(titulo_prisional),
        0);

    gtk_box_append(
        GTK_BOX(sec_prisional),
        titulo_prisional);

    GtkWidget *grid_prisional =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid_prisional),
        8);

    gtk_grid_set_column_spacing(
        GTK_GRID(grid_prisional),
        20);

    state->detail_crime = criar_label_detalhe("Crime: -");
    state->detail_pena = criar_label_detalhe("Pena: -");
    state->detail_cela = criar_label_detalhe("Cela: -");
    state->detail_perigo = criar_label_detalhe("Periculosidade: -");
    state->detail_comportamento = criar_label_detalhe("Comportamento: -");
    state->detail_regime = criar_label_detalhe("Regime: -");
    state->detail_data_prisao = criar_label_detalhe("Data Prisão: -");
    state->detail_tempo_restante = criar_label_detalhe("Tempo Restante: -");
    state->detail_isolamento = criar_label_detalhe("Isolamento: -");
    state->detail_risco_fuga = criar_label_detalhe("Risco Fuga: -");

    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_crime, 0, 0, 3, 1);

    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_pena, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_cela, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_regime, 2, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_perigo, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_comportamento, 1, 2, 2, 1);

    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_data_prisao, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_tempo_restante, 1, 3, 1, 1);

    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_isolamento, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid_prisional), state->detail_risco_fuga, 1, 4, 1, 1);

    gtk_box_append(
        GTK_BOX(sec_prisional),
        grid_prisional);

    gtk_grid_attach(
        GTK_GRID(grid_secoes),
        sec_prisional,
        0,
        0,
        3,
        1);

    // ------------------------------------------------------------
    // SECAO SAUDE
    // ------------------------------------------------------------

    GtkWidget *sec_saude =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_add_css_class(
        sec_saude,
        "detail-section");

    gtk_widget_set_hexpand(
        sec_saude,
        TRUE);

    GtkWidget *titulo_saude =
        gtk_label_new("Saúde");

    gtk_widget_add_css_class(
        titulo_saude,
        "title-4");

    gtk_label_set_xalign(
        GTK_LABEL(titulo_saude),
        0);

    gtk_box_append(
        GTK_BOX(sec_saude),
        titulo_saude);

    state->detail_saude = criar_label_detalhe("Saúde: -");
    state->detail_doencas = criar_label_detalhe("Doenças: -");
    state->detail_psicologico = criar_label_detalhe("Psicológico: -");
    state->detail_medicacao = criar_label_detalhe("Medicação: -");

    gtk_box_append(GTK_BOX(sec_saude), state->detail_saude);
    gtk_box_append(GTK_BOX(sec_saude), state->detail_doencas);
    gtk_box_append(GTK_BOX(sec_saude), state->detail_psicologico);
    gtk_box_append(GTK_BOX(sec_saude), state->detail_medicacao);

    gtk_grid_attach(
        GTK_GRID(grid_secoes),
        sec_saude,
        0,
        1,
        1,
        1);

    // ------------------------------------------------------------
    // SECAO SEGURANCA
    // ------------------------------------------------------------

    GtkWidget *sec_seg =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_add_css_class(
        sec_seg,
        "detail-section");

    gtk_widget_set_hexpand(
        sec_seg,
        TRUE);

    GtkWidget *titulo_seg =
        gtk_label_new("Segurança");

    gtk_widget_add_css_class(
        titulo_seg,
        "title-4");

    gtk_label_set_xalign(
        GTK_LABEL(titulo_seg),
        0);

    gtk_box_append(
        GTK_BOX(sec_seg),
        titulo_seg);

    state->detail_tentativas_fuga = criar_label_detalhe("Tentativas de fuga: -");
    state->detail_historico_violencia = criar_label_detalhe("Histórico Violência: -");
    state->detail_ultima_ocorrencia = criar_label_detalhe("Última Ocorrência: -");
    state->detail_alerta = criar_label_detalhe("Alerta: -");

    gtk_box_append(
        GTK_BOX(sec_seg),
        state->detail_tentativas_fuga);

    gtk_box_append(
        GTK_BOX(sec_seg),
        state->detail_historico_violencia);

    gtk_box_append(
        GTK_BOX(sec_seg),
        state->detail_ultima_ocorrencia);

    gtk_box_append(
        GTK_BOX(sec_seg),
        state->detail_alerta);

    gtk_grid_attach(
        GTK_GRID(grid_secoes),
        sec_seg,
        1,
        1,
        1,
        1);

    // ------------------------------------------------------------
    // CARREGA DADOS
    // ------------------------------------------------------------

    executar_pre_carregamentos(state);

    // ------------------------------------------------------------
    // CONECTA PESQUISA
    // ------------------------------------------------------------

    g_signal_connect(
        search,
        "changed",
        G_CALLBACK(pesquisar),
        state);

    return window;
}


static gboolean atualizar_data(gpointer data)
{
    GtkLabel *label = GTK_LABEL(data);

    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", t);

    gtk_label_set_text(label, buffer);

    return G_SOURCE_CONTINUE; // Mantém o temporizador ativo
}

// ------------------------------------------------------------
// ACTIVATE
// ------------------------------------------------------------
void janela_activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    carregar_css();

    GtkWidget *window =
        create_main_window(app);

    gtk_window_present(
        GTK_WINDOW(window));
}

static GtkWidget *criar_label_detalhe(const char *texto)
{
    GtkWidget *label = gtk_label_new(texto);

    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
    gtk_widget_add_css_class(label, "detail-label");

    return label;
}

void app_state_iniciar_carregamento(AppState *state, const char *mensagem)
{
    if (!state)
        return;

    state->carregamentos_pendentes++;

    if (mensagem && state->loading_label)
        gtk_label_set_text(GTK_LABEL(state->loading_label), mensagem);

    atualizar_estado_carregamento(state);
}

void app_state_finalizar_carregamento(AppState *state)
{
    if (!state)
        return;

    if (state->carregamentos_pendentes > 0)
        state->carregamentos_pendentes--;

    atualizar_estado_carregamento(state);
}

static void atualizar_estado_carregamento(AppState *state)
{
    gboolean carregando =
        state && state->carregamentos_pendentes > 0;

    if (!state)
        return;

    if (state->header)
        gtk_widget_set_sensitive(state->header, !carregando);

    if (state->main_content)
        gtk_widget_set_sensitive(state->main_content, !carregando);

    if (state->loading_label)
        gtk_widget_set_visible(state->loading_label, carregando);
}

static void executar_pre_carregamentos(AppState *state)
{
    app_state_iniciar_carregamento(
        state,
        "Carregando dados e preparando janelas...");

    carregar_arquivo(state);
    preparar_janela_novo(state);

    app_state_finalizar_carregamento(state);
}
// ------------------------------------------------------------

static void carregar_css(void)
{
    GtkCssProvider *css =
        gtk_css_provider_new();

    gtk_css_provider_load_from_path(
        css,
        "assets/style.css");

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(css);
}

static void liberar_app_state(gpointer data)
{
    AppState *state = data;

    if (!state)
        return;

    if (state->data_timer_id != 0)
    {
        g_source_remove(state->data_timer_id);
        state->data_timer_id = 0;
    }

    if (state->form_adicionar && state->form_adicionar->window)
    {
        GtkWidget *window = state->form_adicionar->window;
        state->form_adicionar = NULL;
        gtk_window_destroy(GTK_WINDOW(window));
    }

    g_list_free_full(state->items, liberar_item_lista);
    g_free(state);
}


// ------------------------------------------------------------
// FECHAR JANELA
// ------------------------------------------------------------
static void fechar_janela(GtkWidget *widget, gpointer data)
{
    (void)widget;

    GtkWindow *window = GTK_WINDOW(data);

    GtkWidget *dialog =
        gtk_message_dialog_new(
            window,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_NONE,
            "Tem certeza que deseja sair?");

    gtk_dialog_add_buttons(
        GTK_DIALOG(dialog),
        "Cancelar", GTK_RESPONSE_CANCEL,
        "Sair", GTK_RESPONSE_OK,
        NULL);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(on_dialog_response),
                     window);

    gtk_widget_show(dialog);
}

static void on_dialog_response(GtkDialog *dialog, int response_id, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);

    if (response_id == GTK_RESPONSE_OK)
    {
        gtk_window_close(window);
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}
// ------------------------------------------------------------
