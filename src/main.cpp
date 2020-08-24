#include "mainwindow.h"
#include <QApplication>

#include "ping.h"

int main(int argc, char *argv[]) {
    qRegisterMetaType<Ping::PingResponse>();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

#ifdef _MSC_VER
int __stdcall WinMain(struct HINSTANCE__ *hInstance, struct HINSTANCE__ *hPrevInstance, char *lpszCmdLine, int nCmdShow) {
	return main(__argc, __argv);
}

#endif
