/*
 * formulario.c
 * Contem a janela de criacao e edicao de detentas.
 * Le os campos do formulario, valida os dados e atualiza a lista.
 */

#include "formulario.h"

#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "app_state.h"
#include "arquivo.h"
#include "lista.h"
#include "pessoa.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static const char *GENEROS[] = {
    "Mulher Cis",
    "Mulher Trans",
    NULL};

static const char *PERICULOSIDADES[] = {
    "Baixa",
    "Media",
    "Alta",
    "Extrema",
    NULL};

static const char *COMPORTAMENTOS[] = {
    "Bom",
    "Regular",
    "Ruim",
    "Critico",
    NULL};

static const char *REGIMES[] = {
    "Fechado",
    "Semiaberto",
    "Aberto",
    NULL};

static const char *DOENCAS[] = {
    "Nenhuma",
    "Diabetes",
    "Hipertensão",
    "Asma",
    "Cardiopatia",
    "Alergias",
    NULL};

static void abrir_janela_adicionar_helper(AppState *state, gboolean modo_edicao, ItemLista *item_editando, gboolean apresentar);
static void on_window_destroy(GtkWidget *window, gpointer data);
static gboolean on_window_close_request(GtkWindow *window, gpointer data);
static void salvar_pessoa(GtkWidget *widget, gpointer data);
static void cancelar_formulario(GtkWidget *widget, gpointer data);
static void escolher_imagem(GtkButton *button, gpointer data);
static void on_imagem_response(GtkNativeDialog *dialog, int response_id, gpointer data);
static void abrir_camera(GtkButton *button, gpointer data);
static GtkWindow *obter_janela_principal(FormData *form);
static void apresentar_janela_principal(FormData *form);
static GtkWidget *criar_campo_com_contador(GtkWidget *entry, int maximo);
static void atualizar_contador_campo(GtkEditable *editable, gpointer data);
static GtkWidget *criar_spin_inteiro(double minimo, double maximo, double passo);
static GtkWidget *criar_spin_decimal(double minimo, double maximo, double passo, guint digitos);
static void copiar_texto(char *destino, size_t tamanho, GtkWidget *entry);
static void copiar_opcao(char *destino, size_t tamanho, GtkWidget *dropdown, const char *opcoes[]);
static void selecionar_opcao(GtkWidget *dropdown, const char *valor, const char *opcoes[]);
static GtkWidget *criar_checklist_doencas(FormData *form);
static void copiar_doencas(char *destino, size_t tamanho, FormData *form);
static void selecionar_doencas(FormData *form, const char *valor);
static void limpar_doencas(FormData *form);
static void on_doenca_toggled(GtkCheckButton *button, gpointer data);
static void filtrar_data(GtkEditable *editable, gpointer data);
static void atualizar_idade(GtkEditable *editable, gpointer data);
static void atualizar_tempo_restante(GtkWidget *widget, gpointer data);
static int calcular_idade(const char *data_nascimento);
static int calcular_tempo_restante(int pena_anos, const char *data_prisao);
static gboolean validar_formulario(FormData *form, GtkWidget *widget);
static gboolean data_valida(const char *texto);
static void mostrar_erro_validacao(GtkWidget *widget, const char *mensagem);
static void reset_form(FormData *form);
static void preparar_formulario_criacao(FormData *form);
static void atualizar_preview_imagem(FormData *form);
static void reordenar_item_editado(AppState *state, ItemLista *item);
static int gerar_serie(AppState *state);

static GtkWindow *obter_janela_principal(FormData *form)
{
    if (!form || !form->state || !form->state->list)
        return NULL;

    GtkRoot *root =
        gtk_widget_get_root(form->state->list);

    if (!root || !GTK_IS_WINDOW(root))
        return NULL;

    return GTK_WINDOW(root);
}

static void apresentar_janela_principal(FormData *form)
{
    GtkWindow *window =
        obter_janela_principal(form);

    if (window)
        gtk_window_present(window);
}

// ------------------------------------------------------------
// ABRIR JANELAS ADICIONAIS
// ------------------------------------------------------------
// Abre o formulario em modo de criacao ou edicao.
void abrir_janela_novo(GtkWidget *widget, gpointer data)
{
    (void)widget;

    AppState *state = data;

    if (state->form_adicionar)
    {
        if (!gtk_widget_get_visible(state->form_adicionar->window))
            preparar_formulario_criacao(state->form_adicionar);

        gtk_window_present(GTK_WINDOW(state->form_adicionar->window));
        return;
    }

    abrir_janela_adicionar_helper(state, FALSE, NULL, TRUE);
}

void abrir_janela_modificar(GtkWidget *widget, gpointer data)
{
    (void)widget;

    AppState *state = data;

    if (!state->selected_item)
        return;

    abrir_janela_adicionar_helper(state, TRUE, state->selected_item, TRUE);
}

void preparar_janela_novo(AppState *state)
{
    if (!state || state->form_adicionar)
        return;

    abrir_janela_adicionar_helper(state, FALSE, NULL, FALSE);
}

static void abrir_janela_adicionar_helper(AppState *state, gboolean modo_edicao, ItemLista *item_editando, gboolean apresentar)
{
    FormData *form = g_malloc0(sizeof(FormData));

    form->state = state;
    form->list = state->list;
    form->modo_edicao = modo_edicao;
    form->item_editando = item_editando;

    GtkWidget *window =
        GTK_WIDGET(
            gtk_application_window_new(
                GTK_APPLICATION(
                    g_application_get_default())));

    // ------------------------------------------------------------
    // JANELA
    // ------------------------------------------------------------

    // janela pai
    GtkWindow *parent =
        GTK_WINDOW(
            gtk_widget_get_root(state->list));

    // conecta como modal da principal
    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent);

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE);

    // salva referencia
    form->window = window;

    g_signal_connect(
        window,
        "destroy",
        G_CALLBACK(on_window_destroy),
        form);

    g_signal_connect(
        window,
        "close-request",
        G_CALLBACK(on_window_close_request),
        form);

    gtk_window_set_title(
        GTK_WINDOW(window),
        (modo_edicao ? "Modificar Detenta" : "Adicionar Detenta"));

    gtk_window_set_default_size(GTK_WINDOW(window), 840, 780);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    // ------------------------------------------------------------
    // CONTAINER PRINCIPAL
    // ------------------------------------------------------------

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);

    gtk_widget_add_css_class(
        main_box,
        "form-shell");

    gtk_widget_set_margin_top(main_box, 22);
    gtk_widget_set_margin_bottom(main_box, 22);
    gtk_widget_set_margin_start(main_box, 22);
    gtk_widget_set_margin_end(main_box, 22);

    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // ------------------------------------------------------------
    // TITULO
    // ------------------------------------------------------------

    GtkWidget *header_form =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    GtkWidget *titulo =
        gtk_label_new(modo_edicao ? "Modificar Registro" : "Novo Registro de Detenta");

    GtkWidget *subtitulo =
        gtk_label_new("Preencha os dados cadastrais e revise as informações antes de salvar.");

    gtk_widget_add_css_class(titulo, "title-2");
    gtk_widget_add_css_class(subtitulo, "dim-label");

    gtk_label_set_xalign(GTK_LABEL(titulo), 0);
    gtk_label_set_xalign(GTK_LABEL(subtitulo), 0);

    gtk_box_append(GTK_BOX(header_form), titulo);
    gtk_box_append(GTK_BOX(header_form), subtitulo);

    gtk_box_append(GTK_BOX(main_box), header_form);

    // ------------------------------------------------------------
    // AREA DO FORMULARIO
    // ------------------------------------------------------------

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_add_css_class(frame, "panel");
    gtk_widget_add_css_class(frame, "form-panel");
    gtk_box_append(GTK_BOX(main_box), frame);

    GtkWidget *scroll =
        gtk_scrolled_window_new();

    gtk_widget_set_vexpand(scroll, TRUE);

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC);

    gtk_frame_set_child(
        GTK_FRAME(frame),
        scroll);

    GtkWidget *content =
        gtk_notebook_new();

    gtk_widget_add_css_class(
        content,
        "form-tabs");

    gtk_widget_set_margin_top(content, 12);
    gtk_widget_set_margin_bottom(content, 12);
    gtk_widget_set_margin_start(content, 12);
    gtk_widget_set_margin_end(content, 12);

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        content);

    // ------------------------------------------------------------
    // CAMPOS
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // DADOS PESSOAIS
    // ------------------------------------------------------------

    GtkWidget *frame_pessoal =
        gtk_frame_new("Dados Pessoais");

    gtk_widget_add_css_class(frame_pessoal, "section-card");

    gtk_notebook_append_page(
        GTK_NOTEBOOK(content),
        frame_pessoal,
        gtk_label_new("Dados pessoais"));

    GtkWidget *grid_pessoal =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid_pessoal),
        14);

    gtk_grid_set_column_spacing(
        GTK_GRID(grid_pessoal),
        18);

    gtk_widget_set_margin_top(grid_pessoal, 18);
    gtk_widget_set_margin_bottom(grid_pessoal, 18);
    gtk_widget_set_margin_start(grid_pessoal, 18);
    gtk_widget_set_margin_end(grid_pessoal, 18);

    gtk_frame_set_child(
        GTK_FRAME(frame_pessoal),
        grid_pessoal);

    // Campos do FORMULARIO
    form->entry_nome = gtk_entry_new();
    form->entry_apelido = gtk_entry_new();
    form->entry_idade = gtk_label_new("Calculada pela data de nascimento");
    form->entry_nacionalidade = gtk_entry_new();
    form->entry_genero = gtk_drop_down_new_from_strings(GENEROS);
    form->entry_altura = criar_spin_decimal(0.0, 3.0, 0.01, 2);
    form->entry_peso = criar_spin_decimal(0.0, 500.0, 0.1, 1);
    form->entry_data_nascimento = gtk_entry_new();
    form->entry_imagem = gtk_entry_new();
    form->preview_imagem = gtk_image_new_from_icon_name("image-x-generic-symbolic");

    GtkWidget *box_imagem =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_add_css_class(
        box_imagem,
        "media-field");

    GtkWidget *linha_imagem =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    GtkWidget *btn_imagem =
        gtk_button_new_with_label("Escolher...");

    GtkWidget *btn_camera =
        gtk_button_new_with_label("Webcam");

    gtk_widget_add_css_class(btn_imagem, "small-action");
    gtk_widget_add_css_class(btn_camera, "small-action");

    gtk_editable_set_editable(
        GTK_EDITABLE(form->entry_imagem),
        FALSE);

    gtk_image_set_pixel_size(
        GTK_IMAGE(form->preview_imagem),
        120);

    gtk_widget_set_size_request(
        form->preview_imagem,
        140,
        140);

    gtk_widget_set_halign(
        form->preview_imagem,
        GTK_ALIGN_START);

    gtk_widget_set_valign(
        form->preview_imagem,
        GTK_ALIGN_START);

    gtk_widget_set_vexpand(
        form->preview_imagem,
        FALSE);

    gtk_widget_add_css_class(
        form->preview_imagem,
        "photo-preview");

    gtk_widget_set_hexpand(
        box_imagem,
        TRUE);

    gtk_box_append(
        GTK_BOX(linha_imagem),
        criar_campo_com_contador(form->entry_imagem, sizeof(((Pessoa *)0)->imagem) - 1));

    gtk_box_append(
        GTK_BOX(linha_imagem),
        btn_imagem);

    gtk_box_append(
        GTK_BOX(linha_imagem),
        btn_camera);

    gtk_box_append(
        GTK_BOX(box_imagem),
        linha_imagem);

    gtk_box_append(
        GTK_BOX(box_imagem),
        form->preview_imagem);

    g_signal_connect(
        btn_imagem,
        "clicked",
        G_CALLBACK(escolher_imagem),
        form);

    g_signal_connect(
        btn_camera,
        "clicked",
        G_CALLBACK(abrir_camera),
        form);

    gtk_drop_down_set_selected(
        GTK_DROP_DOWN(form->entry_genero),
        0);

    gtk_entry_set_input_purpose(
        GTK_ENTRY(form->entry_data_nascimento),
        GTK_INPUT_PURPOSE_DIGITS);

    gtk_entry_set_max_length(
        GTK_ENTRY(form->entry_data_nascimento),
        10);

    g_signal_connect(form->entry_data_nascimento, "changed", G_CALLBACK(filtrar_data), NULL);
    g_signal_connect(form->entry_data_nascimento, "changed", G_CALLBACK(atualizar_idade), form);

    // serie
    GtkWidget *serie_label =
        gtk_label_new("");

    form->label_serie =
        serie_label;

    int serie_temp =
        gerar_serie(form->state);

    form->serie_temp =
        serie_temp;

    char buf[32];

    snprintf(buf, sizeof(buf), "%06d", serie_temp);

    gtk_label_set_text(
        GTK_LABEL(serie_label),
        buf);

    gtk_widget_add_css_class(
        serie_label,
        "heading");

// Macro auxiliar para adicionar campos ao grid
#define ADD_FIELD(grid, texto, widget, linha)              \
    {                                                      \
        GtkWidget *label = gtk_label_new(texto);           \
        GtkWidget *field_widget = (widget);                \
                                                           \
        gtk_label_set_xalign(GTK_LABEL(label), 0);         \
                                                           \
        gtk_widget_set_halign(label, GTK_ALIGN_START);     \
        gtk_widget_set_valign(label, GTK_ALIGN_START);     \
        gtk_widget_set_margin_top(label, 9);               \
                                                           \
        gtk_grid_attach(GTK_GRID(grid),                    \
                        label,                             \
                        0, linha, 1, 1);                   \
                                                           \
        gtk_widget_set_hexpand(field_widget, TRUE);        \
                                                           \
        gtk_grid_attach(GTK_GRID(grid),                    \
                        field_widget,                      \
                        1, linha, 1, 1);                   \
    }

    // Adiciona campos ao grid
    ADD_FIELD(grid_pessoal, "Nome",
              criar_campo_com_contador(form->entry_nome, sizeof(((Pessoa *)0)->nome) - 1), 0);
    ADD_FIELD(grid_pessoal, "Apelido",
              criar_campo_com_contador(form->entry_apelido, sizeof(((Pessoa *)0)->apelido) - 1), 1);
    ADD_FIELD(grid_pessoal, "Série", serie_label, 2);
    ADD_FIELD(grid_pessoal, "Nascimento",
              criar_campo_com_contador(form->entry_data_nascimento, 10), 3);
    ADD_FIELD(grid_pessoal, "Idade", form->entry_idade, 4);
    ADD_FIELD(grid_pessoal, "Nacionalidade",
              criar_campo_com_contador(form->entry_nacionalidade, sizeof(((Pessoa *)0)->nacionalidade) - 1), 5);
    ADD_FIELD(grid_pessoal, "Gênero", form->entry_genero, 6);
    ADD_FIELD(grid_pessoal, "Altura", form->entry_altura, 7);
    ADD_FIELD(grid_pessoal, "Peso", form->entry_peso, 8);
    ADD_FIELD(grid_pessoal, "Imagem", box_imagem, 9);

    // ------------------------------------------------------------
    // SISTEMA PRISIONAL
    // ------------------------------------------------------------

    GtkWidget *frame_prisional =
        gtk_frame_new("Sistema Prisional");

    gtk_widget_add_css_class(frame_prisional, "section-card");

    gtk_notebook_append_page(
        GTK_NOTEBOOK(content),
        frame_prisional,
        gtk_label_new("Prisional"));

    GtkWidget *grid_prisional =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid_prisional),
        14);

    gtk_grid_set_column_spacing(
        GTK_GRID(grid_prisional),
        18);

    gtk_widget_set_margin_top(grid_prisional, 18);
    gtk_widget_set_margin_bottom(grid_prisional, 18);
    gtk_widget_set_margin_start(grid_prisional, 18);
    gtk_widget_set_margin_end(grid_prisional, 18);

    gtk_frame_set_child(
        GTK_FRAME(frame_prisional),
        grid_prisional);

    form->entry_crime = gtk_entry_new();
    form->entry_pena = criar_spin_inteiro(0, 200, 1);
    form->entry_cela = gtk_entry_new();
    form->entry_perigo = gtk_drop_down_new_from_strings(PERICULOSIDADES);
    form->entry_comportamento = gtk_drop_down_new_from_strings(COMPORTAMENTOS);
    form->entry_regime = gtk_drop_down_new_from_strings(REGIMES);
    form->entry_data_prisao = gtk_entry_new();
    form->entry_tempo_restante = gtk_label_new("Calculado automaticamente");
    form->check_isolamento = gtk_check_button_new();
    form->check_risco_fuga = gtk_check_button_new();

    gtk_entry_set_input_purpose(
        GTK_ENTRY(form->entry_data_prisao),
        GTK_INPUT_PURPOSE_DIGITS);

    gtk_label_set_xalign(
        GTK_LABEL(form->entry_tempo_restante),
        0.0);

    gtk_widget_set_halign(
        form->entry_tempo_restante,
        GTK_ALIGN_START);

    gtk_widget_add_css_class(
        form->entry_tempo_restante,
        "computed-value");

    gtk_entry_set_max_length(
        GTK_ENTRY(form->entry_data_prisao),
        10);

    g_signal_connect(form->entry_pena, "value-changed", G_CALLBACK(atualizar_tempo_restante), form);
    g_signal_connect(form->entry_data_prisao, "changed", G_CALLBACK(filtrar_data), NULL);
    g_signal_connect(form->entry_data_prisao, "changed", G_CALLBACK(atualizar_tempo_restante), form);

    ADD_FIELD(grid_prisional, "Crime",
              criar_campo_com_contador(form->entry_crime, sizeof(((Pessoa *)0)->crime) - 1), 0);
    ADD_FIELD(grid_prisional, "Pena", form->entry_pena, 1);
    ADD_FIELD(grid_prisional, "Cela",
              criar_campo_com_contador(form->entry_cela, sizeof(((Pessoa *)0)->cela) - 1), 2);
    ADD_FIELD(grid_prisional, "Periculosidade", form->entry_perigo, 3);
    ADD_FIELD(grid_prisional, "Comportamento", form->entry_comportamento, 4);
    ADD_FIELD(grid_prisional, "Regime", form->entry_regime, 5);
    ADD_FIELD(grid_prisional, "Data Prisão",
              criar_campo_com_contador(form->entry_data_prisao, 10), 6);
    ADD_FIELD(grid_prisional, "Tempo Restante", form->entry_tempo_restante, 7);
    ADD_FIELD(grid_prisional, "Isolamento", form->check_isolamento, 8);
    ADD_FIELD(grid_prisional, "Risco de Fuga", form->check_risco_fuga, 9);

    // ------------------------------------------------------------
    // saude
    // ------------------------------------------------------------

    GtkWidget *frame_saude =
        gtk_frame_new("Saúde");

    gtk_widget_add_css_class(frame_saude, "section-card");

    gtk_notebook_append_page(
        GTK_NOTEBOOK(content),
        frame_saude,
        gtk_label_new("Saude"));

    GtkWidget *grid_saude =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid_saude),
        14);

    gtk_grid_set_column_spacing(
        GTK_GRID(grid_saude),
        18);

    gtk_widget_set_margin_top(grid_saude, 18);
    gtk_widget_set_margin_bottom(grid_saude, 18);
    gtk_widget_set_margin_start(grid_saude, 18);
    gtk_widget_set_margin_end(grid_saude, 18);

    gtk_frame_set_child(
        GTK_FRAME(frame_saude),
        grid_saude);

    form->entry_saude =
        gtk_entry_new();

    form->entry_psicologico =
        gtk_entry_new();

    form->entry_medicacao =
        gtk_entry_new();

    ADD_FIELD(grid_saude, "Saúde",
              criar_campo_com_contador(form->entry_saude, sizeof(((Pessoa *)0)->saude) - 1), 0);
    ADD_FIELD(grid_saude, "Psicológico",
              criar_campo_com_contador(form->entry_psicologico, sizeof(((Pessoa *)0)->psicologico) - 1), 1);
    ADD_FIELD(grid_saude, "Medicação",
              criar_campo_com_contador(form->entry_medicacao, sizeof(((Pessoa *)0)->medicacao) - 1), 2);
    ADD_FIELD(grid_saude, "Doenças", criar_checklist_doencas(form), 3);

    // ------------------------------------------------------------
    // SEGURANCA
    // ------------------------------------------------------------

    GtkWidget *frame_seg =
        gtk_frame_new("Segurança");

    gtk_widget_add_css_class(frame_seg, "section-card");

    gtk_notebook_append_page(
        GTK_NOTEBOOK(content),
        frame_seg,
        gtk_label_new("Seguranca"));

    GtkWidget *grid_seg =
        gtk_grid_new();

    gtk_grid_set_row_spacing(
        GTK_GRID(grid_seg),
        14);

    gtk_grid_set_column_spacing(
        GTK_GRID(grid_seg),
        18);

    gtk_widget_set_margin_top(grid_seg, 18);
    gtk_widget_set_margin_bottom(grid_seg, 18);
    gtk_widget_set_margin_start(grid_seg, 18);
    gtk_widget_set_margin_end(grid_seg, 18);

    gtk_frame_set_child(
        GTK_FRAME(frame_seg),
        grid_seg);

    form->entry_tentativas_fuga =
        criar_spin_inteiro(0, 999, 1);

    form->entry_alerta =
        gtk_entry_new();

    form->entry_historico_violencia =
        gtk_entry_new();

    form->entry_ultima_ocorrencia =
        gtk_entry_new();

    ADD_FIELD(grid_seg, "Tentativas de fuga",
              form->entry_tentativas_fuga, 0);

    ADD_FIELD(grid_seg, "Alerta",
              criar_campo_com_contador(form->entry_alerta, sizeof(((Pessoa *)0)->alerta) - 1), 1);

    ADD_FIELD(grid_seg,
              "Histórico Violência",
              criar_campo_com_contador(form->entry_historico_violencia, sizeof(((Pessoa *)0)->historico_violencia) - 1),
              2);

    ADD_FIELD(grid_seg,
              "Última Ocorrência",
              criar_campo_com_contador(form->entry_ultima_ocorrencia, sizeof(((Pessoa *)0)->ultima_ocorrencia) - 1),
              3);

    // ------------------------------------------------------------
    // botao
    // ------------------------------------------------------------

    GtkWidget *box_acoes =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    gtk_widget_add_css_class(
        box_acoes,
        "action-bar");

    GtkWidget *btn_cancelar =
        gtk_button_new_with_label("Cancelar");

    GtkWidget *btn_salvar =
        gtk_button_new_with_label(modo_edicao ? "Salvar alterações" : "Cadastrar");

    gtk_widget_set_size_request(btn_salvar, -1, 45);
    gtk_widget_set_size_request(btn_cancelar, -1, 45);
    gtk_widget_set_hexpand(btn_cancelar, TRUE);
    gtk_widget_set_hexpand(btn_salvar, TRUE);

    gtk_widget_add_css_class(btn_salvar, "suggested-action");

    gtk_box_append(
        GTK_BOX(box_acoes),
        btn_cancelar);

    gtk_box_append(
        GTK_BOX(box_acoes),
        btn_salvar);

    gtk_box_append(
        GTK_BOX(main_box),
        box_acoes);

    g_signal_connect(
        btn_cancelar,
        "clicked",
        G_CALLBACK(cancelar_formulario),
        form);

    g_signal_connect(
        btn_salvar,
        "clicked",
        G_CALLBACK(salvar_pessoa),
        form);

    // ------------------------------------------------------------
    // MODO EDICAO
    // ------------------------------------------------------------

    if (!form->modo_edicao)
    {
        state->form_adicionar = form;
        preparar_formulario_criacao(form);
    }
    else if (form->item_editando)
    {
        Pessoa *p =
            form->item_editando->pessoa;

        char buf[64];

        // ------------------------------------------------------------
        // DADOS PESSOAIS
        // ------------------------------------------------------------

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_nome),
            p->nome);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_apelido),
            p->apelido);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_nacionalidade),
            p->nacionalidade);

        selecionar_opcao(
            form->entry_genero,
            p->genero,
            GENEROS);

        gtk_spin_button_set_value(
            GTK_SPIN_BUTTON(form->entry_altura),
            p->altura);

        gtk_spin_button_set_value(
            GTK_SPIN_BUTTON(form->entry_peso),
            p->peso);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_data_nascimento),
            p->data_nascimento);

        atualizar_idade(
            GTK_EDITABLE(form->entry_data_nascimento),
            form);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_imagem),
            g_strcmp0(p->imagem, "-") == 0 ? "" : p->imagem);

        atualizar_preview_imagem(form);

        // ------------------------------------------------------------
        // CRIME
        // ------------------------------------------------------------

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_crime),
            p->crime);

        gtk_spin_button_set_value(
            GTK_SPIN_BUTTON(form->entry_pena),
            p->pena_anos);

        // ------------------------------------------------------------
        // SISTEMA PRISIONAL
        // ------------------------------------------------------------

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_cela),
            p->cela);

        selecionar_opcao(
            form->entry_perigo,
            p->perigo,
            PERICULOSIDADES);

        selecionar_opcao(
            form->entry_comportamento,
            p->comportamento,
            COMPORTAMENTOS);

        selecionar_opcao(
            form->entry_regime,
            p->regime,
            REGIMES);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_data_prisao),
            p->data_prisao);

        snprintf(
            buf,
            sizeof(buf),
            "%d",
            calcular_tempo_restante(p->pena_anos, p->data_prisao));

        gtk_label_set_text(
            GTK_LABEL(form->entry_tempo_restante),
            buf);

        gtk_check_button_set_active(
            GTK_CHECK_BUTTON(form->check_isolamento),
            p->isolamento);

        gtk_check_button_set_active(
            GTK_CHECK_BUTTON(form->check_risco_fuga),
            p->risco_fuga);

        // ------------------------------------------------------------
        // saude
        // ------------------------------------------------------------

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_saude),
            p->saude);

        selecionar_doencas(form, p->doencas);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_psicologico),
            p->psicologico);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_medicacao),
            p->medicacao);

        // ------------------------------------------------------------
        // SEGURANCA
        // ------------------------------------------------------------

        gtk_spin_button_set_value(
            GTK_SPIN_BUTTON(form->entry_tentativas_fuga),
            p->tentativas_fuga);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_historico_violencia),
            p->historico_violencia);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_ultima_ocorrencia),
            p->ultima_ocorrencia);

        gtk_editable_set_text(
            GTK_EDITABLE(form->entry_alerta),
            p->alerta);
    }

    if (apresentar)
        gtk_window_present(GTK_WINDOW(window));
}
// ------------------------------------------------------------

// ------------------------------------------------------------
// SALVAR DETENTA
// ------------------------------------------------------------
static void copiar_texto(char *destino, size_t tamanho, GtkWidget *entry)
{
    strncpy(
        destino,
        gtk_editable_get_text(GTK_EDITABLE(entry)),
        tamanho - 1);

    destino[tamanho - 1] = '\0';
}

static GtkWidget *criar_campo_com_contador(GtkWidget *entry, int maximo)
{
    GtkWidget *box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    GtkWidget *contador =
        gtk_label_new(NULL);

    gtk_entry_set_max_length(
        GTK_ENTRY(entry),
        maximo);

    gtk_widget_set_hexpand(entry, TRUE);
    gtk_widget_set_hexpand(box, TRUE);

    gtk_label_set_xalign(
        GTK_LABEL(contador),
        1.0);

    gtk_widget_set_halign(
        contador,
        GTK_ALIGN_END);

    gtk_widget_add_css_class(
        contador,
        "char-counter");

    g_object_set_data(
        G_OBJECT(contador),
        "maximo",
        GINT_TO_POINTER(maximo));

    g_signal_connect(
        entry,
        "changed",
        G_CALLBACK(atualizar_contador_campo),
        contador);

    gtk_box_append(
        GTK_BOX(box),
        entry);

    gtk_box_append(
        GTK_BOX(box),
        contador);

    atualizar_contador_campo(
        GTK_EDITABLE(entry),
        contador);

    return box;
}

static void atualizar_contador_campo(GtkEditable *editable, gpointer data)
{
    GtkWidget *contador =
        GTK_WIDGET(data);

    int maximo =
        GPOINTER_TO_INT(
            g_object_get_data(G_OBJECT(contador), "maximo"));

    const char *texto =
        gtk_editable_get_text(editable);

    glong atual =
        g_utf8_strlen(texto ? texto : "", -1);

    char resumo[64];

    snprintf(
        resumo,
        sizeof(resumo),
        "%ld/%d caracteres",
        atual,
        maximo);

    gtk_label_set_text(
        GTK_LABEL(contador),
        resumo);
}

static void atualizar_preview_imagem(FormData *form)
{
    const char *caminho =
        gtk_editable_get_text(GTK_EDITABLE(form->entry_imagem));

    if (caminho && caminho[0] != '\0' && g_file_test(caminho, G_FILE_TEST_EXISTS))
    {
        gtk_image_set_from_file(
            GTK_IMAGE(form->preview_imagem),
            caminho);
    }
    else
    {
        gtk_image_set_from_icon_name(
            GTK_IMAGE(form->preview_imagem),
            "image-x-generic-symbolic");
    }

    gtk_image_set_pixel_size(
        GTK_IMAGE(form->preview_imagem),
        120);
}

static void escolher_imagem(GtkButton *button, gpointer data)
{
    FormData *form = data;

    GtkFileChooserNative *dialog =
        gtk_file_chooser_native_new(
            "Selecionar imagem",
            GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button))),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            "Selecionar",
            "Cancelar");

    GtkFileFilter *filter =
        gtk_file_filter_new();

    gtk_file_filter_set_name(
        filter,
        "Imagens");

    gtk_file_filter_add_mime_type(filter, "image/png");
    gtk_file_filter_add_mime_type(filter, "image/jpeg");
    gtk_file_filter_add_mime_type(filter, "image/webp");
    gtk_file_filter_add_mime_type(filter, "image/gif");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.webp");
    gtk_file_filter_add_pattern(filter, "*.gif");

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(dialog),
        filter);

    g_mkdir_with_parents("fotos", 0755);

    GFile *pasta_fotos =
        g_file_new_for_path("fotos");

    GError *erro_pasta = NULL;

    if (!gtk_file_chooser_set_current_folder(
            GTK_FILE_CHOOSER(dialog),
            pasta_fotos,
            &erro_pasta))
    {
        g_warning(
            "Nao foi possivel abrir a pasta fotos: %s",
            erro_pasta ? erro_pasta->message : "erro desconhecido");

        if (erro_pasta)
            g_error_free(erro_pasta);
    }

    g_object_unref(pasta_fotos);

    g_signal_connect(
        dialog,
        "response",
        G_CALLBACK(on_imagem_response),
        form);

    gtk_native_dialog_show(
        GTK_NATIVE_DIALOG(dialog));
}

static void on_imagem_response(GtkNativeDialog *dialog, int response_id, gpointer data)
{
    FormData *form = data;

    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        GFile *file =
            gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));

        if (file)
        {
            char *caminho =
                g_file_get_path(file);

            if (caminho)
            {
                gtk_editable_set_text(
                    GTK_EDITABLE(form->entry_imagem),
                    caminho);

                atualizar_preview_imagem(form);
                g_free(caminho);
            }

            g_object_unref(file);
        }
    }

    g_object_unref(dialog);
}

static void abrir_camera(GtkButton *button, gpointer data)
{
    FormData *form = data;
    (void)button;

    char *stdout_text = NULL;
    char *stderr_text = NULL;
    GError *erro = NULL;
    gint status = 0;
    char *argv[] = {".\\bin\\camera_capture.exe", NULL};
    char **envp = g_get_environ();
    const char *path_atual = g_environ_getenv(envp, "PATH");
    char *path_camera =
        g_strdup_printf(
            "C:\\msys64\\ucrt64\\bin;C:\\msys64\\usr\\bin;C:\\Windows\\System32\\downlevel;%s",
            path_atual ? path_atual : "");

    envp =
        g_environ_setenv(envp, "PATH", path_camera, TRUE);

    gboolean executou =
        g_spawn_sync(
            NULL,
            argv,
            envp,
            0,
            NULL,
            NULL,
            &stdout_text,
            &stderr_text,
            &status,
            &erro);

    g_strfreev(envp);
    g_free(path_camera);

    if (!executou)
    {
        mostrar_erro_validacao(
            GTK_WIDGET(button),
            erro ? erro->message : "Nao consegui abrir o capturador de webcam.");

        if (erro)
            g_error_free(erro);

        g_free(stdout_text);
        g_free(stderr_text);
        return;
    }

    if (status != 0)
    {
        if (status == 1 ||
            (stderr_text && g_strrstr(stderr_text, "Captura cancelada.") != NULL))
        {
            g_free(stdout_text);
            g_free(stderr_text);
            return;
        }

        mostrar_erro_validacao(
            GTK_WIDGET(button),
            stderr_text && stderr_text[0] != '\0'
                ? stderr_text
                : "A captura pela webcam foi cancelada ou nao encontrou uma camera.");

        g_free(stdout_text);
        g_free(stderr_text);
        return;
    }

    char *caminho =
        g_strstrip(stdout_text);

    if (!caminho || caminho[0] == '\0')
    {
        mostrar_erro_validacao(
            GTK_WIDGET(button),
            "A webcam capturou, mas nao devolveu o caminho da foto.");

        g_free(stdout_text);
        g_free(stderr_text);
        return;
    }

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_imagem),
        caminho);

    atualizar_preview_imagem(form);

    g_free(stdout_text);
    g_free(stderr_text);
}

static GtkWidget *criar_spin_inteiro(double minimo, double maximo, double passo)
{
    GtkWidget *spin =
        gtk_spin_button_new_with_range(minimo, maximo, passo);

    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), 0);

    return spin;
}

static GtkWidget *criar_spin_decimal(double minimo, double maximo, double passo, guint digitos)
{
    GtkWidget *spin =
        gtk_spin_button_new_with_range(minimo, maximo, passo);

    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), digitos);

    return spin;
}

static void copiar_opcao(char *destino, size_t tamanho, GtkWidget *dropdown, const char *opcoes[])
{
    guint selecionado =
        gtk_drop_down_get_selected(GTK_DROP_DOWN(dropdown));

    if (selecionado == GTK_INVALID_LIST_POSITION)
        selecionado = 0;

    for (guint i = 0; i < selecionado; i++)
    {
        if (!opcoes[i])
        {
            selecionado = 0;
            break;
        }
    }

    if (!opcoes[selecionado])
        selecionado = 0;

    snprintf(
        destino,
        tamanho,
        "%s",
        opcoes[selecionado]);
}

static void selecionar_opcao(GtkWidget *dropdown, const char *valor, const char *opcoes[])
{
    for (guint i = 0; opcoes[i] != NULL; i++)
    {
        if (g_strcmp0(valor, opcoes[i]) == 0)
        {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown), i);
            return;
        }
    }

    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown), 0);
}

static GtkWidget *criar_checklist_doencas(FormData *form)
{
    GtkWidget *box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    for (int i = 0; i < FORM_DOENCAS_TOTAL; i++)
    {
        form->check_doencas[i] =
            gtk_check_button_new_with_label(DOENCAS[i]);

        g_signal_connect(
            form->check_doencas[i],
            "toggled",
            G_CALLBACK(on_doenca_toggled),
            form);

        gtk_box_append(
            GTK_BOX(box),
            form->check_doencas[i]);
    }

    gtk_check_button_set_active(
        GTK_CHECK_BUTTON(form->check_doencas[0]),
        TRUE);

    return box;
}

static void copiar_doencas(char *destino, size_t tamanho, FormData *form)
{
    destino[0] = '\0';

    for (int i = 0; i < FORM_DOENCAS_TOTAL; i++)
    {
        if (!gtk_check_button_get_active(GTK_CHECK_BUTTON(form->check_doencas[i])))
            continue;

        if (destino[0] != '\0')
            g_strlcat(destino, "; ", tamanho);

        g_strlcat(destino, DOENCAS[i], tamanho);
    }

    if (destino[0] == '\0')
        snprintf(destino, tamanho, "Nenhuma");
}

static void selecionar_doencas(FormData *form, const char *valor)
{
    gboolean encontrou = FALSE;

    for (int i = 0; i < FORM_DOENCAS_TOTAL; i++)
    {
        gboolean marcado =
            valor && g_strrstr(valor, DOENCAS[i]) != NULL;

        gtk_check_button_set_active(
            GTK_CHECK_BUTTON(form->check_doencas[i]),
            marcado);

        encontrou = encontrou || marcado;
    }

    if (!encontrou)
    {
        gtk_check_button_set_active(
            GTK_CHECK_BUTTON(form->check_doencas[0]),
            TRUE);
    }
}

static void limpar_doencas(FormData *form)
{
    for (int i = 1; i < FORM_DOENCAS_TOTAL; i++)
    {
        gtk_check_button_set_active(
            GTK_CHECK_BUTTON(form->check_doencas[i]),
            FALSE);
    }

    gtk_check_button_set_active(
        GTK_CHECK_BUTTON(form->check_doencas[0]),
        TRUE);
}

static void on_doenca_toggled(GtkCheckButton *button, gpointer data)
{
    FormData *form = data;

    if (!gtk_check_button_get_active(button))
        return;

    if (GTK_WIDGET(button) == form->check_doencas[0])
    {
        for (int i = 1; i < FORM_DOENCAS_TOTAL; i++)
        {
            gtk_check_button_set_active(
                GTK_CHECK_BUTTON(form->check_doencas[i]),
                FALSE);
        }

        return;
    }

    gtk_check_button_set_active(
        GTK_CHECK_BUTTON(form->check_doencas[0]),
        FALSE);
}

static void atualizar_texto_filtrado(GtkEditable *editable, const char *texto)
{
    gtk_editable_set_text(editable, texto);
}

static void filtrar_data(GtkEditable *editable, gpointer data)
{
    (void)data;

    const char *texto = gtk_editable_get_text(editable);
    char filtrado[11];
    char digitos[9];
    size_t pos = 0;
    size_t total_digitos = 0;

    for (size_t i = 0; texto[i] != '\0' && total_digitos < sizeof(digitos) - 1; i++)
    {
        if (g_ascii_isdigit(texto[i]))
            digitos[total_digitos++] = texto[i];
    }

    digitos[total_digitos] = '\0';

    for (size_t i = 0; i < total_digitos && pos < sizeof(filtrado) - 1; i++)
    {
        if ((i == 2 || i == 4) && pos < sizeof(filtrado) - 1)
            filtrado[pos++] = '/';

        filtrado[pos++] = digitos[i];
    }

    filtrado[pos] = '\0';

    if (strcmp(texto, filtrado) != 0)
    {
        atualizar_texto_filtrado(editable, filtrado);
        gtk_editable_set_position(editable, -1);
    }
}

static int calcular_idade(const char *data_nascimento)
{
    int dia;
    int mes;
    int ano;

    if (!data_nascimento ||
        sscanf(data_nascimento, "%d/%d/%d", &dia, &mes, &ano) != 3)
        return -1;

    time_t agora = time(NULL);
    struct tm *hoje = localtime(&agora);

    if (!hoje)
        return -1;

    int ano_atual = hoje->tm_year + 1900;
    int mes_atual = hoje->tm_mon + 1;
    int dia_atual = hoje->tm_mday;
    int idade = ano_atual - ano;

    if (mes_atual < mes || (mes_atual == mes && dia_atual < dia))
        idade--;

    return idade;
}

static void atualizar_idade(GtkEditable *editable, gpointer data)
{
    FormData *form = data;
    const char *nascimento = gtk_editable_get_text(editable);
    int idade = calcular_idade(nascimento);
    char texto[32];

    if (idade < 0)
    {
        gtk_label_set_text(GTK_LABEL(form->entry_idade), "Calculada pela data de nascimento");
        return;
    }

    snprintf(texto, sizeof(texto), "%d anos", idade);
    gtk_label_set_text(GTK_LABEL(form->entry_idade), texto);
}

static int calcular_tempo_restante(int pena_anos, const char *data_prisao)
{
    int dia;
    int mes;
    int ano;

    if (pena_anos <= 0)
        return 0;

    if (!data_prisao || sscanf(data_prisao, "%d/%d/%d", &dia, &mes, &ano) != 3)
        return pena_anos;

    if (dia < 1 || dia > 31 || mes < 1 || mes > 12 || ano < 1900)
        return pena_anos;

    time_t agora = time(NULL);
    struct tm *hoje = localtime(&agora);

    if (!hoje)
        return pena_anos;

    int ano_atual = hoje->tm_year + 1900;
    int mes_atual = hoje->tm_mon + 1;
    int dia_atual = hoje->tm_mday;

    int anos_cumpridos = ano_atual - ano;

    if (mes_atual < mes || (mes_atual == mes && dia_atual < dia))
        anos_cumpridos--;

    if (anos_cumpridos < 0)
        anos_cumpridos = 0;

    if (anos_cumpridos > pena_anos)
        anos_cumpridos = pena_anos;

    return pena_anos - anos_cumpridos;
}

static void atualizar_tempo_restante(GtkWidget *widget, gpointer data)
{
    (void)widget;

    FormData *form = data;
    char buffer[32];

    int pena_anos =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(form->entry_pena));

    const char *data_prisao =
        gtk_editable_get_text(GTK_EDITABLE(form->entry_data_prisao));

    snprintf(
        buffer,
        sizeof(buffer),
        "%d",
        calcular_tempo_restante(pena_anos, data_prisao));

    gtk_label_set_text(
        GTK_LABEL(form->entry_tempo_restante),
        buffer);
}

static gboolean campo_vazio(const char *texto)
{
    return !texto || texto[0] == '\0';
}

static gboolean data_valida(const char *texto)
{
    int dia;
    int mes;
    int ano;
    char extra;
    int dias_no_mes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (campo_vazio(texto))
        return FALSE;

    if (sscanf(texto, "%2d/%2d/%4d%c", &dia, &mes, &ano, &extra) != 3)
        return FALSE;

    if (ano < 1900 || mes < 1 || mes > 12 || dia < 1)
        return FALSE;

    if ((ano % 4 == 0 && ano % 100 != 0) || ano % 400 == 0)
        dias_no_mes[1] = 29;

    return dia <= dias_no_mes[mes - 1];
}

static void mostrar_erro_validacao(GtkWidget *widget, const char *mensagem)
{
    GtkWidget *dialog =
        gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_root(widget)),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "%s",
            mensagem);

    g_signal_connect(
        dialog,
        "response",
        G_CALLBACK(gtk_window_destroy),
        NULL);

    gtk_widget_show(dialog);
}

static gboolean validar_formulario(FormData *form, GtkWidget *widget)
{
    const char *nome = gtk_editable_get_text(GTK_EDITABLE(form->entry_nome));
    const char *nascimento = gtk_editable_get_text(GTK_EDITABLE(form->entry_data_nascimento));
    const char *crime = gtk_editable_get_text(GTK_EDITABLE(form->entry_crime));
    const char *cela = gtk_editable_get_text(GTK_EDITABLE(form->entry_cela));
    const char *data_prisao = gtk_editable_get_text(GTK_EDITABLE(form->entry_data_prisao));

    if (campo_vazio(nome) || campo_vazio(nascimento) || campo_vazio(crime) ||
        campo_vazio(cela) || campo_vazio(data_prisao))
    {
        mostrar_erro_validacao(widget, "Preencha nome, nascimento, crime, pena, cela e data da prisão.");
        return FALSE;
    }

    int idade =
        calcular_idade(nascimento);

    int pena =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(form->entry_pena));

    if (!data_valida(nascimento))
    {
        mostrar_erro_validacao(widget, "Informe a data de nascimento no formato dd/mm/aaaa.");
        return FALSE;
    }

    if (idade < 0 || idade > 130)
    {
        mostrar_erro_validacao(widget, "A data de nascimento gera uma idade inválida.");
        return FALSE;
    }

    if (pena < 1)
    {
        mostrar_erro_validacao(widget, "Informe uma pena maior que zero.");
        return FALSE;
    }

    if (!data_valida(data_prisao))
    {
        mostrar_erro_validacao(widget, "Informe a data da prisão no formato dd/mm/aaaa.");
        return FALSE;
    }

    return TRUE;
}

static void salvar_pessoa(GtkWidget *widget, gpointer data)
{
    (void)widget;

    FormData *form = (FormData *)data;
    AppState *state = form->state;

    if (!validar_formulario(form, widget))
        return;

    // ------------------------------------------------------------
    // MODO EDICAO
    // ------------------------------------------------------------
    if (form->modo_edicao && form->item_editando)
    {
        Pessoa *p =
            form->item_editando->pessoa;

        // ------------------------------------------------------------
        // DADOS PESSOAIS
        // ------------------------------------------------------------

        copiar_texto(
            p->nome,
            sizeof(p->nome),
            form->entry_nome);

        copiar_texto(
            p->apelido,
            sizeof(p->apelido),
            form->entry_apelido);

        p->idade =
            calcular_idade(
                gtk_editable_get_text(GTK_EDITABLE(form->entry_data_nascimento)));

        copiar_texto(
            p->nacionalidade,
            sizeof(p->nacionalidade),
            form->entry_nacionalidade);

        copiar_opcao(
            p->genero,
            sizeof(p->genero),
            form->entry_genero,
            GENEROS);

        p->altura =
            gtk_spin_button_get_value(
                GTK_SPIN_BUTTON(form->entry_altura));

        p->peso =
            gtk_spin_button_get_value(
                GTK_SPIN_BUTTON(form->entry_peso));

        copiar_texto(
            p->data_nascimento,
            sizeof(p->data_nascimento),
            form->entry_data_nascimento);

        copiar_texto(
            p->imagem,
            sizeof(p->imagem),
            form->entry_imagem);

        // ------------------------------------------------------------
        // CRIME
        // ------------------------------------------------------------

        copiar_texto(
            p->crime,
            sizeof(p->crime),
            form->entry_crime);

        p->pena_anos =
            gtk_spin_button_get_value_as_int(
                GTK_SPIN_BUTTON(form->entry_pena));

        // ------------------------------------------------------------
        // SISTEMA PRISIONAL
        // ------------------------------------------------------------

        copiar_texto(
            p->cela,
            sizeof(p->cela),
            form->entry_cela);

        copiar_opcao(
            p->perigo,
            sizeof(p->perigo),
            form->entry_perigo,
            PERICULOSIDADES);

        copiar_opcao(
            p->comportamento,
            sizeof(p->comportamento),
            form->entry_comportamento,
            COMPORTAMENTOS);

        copiar_opcao(
            p->regime,
            sizeof(p->regime),
            form->entry_regime,
            REGIMES);

        copiar_texto(
            p->data_prisao,
            sizeof(p->data_prisao),
            form->entry_data_prisao);

        p->tempo_restante =
            calcular_tempo_restante(p->pena_anos, p->data_prisao);

        p->isolamento =
            gtk_check_button_get_active(
                GTK_CHECK_BUTTON(form->check_isolamento));

        p->risco_fuga =
            gtk_check_button_get_active(
                GTK_CHECK_BUTTON(form->check_risco_fuga));

        // ------------------------------------------------------------
        // saude
        // ------------------------------------------------------------

        copiar_texto(
            p->saude,
            sizeof(p->saude),
            form->entry_saude);

        copiar_doencas(
            p->doencas,
            sizeof(p->doencas),
            form);

        copiar_texto(
            p->psicologico,
            sizeof(p->psicologico),
            form->entry_psicologico);

        copiar_texto(
            p->medicacao,
            sizeof(p->medicacao),
            form->entry_medicacao);

        // ------------------------------------------------------------
        // SEGURANCA
        // ------------------------------------------------------------

        p->tentativas_fuga =
            gtk_spin_button_get_value_as_int(
                GTK_SPIN_BUTTON(form->entry_tentativas_fuga));

        copiar_texto(
            p->historico_violencia,
            sizeof(p->historico_violencia),
            form->entry_historico_violencia);

        copiar_texto(
            p->ultima_ocorrencia,
            sizeof(p->ultima_ocorrencia),
            form->entry_ultima_ocorrencia);

        copiar_texto(
            p->alerta,
            sizeof(p->alerta),
            form->entry_alerta);

        // ------------------------------------------------------------
        // ATUALIZA TEXTO DA LISTA
        // ------------------------------------------------------------

        atualizar_item_lista(form->item_editando);
        reordenar_item_editado(state, form->item_editando);

        salvar_arquivo(form->state->items);

        form->modo_edicao = FALSE;
        form->item_editando = NULL;

        GtkWindow *janela_principal =
            obter_janela_principal(form);

        gtk_window_destroy(
            GTK_WINDOW(form->window));

        if (janela_principal)
            gtk_window_present(janela_principal);

        return;
    }

    // ------------------------------------------------------------
    // MODO CRIACAO
    // ------------------------------------------------------------

    Pessoa *nova =
        g_malloc0(sizeof(Pessoa));

    // ------------------------------------------------------------
    // DADOS PESSOAIS
    // ------------------------------------------------------------

    copiar_texto(
        nova->nome,
        sizeof(nova->nome),
        form->entry_nome);

    copiar_texto(
        nova->apelido,
        sizeof(nova->apelido),
        form->entry_apelido);

    // serie (gerada automaticamente)
    nova->serie =
        form->serie_temp;

    nova->idade =
        calcular_idade(
            gtk_editable_get_text(GTK_EDITABLE(form->entry_data_nascimento)));

    copiar_texto(
        nova->nacionalidade,
        sizeof(nova->nacionalidade),
        form->entry_nacionalidade);

    // genero
    copiar_opcao(
        nova->genero,
        sizeof(nova->genero),
        form->entry_genero,
        GENEROS);

    nova->altura =
        gtk_spin_button_get_value(
            GTK_SPIN_BUTTON(form->entry_altura));

    nova->peso =
        gtk_spin_button_get_value(
            GTK_SPIN_BUTTON(form->entry_peso));

    // data nascimento
    copiar_texto(
        nova->data_nascimento,
        sizeof(nova->data_nascimento),
        form->entry_data_nascimento);

    copiar_texto(
        nova->imagem,
        sizeof(nova->imagem),
        form->entry_imagem);

    // ------------------------------------------------------------
    // CRIME
    // ------------------------------------------------------------

    copiar_texto(
        nova->crime,
        sizeof(nova->crime),
        form->entry_crime);

    nova->pena_anos =
        gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(form->entry_pena));

    // ------------------------------------------------------------
    // SISTEMA PRISIONAL
    // ------------------------------------------------------------

    copiar_texto(
        nova->cela,
        sizeof(nova->cela),
        form->entry_cela);

    copiar_opcao(
        nova->perigo,
        sizeof(nova->perigo),
        form->entry_perigo,
        PERICULOSIDADES);

    copiar_opcao(
        nova->comportamento,
        sizeof(nova->comportamento),
        form->entry_comportamento,
        COMPORTAMENTOS);

    copiar_opcao(
        nova->regime,
        sizeof(nova->regime),
        form->entry_regime,
        REGIMES);

    // data prisao
    copiar_texto(
        nova->data_prisao,
        sizeof(nova->data_prisao),
        form->entry_data_prisao);

    // tempo restante
    nova->tempo_restante =
        calcular_tempo_restante(nova->pena_anos, nova->data_prisao);

    nova->isolamento =
        gtk_check_button_get_active(
            GTK_CHECK_BUTTON(form->check_isolamento));

    // risco fuga
    nova->risco_fuga =
        gtk_check_button_get_active(
            GTK_CHECK_BUTTON(form->check_risco_fuga));

    // ------------------------------------------------------------
    // saude
    // ------------------------------------------------------------

    // saude
    copiar_texto(
        nova->saude,
        sizeof(nova->saude),
        form->entry_saude);

    copiar_doencas(
        nova->doencas,
        sizeof(nova->doencas),
        form);

    // psicologico
    copiar_texto(
        nova->psicologico,
        sizeof(nova->psicologico),
        form->entry_psicologico);

    // medicacao
    copiar_texto(
        nova->medicacao,
        sizeof(nova->medicacao),
        form->entry_medicacao);

    // ------------------------------------------------------------
    // SEGURANCA
    // ------------------------------------------------------------

    // tentativas fuga
    nova->tentativas_fuga =
        gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(form->entry_tentativas_fuga));

    // historico violencia
    copiar_texto(
        nova->historico_violencia,
        sizeof(nova->historico_violencia),
        form->entry_historico_violencia);

    // ultima ocorrencia
    copiar_texto(
        nova->ultima_ocorrencia,
        sizeof(nova->ultima_ocorrencia),
        form->entry_ultima_ocorrencia);

    copiar_texto(
        nova->alerta,
        sizeof(nova->alerta),
        form->entry_alerta);

    // ------------------------------------------------------------
    // CRIACAO DA LISTA
    // ------------------------------------------------------------


    ItemLista *item = criar_item_lista(state, nova);
    state->items = g_list_insert_sorted(state->items, item, comparar);
    sincronizar_ordem_lista(state);

    // ------------------------------------------------------------
    // FECHAR JANELA CORRETAMENTE
    // ------------------------------------------------------------

    // Salva o arquivo
    salvar_arquivo(state->items);

    // Limpa e esconde a janela para reutilizar no proximo cadastro.
    reset_form(form);
    gtk_widget_set_visible(form->window, FALSE);
    apresentar_janela_principal(form);
}
// ------------------------------------------------------------

static void cancelar_formulario(GtkWidget *widget, gpointer data)
{
    (void)widget;

    FormData *form = data;

    if (form->modo_edicao)
    {
        gtk_window_destroy(GTK_WINDOW(form->window));
        return;
    }

    reset_form(form);
    gtk_widget_set_visible(form->window, FALSE);
    apresentar_janela_principal(form);
}
// ------------------------------------------------------------

// ------------------------------------------------------------
// LIMPEZA DE MEMORIA
// ------------------------------------------------------------
static void on_window_destroy(GtkWidget *window, gpointer data)
{
    (void)window;

    FormData *form = data;

    if (form->state && form->state->form_adicionar == form)
    {
        form->state->form_adicionar = NULL;
    }

    g_free(form);
}
// ------------------------------------------------------------

static gboolean on_window_close_request(GtkWindow *window, gpointer data)
{
    FormData *form = data;

    if (!form->modo_edicao)
    {
        reset_form(form);
        gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
        apresentar_janela_principal(form);
        return TRUE;
    }

    return FALSE;
}
// ------------------------------------------------------------

// ------------------------------------------------------------
// RESETAR FORMULARIO
// ------------------------------------------------------------
static void reset_form(FormData *form)
{
    // ------------------------------------------------------------
    // DADOS PESSOAIS
    // ------------------------------------------------------------

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_nome), "");

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_apelido), "");

    gtk_label_set_text(
        GTK_LABEL(form->entry_idade),
        "Calculada pela data de nascimento");

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_nacionalidade), "");

    gtk_drop_down_set_selected(
        GTK_DROP_DOWN(form->entry_genero),
        0);

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(form->entry_altura), 0.0);

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(form->entry_peso), 0.0);

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_data_nascimento), "");

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_imagem), "");

    atualizar_preview_imagem(form);

    // ------------------------------------------------------------
    // CRIME
    // ------------------------------------------------------------

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_crime), "");

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(form->entry_pena), 0);

    // ------------------------------------------------------------
    // SISTEMA PRISIONAL
    // ------------------------------------------------------------

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_cela), "");

    gtk_drop_down_set_selected(
        GTK_DROP_DOWN(form->entry_perigo),
        0);

    gtk_drop_down_set_selected(
        GTK_DROP_DOWN(form->entry_comportamento),
        0);

    gtk_drop_down_set_selected(
        GTK_DROP_DOWN(form->entry_regime),
        0);

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_data_prisao), "");

    gtk_label_set_text(
        GTK_LABEL(form->entry_tempo_restante),
        "Calculado automaticamente");

    gtk_check_button_set_active(
        GTK_CHECK_BUTTON(form->check_isolamento),
        FALSE);

    gtk_check_button_set_active(
        GTK_CHECK_BUTTON(form->check_risco_fuga),
        FALSE);

    // ------------------------------------------------------------
    // saude
    // ------------------------------------------------------------

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_saude), "");

    limpar_doencas(form);

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_psicologico), "");

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_medicacao), "");

    // ------------------------------------------------------------
    // SEGURANCA
    // ------------------------------------------------------------

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(form->entry_tentativas_fuga), 0);

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_historico_violencia), "");

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_ultima_ocorrencia), "");

    gtk_editable_set_text(
        GTK_EDITABLE(form->entry_alerta), "");

    // ------------------------------------------------------------
    // serie
    // ------------------------------------------------------------

    gtk_label_set_text(
        GTK_LABEL(form->label_serie),
        "");

    form->modo_edicao = FALSE;
    form->item_editando = NULL;
}
// ------------------------------------------------------------

static void preparar_formulario_criacao(FormData *form)
{
    char buf[32];

    reset_form(form);

    form->modo_edicao = FALSE;
    form->item_editando = NULL;
    form->serie_temp = gerar_serie(form->state);

    snprintf(buf, sizeof(buf), "%06d", form->serie_temp);

    gtk_label_set_text(
        GTK_LABEL(form->label_serie),
        buf);
}
// ------------------------------------------------------------

static void reordenar_item_editado(AppState *state, ItemLista *item)
{
    if (!state || !item)
        return;

    state->items =
        g_list_remove(state->items, item);

    state->items =
        g_list_insert_sorted(state->items, item, comparar);

    sincronizar_ordem_lista(state);
}
// ------------------------------------------------------------

// ------------------------------------------------------------
// GERA SERIE UNICA
// ------------------------------------------------------------
static int gerar_serie(AppState *state)
{
    int serie;

    do
    {
        serie = 10000 + (rand() % 90000);

        gboolean existe = FALSE;

        for (GList *l = state->items; l != NULL; l = l->next)
        {
            ItemLista *it = l->data;
            if (it->pessoa && it->pessoa->serie == serie)
            {
                existe = TRUE;
                break;
            }
        }

        if (!existe)
            return serie;

    } while (1);
}
// ------------------------------------------------------------
