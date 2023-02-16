#pragma once
#include <QThread>
extern "C"
{
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

class CRecordAudio : public QThread
{
	Q_OBJECT;

private:
	void run();

public:
	explicit CRecordAudio(QObject* parent = nullptr);
	~CRecordAudio();

public slots:
	void Init();
	void UnInit();
	void showSpec();

private:
	AVFormatContext* m_ctx = nullptr;
	AVPacket* pkt = nullptr;
	
};
