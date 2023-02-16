#pragma once
#include <QThread>
extern "C" {
#include <libavutil/avutil.h>
}

typedef struct _VideoEncodeSpec 
{
    const char* filename;
    int width;
    int height;
    AVPixelFormat pixFmt;
    int fps;
} VideoEncodeSpec;

class CEncode : public QThread
{
    Q_OBJECT;
public:
    explicit CEncode(QObject* parent = nullptr);
    ~CEncode();
    static void h264Encode(VideoEncodeSpec& in, const char* outFilename);
    int startEncode();
private:
    void run();

};