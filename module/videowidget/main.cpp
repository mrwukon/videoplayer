

#include <QApplication>
#include <QTextCodec>

#include "Widget/VideoPlayerWidget.h"

#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);

    VideoPlayerWidget w;
    w.show();

    return a.exec();
}

