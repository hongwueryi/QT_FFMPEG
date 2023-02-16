/*
* ffmpeg -f dshow -list_devices true -i dummy                        //dshow支持的设备
* ffmpeg -h demuxer=dshow                                            //dshow支持的参数
* ffmpeg -f dshow -list_options true -i video="Integrated Camera"    //摄像头支持的参数
* ffmpeg -f dshow -video_size 640x480 -pixel_format yuyv422 -framerate 30 -i video="Integrated Camera" out.yuv
* ffplay -video_size 640x480 -pixel_format yuyv422 -framerate 30 out.yuv
*/

#include "video.h"
#include <QFile>
#include <Windows.h>
#include "pch.h"

// 格式名称
#define FMT_NAME "dshow"
// 设备名称
#define DEVICE_NAME "video=Integrated Camera"
// YUV文件名


#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

QFile v_file(V_FILENAME);

CRecordVideo::CRecordVideo(QObject* parent)
{
    //在线程结束时自动回收线程的内存
    connect(this, &CRecordVideo::finished, this, &CRecordVideo::deleteLater);
}

CRecordVideo::~CRecordVideo()
{
    ::FreeConsole();
}

void CRecordVideo::Init()
{
    // 初始化libavdevice并注册所有输入和输出设备
    avdevice_register_all();

    // 获取输入格式对象
    const AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt) 
    {
        printf("av_find_input_format error, input=%s\n", FMT_NAME);
        return;
    }

    // 格式上下文
    //AVFormatContext* ctx = nullptr;

    // 传递给输入设备的参数
    AVDictionary* options = nullptr;
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "framerate", "30", 0);

    // 打开输入设备
    int ret = avformat_open_input(&m_ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        printf("avformat_open_input error, msg=%s\n", errbuf);
        return;
    }

    // 打开文件
    if (!v_file.open(QFile::WriteOnly))
    {
        printf("file open error, file=%s\n", V_FILENAME);
        // 关闭输入设备
        avformat_close_input(&m_ctx);
        return;
    }

    this->start();
}

void CRecordVideo::UnInit()
{
    // 释放资源
    av_packet_free(&m_pkt);

    // 关闭文件
    v_file.close();

    // 关闭设备
    avformat_close_input(&m_ctx);
}

void CRecordVideo::run()
{
    // 计算每一帧的大小
    AVCodecParameters* params = m_ctx->streams[0]->codecpar;
    int imageSize = av_image_get_buffer_size((AVPixelFormat)params->format, params->width, params->height, 1);

    // 数据包
    m_pkt = av_packet_alloc();
    while (!isInterruptionRequested())
    {
        // 不断采集数据
        int ret = av_read_frame(m_ctx, m_pkt);

        if (ret == 0) 
        { // 读取成功
            // 将数据写入文件
            v_file.write((const char*)m_pkt->data, imageSize);
            /*
             这里要使用imageSize，而不是pkt->size。
             pkt->size有可能比imageSize大（比如在Mac平台），
             使用pkt->size会导致写入一些多余数据到YUV文件中，
             进而导致YUV内容无法正常播放
            */

            // 释放资源
            av_packet_unref(m_pkt);
        }
        else if (ret == AVERROR(EAGAIN)) 
        { // 资源临时不可用
            continue;
        }
        else
        { // 其他错误
            ERROR_BUF(ret);
            printf("av_read_frame error, msg=%s\n", errbuf);
            break;
        }
    }
}