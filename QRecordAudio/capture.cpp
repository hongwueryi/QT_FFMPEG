#include "capture.h"
#include <qdesktopwidget.h>
#include <qscreen.h>
#include <qapplication.h>

CCapture::CCapture(QObject* parent)
{

}
CCapture::~CCapture()
{

}

void CCapture::CaptureScreen()
{
    QDesktopWidget* desk = QApplication::desktop();
    QScreen* screen = QGuiApplication::primaryScreen();
    QPixmap p = screen->grabWindow(desk->winId());
    QImage image = p.toImage();
    image.save("E:\\Dten\\other demo\\Qtest\\QT_FFMPEG\\bin\\temp.png");
}

void CCapture::run()
{
    CaptureScreen();
}