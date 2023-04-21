#include "QRecordAudio.h"
#include <QtWidgets/QApplication>
#include <QFontDatabase>
#include <Windows.h>
#include "recordscreen.h"
int main(int argc, char *argv[])
{
    //int fontId = QFontDatabase::addApplicationFont(":/pic/LatoRegular.ttf");
    //QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QApplication a(argc, argv);
    ::AllocConsole();
    static FILE* fp = NULL;
    freopen_s(&fp, "CONOUT$", "w+t", stdout);
    QRecordAudio w;
    //Widget w;  //ÆÁÄ»Â¼ÖÆ
    w.show();

    
    return a.exec();
}
