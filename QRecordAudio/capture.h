#pragma once
#include <QThread>

class CCapture : public QThread
{
Q_OBJECT;
public:
    explicit CCapture(QObject* parent = nullptr);
    ~CCapture();
public:
    void CaptureScreen();
private:
    void run();

};