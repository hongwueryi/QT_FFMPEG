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
    // 窗口
    SDL_Window* window = nullptr;

    // 渲染上下文
    SDL_Renderer* renderer = nullptr;

    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture* texture = nullptr;

    // 文件
    QFile file(V_FILENAME);

    CHECK(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    // 创建窗口
    window = SDL_CreateWindow(
        // 窗口标题
        "SDL显示YUV图片",
        // 窗口X（未定义）
        SDL_WINDOWPOS_UNDEFINED,
        // 窗口Y（未定义）
        SDL_WINDOWPOS_UNDEFINED,
        // 窗口宽度（跟图片宽度一样）
        IMG_W,
        // 窗口高度（跟图片高度一样）
        IMG_H,
        // 显示窗口
        SDL_WINDOW_SHOWN
    );
    CHECK(!window, SDL_CreateWindow);

    // 创建渲染上下文（默认的渲染目标是window）
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { // 说明开启硬件加速失败
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    CHECK(!renderer, SDL_CreateRenderer);

    // 创建纹理
    texture = SDL_CreateTexture(renderer,
        PIXEL_FORMAT,
        SDL_TEXTUREACCESS_STREAMING,
        IMG_W, IMG_H);
    CHECK(!texture, SDL_CreateTexture);

    // 打开文件
    if (!file.open(QFile::ReadOnly)) {
        std::cout << "file open error" << FILENAME << std::endl;
        goto end;
    }

    // 将YUV的像素数据填充到texture
    CHECK(SDL_UpdateTexture(texture, nullptr, file.readAll().data(), IMG_W),
        SDL_UpdateTexture);

    // 设置绘制颜色（画笔颜色）
    CHECK(SDL_SetRenderDrawColor(renderer,
        0, 0, 0, SDL_ALPHA_OPAQUE),
        SDL_SetRenderDrawColor);

    // 用绘制颜色（画笔颜色）清除渲染目标
    CHECK(SDL_RenderClear(renderer),
        SDL_RenderClear);

    // 拷贝纹理数据到渲染目标（默认是window）
    CHECK(SDL_RenderCopy(renderer, texture, nullptr, nullptr),
        SDL_RenderCopy);

    // 更新所有的渲染操作到屏幕上
    SDL_RenderPresent(renderer);

    // 延迟3秒退出
    SDL_Delay(3000);

end:
    file.close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
}