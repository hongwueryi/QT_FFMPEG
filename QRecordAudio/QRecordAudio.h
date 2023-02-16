#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QRecordAudio.h"

class QRecordAudio : public QMainWindow
{
    Q_OBJECT

public:
    QRecordAudio(QWidget *parent = Q_NULLPTR);

private:
    Ui::QRecordAudioClass ui;
};
