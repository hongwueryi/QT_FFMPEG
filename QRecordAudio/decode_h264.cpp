/*
*ffmpeg -c:v h264 -i my.h264 out.yuv
# -c:v h264是指定使用h264作为解码器
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

// 输入缓冲区的大小
#define IN_DATA_SIZE 4096

// 返回结果
int iRet = 0;

// 用来存放读取的输入文件数据（h264）
char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
char* inData = inDataArray;

// 每次从输入文件中读取的长度（h264）
// 输入缓冲区中，剩下的等待进行解码的有效数据长度
int inLen;
// 是否已经读取到了输入文件的尾部
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
    // 发送压缩数据到解码器
    int iRet = avcodec_send_packet(ctx, pkt);
    if (iRet < 0) {
        ERROR_BUF(iRet);
        std::cout << "avcodec_send_packet error" << errbuf;
        return iRet;
    }

    while (true) {
        // 获取解码后的数据
        iRet = avcodec_receive_frame(ctx, frame);
        if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF) {
            return 0;
        }
        else if (iRet < 0) {
            ERROR_BUF(iRet);
            std::cout << "avcodec_receive_frame error" << errbuf;
            return iRet;
        }

        // 将解码后的数据写入文件
        // 写入Y平面
        outDecodeFile.write((char*)frame->data[0],
            frame->linesize[0] * ctx->height);
        // 写入U平面
        outDecodeFile.write((char*)frame->data[1],
            frame->linesize[1] * ctx->height >> 1);
        // 写入V平面
        outDecodeFile.write((char*)frame->data[2],
            frame->linesize[2] * ctx->height >> 1);
    }
}

void CDecode::h264Decode(VideoDecodeSpec& in, const char* outDecodeFilename)
{
    // 解码器
    const AVCodec* codec = nullptr;
    // 上下文
    AVCodecContext* ctx = nullptr;
    // 解析器上下文
    AVCodecParserContext* parserCtx = nullptr;

    // 存放解码前的数据(h264)
    AVPacket* pkt = nullptr;
    // 存放解码后的数据(yuv)
    AVFrame* frame = nullptr;


    //获取解码器
    //codec = avcodec_find_decoder_by_name("h264");
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cout << "decoder not found";
        return;
    }

    // 初始化解析器上下文
    parserCtx = av_parser_init(codec->id);
    if (!parserCtx) {
        std::cout << "av_parser_init error";
        return;
    }

    // 创建上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        std::cout << "avcodec_alloc_context3 error";
        goto end;
    }

    // 创建AVPacket
    pkt = av_packet_alloc();
    if (!pkt) {
        std::cout << "av_packet_alloc error";
        goto end;
    }

    // 创建AVFrame
    frame = av_frame_alloc();
    if (!frame) {
        std::cout << "av_frame_alloc error";
        goto end;
    }

    // 打开解码器
    iRet = avcodec_open2(ctx, codec, nullptr);
    if (iRet < 0) {
        ERROR_BUF(iRet);
        std::cout << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // 打开文件
    if (!inDecodeFile.open(QFile::ReadOnly)) {
        std::cout << "file open error:" << H264_FILENAME;
        goto end;
    }
    if (!outDecodeFile.open(QFile::WriteOnly)) {
        std::cout << "file open error:" << VOUT_FILENAME;
        goto end;
    }

    // 读取文件数据
    do {
        inLen = inDecodeFile.read(inDataArray, IN_DATA_SIZE);
        // 设置是否到了文件尾部
        inEnd = !inLen;

        // 让inData指向数组的首元素
        inData = inDataArray;

        // 只要输入缓冲区中还有等待进行解码的数据
        while (inLen > 0 || inEnd) {
            // 到了文件尾部（虽然没有读取任何数据，但也要调用av_parser_parse2，修复bug）
            // 经过解析器解析
            iRet = av_parser_parse2(parserCtx, ctx,
                &pkt->data, &pkt->size,
                (uint8_t*)inData, inLen,
                AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

            if (iRet < 0) {
                ERROR_BUF(iRet);
                std::cout << "av_parser_parse2 error" << errbuf;
                goto end;
            }

            // 跳过已经解析过的数据
            inData += iRet;
            // 减去已经解析过的数据大小
            inLen -= iRet;

            std::cout << inEnd << pkt->size << iRet;

            // 解码
            if (pkt->size > 0 && decode(ctx, pkt, frame, outDecodeFile) < 0) {
                goto end;
            }

            // 如果到了文件尾部
            if (inEnd) break;
        }
    } while (!inEnd);

    // 刷新缓冲区
    //    pkt->data = nullptr;
    //    pkt->size = 0;
    //    decode(ctx, pkt, frame, outDecodeFile);
    decode(ctx, nullptr, frame, outDecodeFile);

    // 赋值输出参数
    out.width = ctx->width;
    out.height = ctx->height;
    out.pixFmt = ctx->pix_fmt;
    // 用framerate.num获取帧率，并不是time_base.den
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