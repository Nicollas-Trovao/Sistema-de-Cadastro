/*
 * arquivo.c
 * Responsavel pela persistencia dos registros em detentos.csv.
 * Converte os dados da lista para CSV e recria os itens ao carregar.
 */

#include "arquivo.h"

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lista.h"

#define CSV_ARQUIVO "data/detentos.csv"
#define CSV_BACKUP "data/detentos_backup.csv"
#define CSV_TEMPORARIO "data/detentos.tmp"
#define CSV_CAMPOS 28
#define CSV_CAMPOS_ANTIGO 27
#define CSV_TAM_CAMPO 1024

static const char *CSV_CABECALHO =
    "nome,apelido,serie,idade,nacionalidade,genero,altura,peso,data_nascimento,"
    "imagem,crime,pena_anos,cela,perigo,comportamento,regime,data_prisao,"
    "tempo_restante,isolamento,risco_fuga,saude,doencas,psicologico,medicacao,"
    "tentativas_fuga,historico_violencia,ultima_ocorrencia,alerta";

static void criar_backup_csv(void)
{
    FILE *origem = fopen(CSV_ARQUIVO, "rb");

    if (!origem)
        return;

    FILE *backup = fopen(CSV_BACKUP, "wb");

    if (!backup)
    {
        fclose(origem);
        return;
    }

    char buffer[4096];
    size_t lidos;

    while ((lidos = fread(buffer, 1, sizeof(buffer), origem)) > 0)
    {
        fwrite(buffer, 1, lidos, backup);
    }

    fclose(backup);
    fclose(origem);
}

static void escrever_campo_csv(FILE *f, const char *valor)
{
    fputc('"', f);

    for (const char *c = valor ? valor : ""; *c != '\0'; c++)
    {
        if (*c == '"')
            fputc('"', f);

        fputc(*c, f);
    }

    fputc('"', f);
}

static void escrever_inteiro_csv(FILE *f, int valor)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", valor);
    escrever_campo_csv(f, buffer);
}

static void escrever_float_csv(FILE *f, float valor)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", valor);
    escrever_campo_csv(f, buffer);
}

static void escrever_separador_ou_fim(FILE *f, int indice)
{
    fputc(indice == CSV_CAMPOS - 1 ? '\n' : ',', f);
}

void salvar_arquivo(GList *items)
{
    criar_backup_csv();

    FILE *f =
        fopen(CSV_TEMPORARIO, "w");

    if (!f)
        return;

    fprintf(f, "%s\n", CSV_CABECALHO);

    for (
        GList *l = items;
        l != NULL;
        l = l->next)
    {
        ItemLista *item =
            l->data;

        Pessoa *p =
            item->pessoa;

        int i = 0;

        escrever_campo_csv(f, p->nome);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->apelido);
        escrever_separador_ou_fim(f, i++);
        escrever_inteiro_csv(f, p->serie);
        escrever_separador_ou_fim(f, i++);
        escrever_inteiro_csv(f, p->idade);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->nacionalidade);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->genero);
        escrever_separador_ou_fim(f, i++);
        escrever_float_csv(f, p->altura);
        escrever_separador_ou_fim(f, i++);
        escrever_float_csv(f, p->peso);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->data_nascimento);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->imagem);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->crime);
        escrever_separador_ou_fim(f, i++);
        escrever_inteiro_csv(f, p->pena_anos);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->cela);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->perigo);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->comportamento);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->regime);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->data_prisao);
        escrever_separador_ou_fim(f, i++);
        escrever_inteiro_csv(f, p->tempo_restante);
        escrever_separador_ou_fim(f, i++);
        escrever_inteiro_csv(f, p->isolamento);
        escrever_separador_ou_fim(f, i++);
        escrever_inteiro_csv(f, p->risco_fuga);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->saude);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->doencas);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->psicologico);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->medicacao);
        escrever_separador_ou_fim(f, i++);
        escrever_inteiro_csv(f, p->tentativas_fuga);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->historico_violencia);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->ultima_ocorrencia);
        escrever_separador_ou_fim(f, i++);
        escrever_campo_csv(f, p->alerta);
        escrever_separador_ou_fim(f, i++);
    }

    if (ferror(f) || fclose(f) != 0)
    {
        g_warning("Nao foi possivel salvar todos os dados no arquivo temporario.");
        g_remove(CSV_TEMPORARIO);
        return;
    }

    if (g_rename(CSV_TEMPORARIO, CSV_ARQUIVO) != 0)
    {
        g_remove(CSV_ARQUIVO);

        if (g_rename(CSV_TEMPORARIO, CSV_ARQUIVO) != 0)
        {
            g_warning("Nao foi possivel substituir %s pelo arquivo temporario.", CSV_ARQUIVO);
            g_remove(CSV_TEMPORARIO);
        }
    }
}

static void copiar_para_campo(char destino[CSV_TAM_CAMPO], size_t *pos, char c)
{
    if (*pos < CSV_TAM_CAMPO - 1)
    {
        destino[*pos] = c;
        (*pos)++;
    }
}

static int ler_linha_csv(const char *linha, char campos[CSV_CAMPOS][CSV_TAM_CAMPO])
{
    int campo = 0;
    size_t pos = 0;
    gboolean entre_aspas = FALSE;

    memset(campos, 0, CSV_CAMPOS * CSV_TAM_CAMPO);

    for (const char *c = linha; *c != '\0'; c++)
    {
        if (*c == '\r' || *c == '\n')
            break;

        if (entre_aspas)
        {
            if (*c == '"')
            {
                if (*(c + 1) == '"')
                {
                    copiar_para_campo(campos[campo], &pos, '"');
                    c++;
                }
                else
                {
                    entre_aspas = FALSE;
                }
            }
            else
            {
                copiar_para_campo(campos[campo], &pos, *c);
            }

            continue;
        }

        if (*c == '"')
        {
            entre_aspas = TRUE;
            continue;
        }

        if (*c == ',')
        {
            campos[campo][pos] = '\0';
            campo++;
            pos = 0;

            if (campo >= CSV_CAMPOS)
                return -1;

            continue;
        }

        copiar_para_campo(campos[campo], &pos, *c);
    }

    campos[campo][pos] = '\0';

    return campo + 1;
}

static gboolean linha_eh_cabecalho(char campos[CSV_CAMPOS][CSV_TAM_CAMPO])
{
    return strcmp(campos[0], "nome") == 0 &&
           strcmp(campos[1], "apelido") == 0 &&
           strcmp(campos[2], "serie") == 0;
}

static void copiar_campo_texto(char *destino, size_t tamanho, const char *origem)
{
    snprintf(destino, tamanho, "%s", origem && origem[0] != '\0' ? origem : "-");
}

void carregar_arquivo(AppState *state)
{
    FILE *f = fopen(CSV_ARQUIVO, "r");
    if (!f)
        return;

    char linha[8192];

    while (fgets(linha, sizeof(linha), f))
    {
        char campos[CSV_CAMPOS][CSV_TAM_CAMPO];
        int total_campos = ler_linha_csv(linha, campos);

        if (total_campos != CSV_CAMPOS && total_campos != CSV_CAMPOS_ANTIGO)
        {
            g_warning("Linha ignorada no CSV: esperado %d ou %d campos, recebido %d.",
                      CSV_CAMPOS, CSV_CAMPOS_ANTIGO, total_campos);
            continue;
        }

        if (linha_eh_cabecalho(campos))
            continue;

        Pessoa *p = g_malloc0(sizeof(Pessoa));

        copiar_campo_texto(p->nome, sizeof(p->nome), campos[0]);
        copiar_campo_texto(p->apelido, sizeof(p->apelido), campos[1]);
        p->serie = atoi(campos[2]);
        p->idade = atoi(campos[3]);
        copiar_campo_texto(p->nacionalidade, sizeof(p->nacionalidade), campos[4]);
        copiar_campo_texto(p->genero, sizeof(p->genero), campos[5]);
        p->altura = atof(campos[6]);
        p->peso = atof(campos[7]);
        copiar_campo_texto(p->data_nascimento, sizeof(p->data_nascimento), campos[8]);
        int offset = total_campos == CSV_CAMPOS ? 1 : 0;
        copiar_campo_texto(p->imagem, sizeof(p->imagem), offset ? campos[9] : "");
        copiar_campo_texto(p->crime, sizeof(p->crime), campos[9 + offset]);
        p->pena_anos = atoi(campos[10 + offset]);
        copiar_campo_texto(p->cela, sizeof(p->cela), campos[11 + offset]);
        copiar_campo_texto(p->perigo, sizeof(p->perigo), campos[12 + offset]);
        copiar_campo_texto(p->comportamento, sizeof(p->comportamento), campos[13 + offset]);
        copiar_campo_texto(p->regime, sizeof(p->regime), campos[14 + offset]);
        copiar_campo_texto(p->data_prisao, sizeof(p->data_prisao), campos[15 + offset]);
        p->tempo_restante = atoi(campos[16 + offset]);
        p->isolamento = atoi(campos[17 + offset]);
        p->risco_fuga = atoi(campos[18 + offset]);
        copiar_campo_texto(p->saude, sizeof(p->saude), campos[19 + offset]);
        copiar_campo_texto(p->doencas, sizeof(p->doencas), campos[20 + offset]);
        copiar_campo_texto(p->psicologico, sizeof(p->psicologico), campos[21 + offset]);
        copiar_campo_texto(p->medicacao, sizeof(p->medicacao), campos[22 + offset]);
        p->tentativas_fuga = atoi(campos[23 + offset]);
        copiar_campo_texto(p->historico_violencia, sizeof(p->historico_violencia), campos[24 + offset]);
        copiar_campo_texto(p->ultima_ocorrencia, sizeof(p->ultima_ocorrencia), campos[25 + offset]);
        copiar_campo_texto(p->alerta, sizeof(p->alerta), campos[26 + offset]);


        ItemLista *item = criar_item_lista(state, p);

        state->items = g_list_insert_sorted(
            state->items,
            item,
            comparar);
    }

    fclose(f);
}
