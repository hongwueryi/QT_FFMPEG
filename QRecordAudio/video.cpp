/*
* ffmpeg -f dshow -list_devices true -i dummy                        //dshow֧�ֵ��豸
* ffmpeg -h demuxer=dshow                                            //dshow֧�ֵĲ���
* ffmpeg -f dshow -list_options true -i video="Integrated Camera"    //����ͷ֧�ֵĲ���
* ffmpeg -f dshow -video_size 640x480 -pixel_format yuyv422 -framerate 30 -i video="Integrated Camera" out.yuv
* ffplay -video_size 640x480 -pixel_format yuyv422 -framerate 30 out.yuv
*/

#include "video.h"
#include <QFile>
#include <Windows.h>
#include "pch.h"

// ��ʽ����
#define FMT_NAME "dshow"
// �豸����
#define DEVICE_NAME "video=Integrated Camera"
// YUV�ļ���


#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

QFile v_file(V_FILENAME);

CRecordVideo::CRecordVideo(QObject* parent)
{
    //���߳̽���ʱ�Զ������̵߳��ڴ�
    connect(this, &CRecordVideo::finished, this, &CRecordVideo::deleteLater);
}

CRecordVideo::~CRecordVideo()
{
    ::FreeConsole();
}

void CRecordVideo::Init()
{
    // ��ʼ��libavdevice��ע���������������豸
    avdevice_register_all();

    // ��ȡ�����ʽ����
    const AVInputFormat* fmt = av_find_input_format(FMT_NAME);
    if (!fmt) 
    {
        printf("av_find_input_format error, input=%s\n", FMT_NAME);
        return;
    }

    // ��ʽ������
    //AVFormatContext* ctx = nullptr;

    // ���ݸ������豸�Ĳ���
    AVDictionary* options = nullptr;
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "framerate", "30", 0);

    // �������豸
    int ret = avformat_open_input(&m_ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0)
    {
        ERROR_BUF(ret);
        printf("avformat_open_input error, msg=%s\n", errbuf);
        return;
    }

    // ���ļ�
    if (!v_file.open(QFile::WriteOnly))
    {
        printf("file open error, file=%s\n", V_FILENAME);
        // �ر������豸
        avformat_close_input(&m_ctx);
        return;
    }

    this->start();
}

void CRecordVideo::UnInit()
{
    // �ͷ���Դ
    av_packet_free(&m_pkt);

    // �ر��ļ�
    v_file.close();

    // �ر��豸
    avformat_close_input(&m_ctx);
}

void CRecordVideo::run()
{
    // ����ÿһ֡�Ĵ�С
    AVCodecParameters* params = m_ctx->streams[0]->codecpar;
    int imageSize = av_image_get_buffer_size((AVPixelFormat)params->format, params->width, params->height, 1);

    // ���ݰ�
    m_pkt = av_packet_alloc();
    while (!isInterruptionRequested())
    {
        // ���ϲɼ�����
        int ret = av_read_frame(m_ctx, m_pkt);

        if (ret == 0) 
        { // ��ȡ�ɹ�
            // ������д���ļ�
            v_file.write((const char*)m_pkt->data, imageSize);
            /*
             ����Ҫʹ��imageSize��������pkt->size��
             pkt->size�п��ܱ�imageSize�󣨱�����Macƽ̨����
             ʹ��pkt->size�ᵼ��д��һЩ�������ݵ�YUV�ļ��У�
             ��������YUV�����޷���������
            */

            // �ͷ���Դ
            av_packet_unref(m_pkt);
        }
        else if (ret == AVERROR(EAGAIN)) 
        { // ��Դ��ʱ������
            continue;
        }
        else
        { // ��������
            ERROR_BUF(ret);
            printf("av_read_frame error, msg=%s\n", errbuf);
            break;
        }
    }
}