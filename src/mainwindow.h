#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QVector>

#include "round_info.h"
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

    void onPingDone(quint64 roundId, quint64 pingId, PingResponse pingResponse);
private:
    Ui::MainWindow* mUi;

    bool mIsStarted;
    bool mUseVerboseJson;

    double mMinCpuUtil;
    double mMinGpuUtil;
    double mMinMemUtil;

    quint64 mPingCounter;

    QVector<QThread*> mPingThreads;
    QVector<Ping*> mPings;

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
