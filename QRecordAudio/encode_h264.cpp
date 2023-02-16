/*
*ffmpeg -s 640x480 -pix_fmt yuv420p -i my.yuv -c:v libx264 my.h264
# -c:v libx264��ָ��ʹ��libx264��Ϊ������
*/
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include "encode_h264.h"
#include "pch.h"
#include <QFile>
#include <iostream>
#include <Windows.h>


#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

// �ļ�
QFile inFile(V_FILENAME);
QFile outFile(H264_FILENAME);
VideoEncodeSpec in = { V_FILENAME, 640, 480, AV_PIX_FMT_YUV420P, 30 };

CEncode::CEncode(QObject* parent)
{

}

CEncode::~CEncode()
{

}

// ���ظ�������;�����˴���
// ����0����������������
static int encode(AVCodecContext* ctx,
    AVFrame* frame,
    AVPacket* pkt,
    QFile& outFile) {
    // �������ݵ�������
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        ERROR_BUF(ret);
        std::cout << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    // ���ϴӱ�������ȡ������������
    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // ������ȡ���ݵ�frame��Ȼ���͵�������
            return 0;
        }
        else if (ret < 0) { // ��������
            return ret;
        }

        // �ɹ��ӱ������õ�����������
        // ������������д���ļ�
        outFile.write((char*)pkt->data, pkt->size);

        // �ͷ�pkt�ڲ�����Դ
        av_packet_unref(pkt);
    }
}

void CEncode::h264Encode(VideoEncodeSpec& in, const char* outFilename)
{
    // һ֡ͼƬ�Ĵ�С
    int imgSize = av_image_get_buffer_size(in.pixFmt, in.width, in.height, 1);
    // ���ؽ��
    int ret = 0;
    // ������
    //AVCodec* codec = nullptr;
    // ����������
    AVCodecContext* ctx = nullptr;
    // ��ű���ǰ�����ݣ�yuv��
    AVFrame* frame = nullptr;
    // ��ű��������ݣ�h264��
    AVPacket* pkt = nullptr;

    // ��ȡ������
    const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        std::cout << "encoder not found";
        return;
    }

    // ����������ݵĲ�����ʽ
   /* if (!check_pix_fmt(codec, in.pixFmt)) {
        std::cout << "unsupported pixel format"
            << av_get_pix_fmt_name(in.pixFmt);
        return;
    }*/

    // ��������������
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        std::cout << "avcodec_alloc_context3 error";
        return;
    }

    // ����yuv����
    ctx->width = in.width;
    ctx->height = in.height;
    ctx->pix_fmt = in.pixFmt;
    // ����֡�ʣ�1������ʾ��֡����in.fps��
    ctx->time_base = { 1, in.fps };

    // �򿪱�����
    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        ERROR_BUF(ret);
        std::cout << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // ����AVFrame
    frame = av_frame_alloc();
    if (!frame) {
        std::cout << "av_frame_alloc error";
        goto end;
    }
    frame->width = ctx->width;
    frame->height = ctx->height;
    frame->format = ctx->pix_fmt;
    frame->pts = 0;

    // ����width��height��format����������
    ret = av_image_alloc(frame->data, frame->linesize,
        in.width, in.height, in.pixFmt, 1);
    if (ret < 0) {
        ERROR_BUF(ret);
        std::cout << "av_frame_get_buffer error" << errbuf;
        goto end;
    }

    // ����AVPacket
    pkt = av_packet_alloc();
    if (!pkt) {
        std::cout << "av_packet_alloc error";
        goto end;
    }

    // ���ļ�
    if (!inFile.open(QFile::ReadOnly)) {
        std::cout << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        std::cout << "file open error" << outFilename;
        goto end;
    }

    // ��ȡ���ݵ�frame��
    while ((ret = inFile.read((char*)frame->data[0],
        imgSize)) > 0) {
        // ���б���
        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }

        // ����֡�����
        frame->pts++;
    }

    // ˢ�»�����
    encode(ctx, nullptr, pkt, outFile);

end:
    // �ر��ļ�
    inFile.close();
    outFile.close();
    // �ͷ���Դ
    if (frame) {
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
    }
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);
    MessageBox(NULL, L"encode h264 ok!", L"encode", MB_OK);
}

void CEncode::run()
{
    h264Encode(in, H264_FILENAME);
}

int CEncode::startEncode()
{
    this->start();
    return 0;
}
