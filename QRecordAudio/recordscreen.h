//ץ������ʾ����д��1920*x�ķֱ���
#pragma once
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avio.h"
#include "libavutil/imgutils.h"
}
#include <QWidget>
class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget* parent = nullptr);
    ~Widget();
private:
    QImage image;
    int lineSize[4];
    AVFormatContext* formatContext;
    AVCodecParameters* codecParameters;
    int videoIndex;
    AVCodecContext* codecContext;
    AVPacket* packet;
    AVFrame* frame;
    SwsContext* imgConvertContext;

protected:
    virtual void paintEvent(QPaintEvent* event) override;
};