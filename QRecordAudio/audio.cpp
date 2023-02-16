/*
ffmpeg [-hide_banner] -devices
//查看可用dshow设备
ffmpeg [-hide_banner] -list_devices true -f dshow -i ''
ffmpeg -f dshow -list_options true -i audio="@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\\wave_{BF29D2FB-3216-401B-BCE0-EB186EF9ABE6}"
//录音
ffmpeg -f dshow -i audio="@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\\wave_{BF29D2FB-3216-401B-BCE0-EB186EF9ABE6}" out.wav

查询支持的播放格式
ffmpeg -formats | findstr PCM
//播放pcm无损音质
ffplay -ar 44100 -ac 2 -f s16le out.pcm

*/
#include <QFile>
#include <stdio.h>
#include <Windows.h>
#include "audio.h"
#include "pch.h"


#define FMT_NAME "dshow"
#define DEVICE_NAME "audio=@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\\wave_{BF29D2FB-3216-401B-BCE0-EB186EF9ABE6}"
QFile g_file(FILENAME);

CRecordAudio::CRecordAudio(QObject* parent)
{
    //在线程结束时自动回收线程的内存
    connect(this, &CRecordAudio::finished, this, &CRecordAudio::deleteLater);
    
}

CRecordAudio::~CRecordAudio()
{
    ::FreeConsole();
}

void CRecordAudio::run()
{
    // 数据包
    pkt = av_packet_alloc();
    while (!isInterruptionRequested())
    {
        // 从设备中采集数据，返回值为0，代表采集数据成功
        int iRet = av_read_frame(m_ctx, pkt);
        if (iRet == 0)
        {
            // 读取成功
            g_file.write((const char*)pkt->data, pkt->size);
            av_packet_unref(pkt);
        }
        else if (iRet == AVERROR(EAGAIN))
        {
            // 资源临时不可用        continue; 
        }
        else
        {
            char buffer[1024] = { 0 };
            av_strerror(iRet, buffer, 1023);
            printf("av_read_frame, err:%s", buffer);
            break;
        }
    }
}

void CRecordAudio::Init()
{
    // WriteOnly：只写模式。如果文件不存在，就创建文件；如果文件存在，就删除文件内容
    if (!g_file.open(QFile::WriteOnly))
    {
        printf("open out file failed, errno=%d\n", GetLastError());
        return;
    }

    //初始化libavdevice并注册所有输入和输出设备
    avdevice_register_all();
    const AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt)
    {
        printf("can not find input format");
        return;
    }

    int iRet = avformat_open_input(&m_ctx, DEVICE_NAME, fmt, NULL);
    if (iRet < 0)
    {
        char buffer[1024] = { 0 };
        av_strerror(iRet, buffer, 1023);
        printf("open device failed, err:%s", buffer);
        return;
    }

    this->start();
}

void CRecordAudio::UnInit()
{
    requestInterruption();
    Sleep(1000);
    g_file.close();
    // 释放资源
    av_packet_free(&pkt);
    // 关闭设备
    avformat_close_input(&m_ctx);
}

// 从AVFormatContext中获取录音设备的相关参数
void CRecordAudio::showSpec()
{    
    // 获取输入流    
    AVStream *stream = m_ctx->streams[0];
    // 获取音频参数    
    AVCodecParameters *params = stream->codecpar;
    // 声道数
    printf("channel number=%d\n", params->channels);
    // 采样率    
    printf("sample_rate=%d\n", params->sample_rate);
    // 采样格式    
    printf("format=%d\n", params->format);
    // 每一个样本的一个声道占用多少个字节    
    printf("bytes_per_sample=%d\n", av_get_bytes_per_sample((AVSampleFormat) params->format));
    // 编码ID（可以看出采样格式）    
    printf("codec_id=%d\n", (int)params->codec_id);
    // 每一个样本的一个声道占用多少位（这个函数需要用到avcodec库）    
    printf("bits_per_sample=%d\n", av_get_bits_per_sample(params->codec_id));
}


