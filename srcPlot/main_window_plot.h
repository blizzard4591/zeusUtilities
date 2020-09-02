#ifndef MAIN_WINDOW_PLOT_H
#define MAIN_WINDOW_PLOT_H

#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMainWindow>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QVector>

namespace Ui {
    class MainWindowPlot;
}

class MainWindowPlot : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindowPlot(QWidget *parent = 0);
    ~MainWindowPlot();

public slots:
    void menuAboutAboutOnClick();
    void menuAboutAboutQtOnClick();
    void menuFileOpenLogDirectoryOnClick();
    void menuFileQuitOnClick();

    void logFileLocationChanged();
    void listWidgetLogFilesOnCurrentItemChanged();
private:
    Ui::MainWindowPlot* mUi;

    QFileInfoList mLogFiles;
    QFile mCurrentLogFile;

    QString formatSize(quint64 number) const;
    void parseLog(QString const& log);
};

#endif
