#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

static cv::Mat g_frame_bgr;
static bool g_capturar = false;
static bool g_cancelar = false;

static cv::Mat preparar_frame(const cv::Mat &frame)
{
    cv::Mat espelhado;
    cv::flip(frame, espelhado, 1);

    cv::Mat ajustado;
    espelhado.convertTo(ajustado, -1, 1.08, 8);

    cv::Mat suavizado;
    cv::GaussianBlur(ajustado, suavizado, cv::Size(0, 0), 1.0);

    cv::Mat nitido;
    cv::addWeighted(ajustado, 1.35, suavizado, -0.35, 0, nitido);

    return nitido;
}

static LRESULT CALLBACK janela_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE)
        {
            g_cancelar = true;
            DestroyWindow(hwnd);
            return 0;
        }

        if (wparam == VK_SPACE || wparam == VK_RETURN)
        {
            g_capturar = true;
            return 0;
        }

        break;

    case WM_CLOSE:
        g_cancelar = true;
        DestroyWindow(hwnd);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT area;
        GetClientRect(hwnd, &area);

        int largura_cliente = area.right - area.left;
        int altura_cliente = area.bottom - area.top;

        HDC memoria = CreateCompatibleDC(hdc);
        HBITMAP bitmap = CreateCompatibleBitmap(hdc, largura_cliente, altura_cliente);
        HGDIOBJ bitmap_antigo = SelectObject(memoria, bitmap);

        HBRUSH fundo = CreateSolidBrush(RGB(245, 247, 250));
        FillRect(memoria, &area, fundo);
        DeleteObject(fundo);

        if (!g_frame_bgr.empty())
        {
            BITMAPINFO info = {};
            info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            info.bmiHeader.biWidth = g_frame_bgr.cols;
            info.bmiHeader.biHeight = -g_frame_bgr.rows;
            info.bmiHeader.biPlanes = 1;
            info.bmiHeader.biBitCount = 24;
            info.bmiHeader.biCompression = BI_RGB;

            int largura = largura_cliente;
            int altura = altura_cliente - 48;

            StretchDIBits(
                memoria,
                0,
                0,
                largura,
                altura,
                0,
                0,
                g_frame_bgr.cols,
                g_frame_bgr.rows,
                g_frame_bgr.data,
                &info,
                DIB_RGB_COLORS,
                SRCCOPY);

            RECT barra = {0, altura, area.right, area.bottom};
            HBRUSH barra_fundo = CreateSolidBrush(RGB(37, 42, 46));
            FillRect(memoria, &barra, barra_fundo);
            DeleteObject(barra_fundo);

            SetBkMode(memoria, TRANSPARENT);
            SetTextColor(memoria, RGB(255, 255, 255));
            DrawTextA(
                memoria,
                "Espaco/Enter: capturar     Esc: cancelar",
                -1,
                &barra,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        else
        {
            SetBkMode(memoria, TRANSPARENT);
            SetTextColor(memoria, RGB(50, 56, 62));
            DrawTextA(
                memoria,
                "Abrindo webcam...",
                -1,
                &area,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        BitBlt(hdc, 0, 0, largura_cliente, altura_cliente, memoria, 0, 0, SRCCOPY);
        SelectObject(memoria, bitmap_antigo);
        DeleteObject(bitmap);
        DeleteDC(memoria);

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

static bool abrir_camera(cv::VideoCapture &camera, std::string &backend_usado)
{
    struct Backend
    {
        int id;
        const char *nome;
    };

    const Backend backends[] = {
        {cv::CAP_DSHOW, "DirectShow"},
        {cv::CAP_MSMF, "Media Foundation"},
        {cv::CAP_ANY, "Automatico"}};

    for (const Backend &backend : backends)
    {
        camera.open(0, backend.id);

        if (camera.isOpened())
        {
            backend_usado = backend.nome;
            return true;
        }

        camera.release();
    }

    return false;
}

int main()
{
    std::filesystem::create_directories("logs");
    std::ofstream log("logs/camera_debug.log", std::ios::out | std::ios::trunc);

    try
    {
        log << "Iniciando captura OpenCV + Win32" << std::endl;

        cv::VideoCapture camera;
        std::string backend_usado;

        if (!abrir_camera(camera, backend_usado))
        {
            log << "Falha ao abrir webcam" << std::endl;
            std::cerr << "Nao consegui abrir a webcam. Verifique se ela nao esta em uso e se o Windows permitiu acesso a camera." << std::endl;
            return 2;
        }

        log << "Webcam aberta com backend: " << backend_usado << std::endl;

        camera.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        camera.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

        HINSTANCE instancia = GetModuleHandle(NULL);

        WNDCLASSA classe = {};
        classe.lpfnWndProc = janela_proc;
        classe.hInstance = instancia;
        classe.lpszClassName = "CameraCaptureWindow";
        classe.hCursor = LoadCursor(NULL, IDC_ARROW);

        RegisterClassA(&classe);

        HWND janela = CreateWindowExA(
            0,
            classe.lpszClassName,
            "Webcam - capturar foto",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            920,
            640,
            NULL,
            NULL,
            instancia,
            NULL);

        if (!janela)
        {
            std::cerr << "Nao consegui criar a janela da webcam." << std::endl;
            return 5;
        }

        MSG msg;

        while (IsWindow(janela) && !g_capturar && !g_cancelar)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            cv::Mat frame_original;
            camera >> frame_original;

            if (frame_original.empty())
            {
                std::cerr << "A webcam abriu usando " << backend_usado << ", mas nao entregou imagem." << std::endl;
                return 3;
            }

            g_frame_bgr = preparar_frame(frame_original);

            InvalidateRect(janela, NULL, FALSE);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        if (g_cancelar)
        {
            std::cerr << "Captura cancelada." << std::endl;
            return 1;
        }

        std::filesystem::create_directories("fotos");

        auto agora = std::chrono::system_clock::now().time_since_epoch();
        auto milissegundos = std::chrono::duration_cast<std::chrono::milliseconds>(agora).count();

        std::filesystem::path caminho =
            std::filesystem::absolute(
                std::filesystem::path("fotos") /
                ("detenta_" + std::to_string(milissegundos) + ".jpg"));

        if (!cv::imwrite(caminho.string(), g_frame_bgr))
        {
            std::cerr << "Nao consegui salvar a foto." << std::endl;
            return 4;
        }

        log << "Foto salva em: " << caminho.string() << std::endl;
        std::cout << caminho.string() << std::endl;

        if (IsWindow(janela))
            DestroyWindow(janela);

        return 0;
    }
    catch (const cv::Exception &erro)
    {
        log << "Excecao OpenCV: " << erro.what() << std::endl;
        std::cerr << "Erro do OpenCV: " << erro.what() << std::endl;
        return 6;
    }
    catch (const std::exception &erro)
    {
        log << "Excecao std: " << erro.what() << std::endl;
        std::cerr << "Erro ao capturar foto: " << erro.what() << std::endl;
        return 7;
    }
}
