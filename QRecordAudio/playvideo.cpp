#include "playvideo.h"
#include <iostream>
#include "pch.h"
#include <QFile>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define CHECK(judge, func) \
    if (judge) { \
        std::cout << #func << "error" << SDL_GetError() << std::endl; \
        goto end; \
    }

#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 640
#define IMG_H 480

CVideoPlayer::CVideoPlayer(QObject * parent)
{

}

CVideoPlayer::~CVideoPlayer()
{

}

void CVideoPlayer::play()
{
    this->start();
}

int32_t shell_create_process(const std::string& lpDirectory, const std::string& lpParameters, const std::string& lpFile, bool blWait, bool blShow)
{
    DWORD dwExitCode = 0;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = blShow ? SW_SHOW : SW_HIDE;

    std::string cmdline = "cmd.exe /c " + lpFile + " " + lpParameters;

    if (CreateProcessA(NULL, (LPSTR)cmdline.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, lpDirectory.c_str(), &si, &pi))
    {
        if (blWait)
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &dwExitCode);
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else
    {
        return -1;
    }

    return dwExitCode;
}

void CVideoPlayer::run()
{
    const char* path = "E:\\Dten\\other demo\\Qtest\\QT_FFMPEG\\bin";
    /*char path[MAX_PATH] = { 0 };
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    PathRemoveFileSpecA(path);*/
    char buffer[512] = { 0 };
    sprintf_s(buffer, "-video_size 640x480 -pixel_format yuyv422 -framerate 30 \"%s\\my.yuv\"", path);
    shell_create_process(path, buffer, "ffplay.exe", true, true);
#if 0
    // ����
    SDL_Window* window = nullptr;

    // ��Ⱦ������
    SDL_Renderer* renderer = nullptr;

    // ����ֱ�Ӹ��ض�����������ص��������ݣ�
    SDL_Texture* texture = nullptr;

    // �ļ�
    QFile file(V_FILENAME);

    CHECK(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    // ��������
    window = SDL_CreateWindow(
        // ���ڱ���
        "SDL��ʾYUVͼƬ",
        // ����X��δ���壩
        SDL_WINDOWPOS_UNDEFINED,
        // ����Y��δ���壩
        SDL_WINDOWPOS_UNDEFINED,
        // ���ڿ�ȣ���ͼƬ���һ����
        IMG_W,
        // ���ڸ߶ȣ���ͼƬ�߶�һ����
        IMG_H,
        // ��ʾ����
        SDL_WINDOW_SHOWN
    );
    CHECK(!window, SDL_CreateWindow);

    // ������Ⱦ�����ģ�Ĭ�ϵ���ȾĿ����window��
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { // ˵������Ӳ������ʧ��
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    CHECK(!renderer, SDL_CreateRenderer);

    // ��������
    texture = SDL_CreateTexture(renderer,
        PIXEL_FORMAT,
        SDL_TEXTUREACCESS_STREAMING,
        IMG_W, IMG_H);
    CHECK(!texture, SDL_CreateTexture);

    // ���ļ�
    if (!file.open(QFile::ReadOnly)) {
        std::cout << "file open error" << FILENAME << std::endl;
        goto end;
    }

    // ��YUV������������䵽texture
    CHECK(SDL_UpdateTexture(texture, nullptr, file.readAll().data(), IMG_W),
        SDL_UpdateTexture);

    // ���û�����ɫ��������ɫ��
    CHECK(SDL_SetRenderDrawColor(renderer,
        0, 0, 0, SDL_ALPHA_OPAQUE),
        SDL_SetRenderDrawColor);

    // �û�����ɫ��������ɫ�������ȾĿ��
    CHECK(SDL_RenderClear(renderer),
        SDL_RenderClear);

    // �����������ݵ���ȾĿ�꣨Ĭ����window��
    CHECK(SDL_RenderCopy(renderer, texture, nullptr, nullptr),
        SDL_RenderCopy);

    // �������е���Ⱦ��������Ļ��
    SDL_RenderPresent(renderer);

    // �ӳ�3���˳�
    SDL_Delay(3000);

end:
    file.close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
}