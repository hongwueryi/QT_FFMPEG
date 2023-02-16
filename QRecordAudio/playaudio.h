#pragma once
#include <Windows.h>
#include <QThread>
#include "SDL.h"
typedef struct {
	int len = 0;
	int pullLen = 0;
	Uint8* data = nullptr;
} AudioBuffer;

class CAudioPlayer : public QThread
{
	Q_OBJECT;
public:
	explicit CAudioPlayer(QObject* parent = nullptr);
	~CAudioPlayer();
	void Init();

private:
	void run();

	AudioBuffer buffer;

};
