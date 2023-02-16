#pragma once
#include <QThread>
extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
}

class CRecordVideo : public QThread
{
	Q_OBJECT;

private:
	void run();

public:
	explicit CRecordVideo(QObject* parent = nullptr);
	~CRecordVideo();

public slots:
	void Init();
	void UnInit();

private:
	AVFormatContext* m_ctx = nullptr;   // 格式上下文
	AVPacket* m_pkt = nullptr;

};
