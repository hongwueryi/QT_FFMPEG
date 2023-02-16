#pragma once

#include "pch.h"
#include <QThread>
extern "C" {
#include <libavutil/avutil.h>
}

typedef struct {
    const char* filename;
    int width;
    int height;
    AVPixelFormat pixFmt;
    int fps;
} VideoDecodeSpec;


class CDecode : public QThread
{
    Q_OBJECT;
public:
    explicit CDecode(QObject* parent = nullptr);
    ~CDecode();
    void h264Decode(VideoDecodeSpec& in, const char* outFilename);
    int startDecode();
private:
    void run();

private:
    VideoDecodeSpec out;
};