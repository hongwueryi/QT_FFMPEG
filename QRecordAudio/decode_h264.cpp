/*
*ffmpeg -c:v h264 -i my.h264 out.yuv
# -c:v h264��ָ��ʹ��h264��Ϊ������
*/
#include "decode_h264.h"
#include <QFile>
#include <iostream>
#include <Windows.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#define ERROR_BUF(iRet) \
    char errbuf[1024]; \
    av_strerror(iRet, errbuf, sizeof (errbuf));

// ���뻺�����Ĵ�С
#define IN_DATA_SIZE 4096

// ���ؽ��
int iRet = 0;

// ������Ŷ�ȡ�������ļ����ݣ�h264��
char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
char* inData = inDataArray;

// ÿ�δ������ļ��ж�ȡ�ĳ��ȣ�h264��
// ���뻺�����У�ʣ�µĵȴ����н������Ч���ݳ���
int inLen;
// �Ƿ��Ѿ���ȡ���������ļ���β��
int inEnd = 0;

QFile inDecodeFile(H264_FILENAME);
QFile outDecodeFile(VOUT_FILENAME);



CDecode::CDecode(QObject* parent)
{
    out.filename = VOUT_FILENAME;
}

CDecode::~CDecode()
{

}

static int decode(AVCodecContext* ctx,
    AVPacket* pkt,
    AVFrame* frame,
    QFile& outDecodeFile) {
    // ����ѹ�����ݵ�������
    int iRet = avcodec_send_packet(ctx, pkt);
    if (iRet < 0) {
        ERROR_BUF(iRet);
        std::cout << "avcodec_send_packet error" << errbuf;
        return iRet;
    }

    while (true) {
        // ��ȡ����������
        iRet = avcodec_receive_frame(ctx, frame);
        if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF) {
            return 0;
        }
        else if (iRet < 0) {
            ERROR_BUF(iRet);
            std::cout << "avcodec_receive_frame error" << errbuf;
            return iRet;
        }

        // ������������д���ļ�
        // д��Yƽ��
        outDecodeFile.write((char*)frame->data[0],
            frame->linesize[0] * ctx->height);
        // д��Uƽ��
        outDecodeFile.write((char*)frame->data[1],
            frame->linesize[1] * ctx->height >> 1);
        // д��Vƽ��
        outDecodeFile.write((char*)frame->data[2],
            frame->linesize[2] * ctx->height >> 1);
    }
}

void CDecode::h264Decode(VideoDecodeSpec& in, const char* outDecodeFilename)
{
    // ������
    const AVCodec* codec = nullptr;
    // ������
    AVCodecContext* ctx = nullptr;
    // ������������
    AVCodecParserContext* parserCtx = nullptr;

    // ��Ž���ǰ������(h264)
    AVPacket* pkt = nullptr;
    // ��Ž���������(yuv)
    AVFrame* frame = nullptr;


    //��ȡ������
    //codec = avcodec_find_decoder_by_name("h264");
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cout << "decoder not found";
        return;
    }

    // ��ʼ��������������
    parserCtx = av_parser_init(codec->id);
    if (!parserCtx) {
        std::cout << "av_parser_init error";
        return;
    }

    // ����������
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        std::cout << "avcodec_alloc_context3 error";
        goto end;
    }

    // ����AVPacket
    pkt = av_packet_alloc();
    if (!pkt) {
        std::cout << "av_packet_alloc error";
        goto end;
    }

    // ����AVFrame
    frame = av_frame_alloc();
    if (!frame) {
        std::cout << "av_frame_alloc error";
        goto end;
    }

    // �򿪽�����
    iRet = avcodec_open2(ctx, codec, nullptr);
    if (iRet < 0) {
        ERROR_BUF(iRet);
        std::cout << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // ���ļ�
    if (!inDecodeFile.open(QFile::ReadOnly)) {
        std::cout << "file open error:" << H264_FILENAME;
        goto end;
    }
    if (!outDecodeFile.open(QFile::WriteOnly)) {
        std::cout << "file open error:" << VOUT_FILENAME;
        goto end;
    }

    // ��ȡ�ļ�����
    do {
        inLen = inDecodeFile.read(inDataArray, IN_DATA_SIZE);
        // �����Ƿ����ļ�β��
        inEnd = !inLen;

        // ��inDataָ���������Ԫ��
        inData = inDataArray;

        // ֻҪ���뻺�����л��еȴ����н��������
        while (inLen > 0 || inEnd) {
            // �����ļ�β������Ȼû�ж�ȡ�κ����ݣ���ҲҪ����av_parser_parse2���޸�bug��
            // ��������������
            iRet = av_parser_parse2(parserCtx, ctx,
                &pkt->data, &pkt->size,
                (uint8_t*)inData, inLen,
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

            if (iRet < 0) {
                ERROR_BUF(iRet);
                std::cout << "av_parser_parse2 error" << errbuf;
                goto end;
            }

            // �����Ѿ�������������
            inData += iRet;
            // ��ȥ�Ѿ������������ݴ�С
            inLen -= iRet;

            std::cout << inEnd << pkt->size << iRet;

            // ����
            if (pkt->size > 0 && decode(ctx, pkt, frame, outDecodeFile) < 0) {
                goto end;
            }

            // ��������ļ�β��
            if (inEnd) break;
        }
    } while (!inEnd);

    // ˢ�»�����
    //    pkt->data = nullptr;
    //    pkt->size = 0;
    //    decode(ctx, pkt, frame, outDecodeFile);
    decode(ctx, nullptr, frame, outDecodeFile);

    // ��ֵ�������
    out.width = ctx->width;
    out.height = ctx->height;
    out.pixFmt = ctx->pix_fmt;
    // ��framerate.num��ȡ֡�ʣ�������time_base.den
    out.fps = ctx->framerate.num;

end:
    inDecodeFile.close();
    outDecodeFile.close();
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);
    MessageBox(NULL, L"decode H264 ok!", L"decode", MB_OK);
}

void CDecode::run()
{
    h264Decode(out, VOUT_FILENAME);
}

int CDecode::startDecode()
{
    this->start();
    return 0;
}