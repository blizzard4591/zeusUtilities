#include "main_window_plot.h"

#include <QApplication>

#include <cstdint>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindowPlot w;
    w.show();

    return a.exec();
}

#ifdef _MSC_VER
int __stdcall WinMain(struct HINSTANCE__ *hInstance, struct HINSTANCE__ *hPrevInstance, char *lpszCmdLine, int nCmdShow) {
	return main(__argc, __argv);
}

#endif
