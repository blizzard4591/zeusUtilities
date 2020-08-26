#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), mUi(new Ui::MainWindow), mIsStarted(false), mPingCounter(0), mCpuLoad() {
    mUi->setupUi(this);

	QObject::connect(mUi->btnStartStop, SIGNAL(clicked()), this, SLOT(onButtonStartStopClick()));
	QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
}

MainWindow::~MainWindow() {
    delete mUi;
}

void MainWindow::addLogItem(QString const& text) {
	QDateTime const now = QDateTime::currentDateTimeUtc();

	mUi->logWidget->addItem(QString("[%1]: %2").arg(now.toString("dd.MM.yyyy hh:mm:ss.zzz")).arg(text));
}

void MainWindow::onButtonStartStopClick() {
	if (mIsStarted) {
		mTimer.stop();

		for (int i = 0; i < mPingThreads.size(); ++i) {
			QThread* pingThread = mPingThreads.at(i);
			pingThread->quit();
			pingThread->wait();
			delete mPings.at(i);
			delete pingThread;
		}
		mPingThreads.clear();
		mPings.clear();

		mUi->btnStartStop->setText("Start Measurement");
	} else {
		mUi->logWidget->clear();
		mUi->btnStartStop->setText("Stop Measurement");

		QStringList const targets = mUi->edtTargets->text().split(QChar(','), Qt::SkipEmptyParts);
		uint32_t const interval = static_cast<uint32_t>(mUi->spinInterval->value());

		addLogItem(QString("Starting measurement with %1 ping targets at %2ms intervals.").arg(targets.size()).arg(interval));

		for (int i = 0; i < targets.size(); ++i) {
			QThread* pingThread = new QThread(this);
			mPingThreads.append(pingThread);

			Ping* ping = new Ping(targets.at(i), 2000, nullptr);
			ping->moveToThread(pingThread);

			if (!QObject::connect(ping, SIGNAL(pingDone(quint64, Ping::PingResponse)), this, SLOT(onPingDone(quint64, Ping::PingResponse)), Qt::ConnectionType::QueuedConnection)) {
				std::cerr << "Failed to connect pingDone to main!" << std::endl;
			}

			mPings.append(ping);

			pingThread->start();
		}

		mTimer.setSingleShot(false);
		mTimer.setInterval(interval);
		mTimer.start();
	}
	mIsStarted = !mIsStarted;
}

void MainWindow::onTimerTimeout() {
	// GPU
	mGpuLoad.update();
	return;

	// CPU
	mCpuLoad.update();
	std::size_t const coreCount = mCpuLoad.getCoreCount();
	QString loadString = "Load: ";
	for (std::size_t i = 0; i < coreCount; ++i) {
		if (i > 0) {
			loadString += ", ";
		}
		loadString += QString("Core %1: %2").arg(i).arg(mCpuLoad.getCpuLoadOfCore(i), 2, 'g', 2);
	}
	addLogItem(loadString);
	return;

	// Pings
	addLogItem(QString("Pinging (round #%1)...").arg(mPingCounter));
	for (int i = 0; i < mPings.size(); ++i) {
		if (!QMetaObject::invokeMethod(mPings.at(i), "doPing", Qt::ConnectionType::QueuedConnection, Q_ARG(quint64, mPingCounter))) {
			std::cerr << "Failed to invoke ping method!" << std::endl;
		}
	}
	++mPingCounter;
}

void MainWindow::onPingDone(uint64_t pingId, Ping::PingResponse pingResponse) {
	addLogItem(QString("Received ping reply from %1 with RTT %2 (round #%3).").arg(pingResponse.target).arg(pingResponse.roundTripTime).arg(pingId));
}
