#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QVector>

#include "ping.h"
#include "cpuload.h"
#include "gpuload.h"

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

    void onPingDone(quint64 roundId, quint64 pingId, Ping::PingResponse pingResponse);

    void incrementCounter();
private:
    Ui::MainWindow* mUi;

    bool mIsStarted;
    quint64 mPingCounter;
    quint64 mMissedFrameCountAtStart;

    QVector<QThread*> mPingThreads;
    QVector<Ping*> mPings;

    struct RoundInfo {
        quint64 remainingPings;
        QString stateData;
        QString startTime;
        QVector<QString> pingResponses;
    };

    QHash<quint64, RoundInfo> mRemainingPings;
    QTimer mTimer;
    CpuLoad mCpuLoad;

    GpuLoad mGpuLoad;

    quint64 mDebugCounter;

    QFile mOutputFile;
    QDateTime mTimeStartRecord;
    quint64 mBytesWritten;

    void addLogItem(QString const& text);
};

#endif // MAINWINDOW_H
