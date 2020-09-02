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
#include "cpu_load.h"
#include "gpu_load.h"
#include "etw_query.h"

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
    void menuAboutAboutOnClick();
    void menuAboutAboutQtOnClick();
    void menuFileOpenLogDirectoryOnClick();
    void menuFileQuitOnClick();

    void onPingDone(quint64 roundId, quint64 pingId, Ping::PingResponse pingResponse);
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

    EtwQuery mEtwQuery;

    quint64 mDebugCounter;

    QFile mOutputFile;
    QDateTime mTimeStartRecord;
    quint64 mBytesWritten;

    void addLogItem(QString const& text);
    QString formatSize(quint64 number) const;
    void updateStats();
    void clearStats();
};

#endif // MAINWINDOW_H
