#include "QRecordAudio.h"
#include "audio.h"
#include <Windows.h>
#include "playaudio.h"
#include "video.h"
#include "playvideo.h"
#include "encode_h264.h"
#include "decode_h264.h"
#include "capture.h"

QRecordAudio::QRecordAudio(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    CRecordAudio* pAudio = new CRecordAudio(this);
    CAudioPlayer* pPlayAudio = new CAudioPlayer(this);
    CRecordVideo* pVideo = new CRecordVideo(this);
    CVideoPlayer* pPlayVideo = new CVideoPlayer(this);
    CEncode* pEncode = new CEncode(this);
    CDecode* pDecode = new CDecode(this);
    CCapture* pCapture = new CCapture(this);
    connect(ui.btn_start, &QPushButton::clicked, pAudio, &CRecordAudio::Init);
    connect(ui.btn_stop, &QPushButton::clicked, pAudio, &CRecordAudio::UnInit);
    connect(ui.btn_showspec, &QPushButton::clicked, pAudio, &CRecordAudio::showSpec);
    connect(ui.BTN_SDLPLAY, &QPushButton::clicked, pPlayAudio, &CAudioPlayer::Init);

    connect(ui.BTN_SART_RECORD_VIDEO, &QPushButton::clicked, pVideo, &CRecordVideo::Init);
    connect(ui.BTN_STOP_RECORD_VIDEO, &QPushButton::clicked, pVideo, &CRecordVideo::UnInit);
    connect(ui.BTN_PLAY_VIDEO, &QPushButton::clicked, pPlayVideo, &CVideoPlayer::play);

    connect(ui.BTN_ENCODE_H264, &QPushButton::clicked, pEncode, &CEncode::startEncode);
    connect(ui.BTN_DECODE_H264, &QPushButton::clicked, pDecode, &CDecode::startDecode);
    connect(ui.BTN_CAPTURE_SCREEN, &QPushButton::clicked, pCapture, &CCapture::CaptureScreen);
}
