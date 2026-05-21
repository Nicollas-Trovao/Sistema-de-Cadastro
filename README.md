# Penitenciaria Santa Olga

Sistema desktop em C/GTK4 para cadastro, consulta, edicao e exclusao de registros de detentas. O projeto salva os dados em CSV e possui um capturador auxiliar de webcam em C++/OpenCV para registrar fotos.

## Funcionalidades

- Cadastro e edicao de detentas.
- Listagem lateral com busca.
- Painel de detalhes do registro selecionado.
- Persistencia em `data/detentos.csv`.
- Backup automatico em `data/detentos_backup.csv`.
- Selecao manual de imagem ou captura pela webcam.
- Interface estilizada com CSS GTK.

## Estrutura

```text
.
|-- assets/      # Arquivos visuais, como o CSS da interface
|-- bin/         # Executaveis gerados pela compilacao
|-- data/        # Arquivos CSV usados pela aplicacao
|-- fotos/       # Fotos cadastradas/capturadas
|-- include/     # Headers do projeto
|-- logs/        # Logs de execucao
|-- src/         # Codigo-fonte C e C++
|-- Makefile     # Build do projeto
```

## Dependencias

O projeto foi preparado para Windows com MSYS2/UCRT64.

Instale ou mantenha disponivel:

- GCC
- G++
- Make
- pkg-config
- GTK4
- OpenCV

No MSYS2, os pacotes principais ficam no ambiente `ucrt64`.

## Como Compilar

No PowerShell, a partir da raiz do projeto:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
C:\msys64\usr\bin\make.exe all
```

Ou pelo VS Code, use a task padrao **build projeto**.

## Como Executar

Depois da compilacao:

```powershell
.\bin\main.exe
```

O executavel principal deve ser iniciado a partir da raiz do projeto, pois a aplicacao usa caminhos relativos para `assets/`, `data/`, `fotos/` e `logs/`.

## Limpeza

Para remover os executaveis gerados:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
C:\msys64\usr\bin\make.exe clean
```

