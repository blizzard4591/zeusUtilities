#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QVector>

#include "ping.h"
#include "cpuload.h"
#include "gpuload.h"
#include "cb_object.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void onButtonStartStopClick();
    void onTimerTimeout();

    void onPingDone(quint64 pingId, Ping::PingResponse pingResponse);

    void incrementCounter();
    void setProcessUpdatesState(bool isCurrentlyOkay, quint64 errorCount);
private:
    Ui::MainWindow* mUi;

    bool mIsStarted;
    quint64 mPingCounter;
    quint64 mMissedFrameCountAtStart;

    QVector<QThread*> mPingThreads;
    QVector<Ping*> mPings;
    QTimer mTimer;
    CpuLoad mCpuLoad;

    GpuLoad mGpuLoad;
    CbObject mCbObject;

    quint64 mDebugCounter;

    void addLogItem(QString const& text);
};

#endif // MAINWINDOW_H
