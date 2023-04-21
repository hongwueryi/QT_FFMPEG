#include "recordscreen.h"
#include <QDebug>
#include <QPainter>
#include <QTimer>

Widget::Widget(QWidget* parent)
    : QWidget(parent)
{
    avdevice_register_all();                                 
    formatContext = avformat_alloc_context();                    
    const AVInputFormat* inputFormat = av_find_input_format("gdigrab"); //寻找输入设备【gdigrab】
    AVDictionary* options = NULL;
    av_dict_set(&options, "framerate", "60", 0); //设置帧数为60
    if (avformat_open_input(&formatContext, "desktop", inputFormat, &options)) { 
        qDebug() << "cant`t open input stream.";
        return;
    }
    if (avformat_find_stream_info(formatContext, nullptr)) {//加载流中存储的信息
        qDebug() << "can`t find stream information.";
        return;
    }

    videoIndex = -1;//寻找视频流
    for (uint i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    }
    if (videoIndex == -1) {
        qDebug() << "can`t find video stream.";
        return;
    }
    codecParameters = formatContext->streams[videoIndex]->codecpar;
    codecContext = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(codecContext, codecParameters);
    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);

    if (codec == nullptr) {
        qDebug() << "can`t find codec";
        return;
    }
    if (avcodec_open2(codecContext, codec, nullptr)) {
        qDebug() << "can`t open codec";
        return;
    }

    packet = av_packet_alloc();
    frame = av_frame_alloc();
    codecParameters->width = 1920;
    imgConvertContext = sws_getContext(codecParameters->width, codecParameters->height, codecContext->pix_fmt, codecParameters->width, codecParameters->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
    image = QImage(codecParameters->width, codecParameters->height, QImage::Format_RGB888);
    av_image_fill_linesizes(lineSize, AV_PIX_FMT_RGB24, codecParameters->width);

    QTimer* timer = new QTimer;           //定时刷新
    connect(timer, &QTimer::timeout, this, static_cast<void (QWidget::*)()>(&QWidget::repaint));
    timer->setInterval(20);
    timer->start();

    resize(codecParameters->width * 0.6, codecParameters->height * 0.6);
}

Widget::~Widget()
{
}

int save_rgb_to_file(AVFrame* frame, int num) {
    //拼接文件名
    char pic_name[200] = { 0 };
    sprintf(pic_name, "./rgba_8888_%d.yuv", num);

    //写入文件
    FILE* fp = NULL;
    fp = fopen(pic_name, "wb+");
    fwrite(frame->data[0], 1, frame->linesize[0] * frame->height, fp);
    fclose(fp);
    return 0;
}

void Widget::paintEvent(QPaintEvent* event)
{
    if (av_read_frame(formatContext, packet)) {
        return;
    }
    if (packet->stream_index == videoIndex) {
        if (avcodec_send_packet(codecContext, packet))
            return;
        if (avcodec_receive_frame(codecContext, frame))
            return;
        uint8_t* dst[] = { image.bits() };
        sws_scale(imgConvertContext, frame->data, frame->linesize, 0, codecParameters->height, dst, lineSize);
        av_packet_unref(packet); //清空数据包
        static int i = 0;
        //save_rgb_to_file(frame, i++); //保存到文件
        QPainter painter(this);
        painter.fillRect(rect(), image.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}
