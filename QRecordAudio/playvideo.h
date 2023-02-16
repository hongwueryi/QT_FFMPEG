#pragma once
#include <Windows.h>
#include <QThread>
#include "SDL.h"


class CVideoPlayer : public QThread
{
	Q_OBJECT;
public:
	explicit CVideoPlayer(QObject* parent = nullptr);
	~CVideoPlayer();
	void play();

private:
	void run();

};
