#include "QRecordAudio.h"
#include <QtWidgets/QApplication>
#include <Windows.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ::AllocConsole();
    static FILE* fp = NULL;
    freopen_s(&fp, "CONOUT$", "w+t", stdout);
    QRecordAudio w;
    w.show();
    return a.exec();
}
