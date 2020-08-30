#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include <iostream>

#include "gpu_query.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), mUi(new Ui::MainWindow), mIsStarted(false), mPingCounter(0), mCpuLoad(), mGpuLoad(), mDebugCounter(0) {
    mUi->setupUi(this);

	QObject::connect(mUi->btnStartStop, SIGNAL(clicked()), this, SLOT(onButtonStartStopClick()));
	QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));

	//CbObject::setMainWindow(this, &mCbObject);

	QStringList locations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
	if (locations.size() > 0) {
		mUi->edtOutputDir->setText(locations.at(0));
	}
}

MainWindow::~MainWindow() {
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

    delete mUi;
}

void MainWindow::addLogItem(QString const& text) {
	QDateTime const now = QDateTime::currentDateTimeUtc();

	mUi->logWidget->addItem(QString("[%1]: %2").arg(now.toString("dd.MM.yyyy hh:mm:ss.zzz")).arg(text));
}

void MainWindow::onButtonStartStopClick() {
	if (mIsStarted) {
		mTimer.stop();
		mUi->btnStartStop->setText(QStringLiteral("Stopping..."));
		mUi->btnStartStop->setEnabled(false);

		mGpuLoad.stop();

		for (int i = 0; i < mPingThreads.size(); ++i) {
			QThread* pingThread = mPingThreads.at(i);
			pingThread->quit();
			pingThread->wait();
			delete mPings.at(i);
			delete pingThread;
		}
		mPingThreads.clear();
		mPings.clear();

		mOutputFile.close();

		/*quint64 const missedFrameCountAtEnd = mCbObject.getCurrentMissedFrameCount();
		if (mMissedFrameCountAtStart < missedFrameCountAtEnd) {
			// Meh, we lost information :(
		}*/

		mUi->btnStartStop->setText(QStringLiteral("Start Measurement"));
		mUi->btnStartStop->setEnabled(true);
	} else {
		mUi->logWidget->clear();
		mUi->btnStartStop->setText(QStringLiteral("Starting..."));
		mUi->btnStartStop->setEnabled(false);

		mTimeStartRecord = QDateTime::currentDateTime();
		mBytesWritten = 0;

		// Try to open output
		QString const location = mUi->edtOutputDir->text();
		QDir topDir(location);
		if ((!topDir.exists("zeusDebug") && !topDir.mkdir("zeusDebug")) || !topDir.cd("zeusDebug")) {
			QMessageBox::warning(this, "Error", QStringLiteral("Failed to create output directory 'zeusDebug' in location '%1'.").arg(location));

			mUi->btnStartStop->setText(QStringLiteral("Start Measurement"));
			mUi->btnStartStop->setEnabled(true);
			return;
		}

		QString const outputFileName = topDir.absoluteFilePath(QStringLiteral("log_%1.txt").arg(QDateTime::currentDateTimeUtc().toString("yyyy_MM_dd_hh_mm_ss_zzz")));
		mOutputFile.setFileName(outputFileName);
		if (mOutputFile.exists() || !mOutputFile.open(QFile::WriteOnly)) {
			QMessageBox::warning(this, "Error", QStringLiteral("Failed to create output file '%1'.").arg(outputFileName));

			mUi->btnStartStop->setText(QStringLiteral("Start Measurement"));
			mUi->btnStartStop->setEnabled(true);
			return;
		}

		QStringList const targets = mUi->edtTargets->text().split(QChar(','), Qt::SkipEmptyParts);
		uint32_t const interval = static_cast<uint32_t>(mUi->spinInterval->value());

		// Headers
		mOutputFile.write(QStringLiteral("# Type 1: State information\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#    Fields:\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#     - Time in msecs since epoch\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#     - userDelta, kernelDelta, idleDelta (100 nanosecond resolution)\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#     - memory load, total mem, free mem, total page, free page, total virt, free virt (in bytes)\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#     - RTT and TTL for pings to all targets (%1)\r\n").arg(targets.join(';')).toUtf8());
		mOutputFile.write(QStringLiteral("# Type 2: Processes information\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#    Fields:\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#     - PID, ImageName\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#     - UserTime, KernelTime\r\n").toUtf8());
		mOutputFile.write(QStringLiteral("#     - WorkingSetSize, PeakWorkingSetSize (both as value and delta to last)\r\n").toUtf8());

		//mMissedFrameCountAtStart = mCbObject.getCurrentMissedFrameCount();

		addLogItem(QStringLiteral("Starting measurement with %1 ping targets at %2ms intervals.").arg(targets.size()).arg(interval));

		for (int i = 0; i < targets.size(); ++i) {
			QThread* pingThread = new QThread(this);
			mPingThreads.append(pingThread);

			Ping* ping = new Ping(targets.at(i), 2000, nullptr);
			ping->moveToThread(pingThread);

			if (!QObject::connect(ping, SIGNAL(pingDone(quint64, quint64, Ping::PingResponse)), this, SLOT(onPingDone(quint64, quint64, Ping::PingResponse)), Qt::ConnectionType::QueuedConnection)) {
				std::cerr << "Failed to connect pingDone to main!" << std::endl;
			}

			mPings.append(ping);

			pingThread->start();
		}

		mGpuLoad.start();

		mTimer.setSingleShot(false);
		mTimer.setInterval(interval);
		//mTimer.setSingleShot(true);
		//mTimer.setInterval(50);
		mTimer.start();

		mUi->btnStartStop->setText(QStringLiteral("Stop Measurement"));
		mUi->btnStartStop->setEnabled(true);
	}
	mIsStarted = !mIsStarted;
}

void MainWindow::incrementCounter() {
	++mDebugCounter;
	if (mDebugCounter > 2147483647uLL) {
		mDebugCounter = 0;
	}

	mUi->spinDebug->setValue(mDebugCounter);
}

void MainWindow::onTimerTimeout() {
	bool const showLog = mUi->cboxShowLog->checkState() == Qt::Checked;
	QDateTime const now = QDateTime::currentDateTimeUtc();

	// GPU
	//QDateTime before = QDateTime::currentDateTime();
	mGpuLoad.update();
	//QDateTime after = QDateTime::currentDateTime();
	//std::cout << "Took " << before.msecsTo(after) << "ms." << std::endl;

	// CPU
	mCpuLoad.update(mGpuLoad.getCurrentGpuLoad());

	if (mCpuLoad.isArmaRunning()) {
		mUi->lblArmaState->setText(QStringLiteral("yes (PID = %1, %2)").arg(mCpuLoad.getArmaPid()).arg(mCpuLoad.getArmaImageName()));
	} else {
		mUi->lblArmaState->setText(QStringLiteral("no"));
	}

	std::size_t const coreCount = mCpuLoad.getCoreCount();
	QString const cpuState = mCpuLoad.getStateString();
	QStringList const processesStates = mCpuLoad.getProcessesStrings();

	QString loadStringDbg = "Load: ";
	QString loadString = "";
	for (std::size_t i = 0; i < coreCount; ++i) {
		if (i > 0) {
			loadStringDbg += ", ";
			loadString += ";";
		}
		loadStringDbg += QString("Core %1: %2").arg(i).arg(mCpuLoad.getCpuLoadOfCore(i), 2, 'g', 2);
		loadString += QString("%1").arg(mCpuLoad.getCpuLoadOfCore(i), 2, 'g', 2);
	}
	if (showLog) {
		addLogItem(loadStringDbg);
	}

	RoundInfo roundInfo;
	roundInfo.startTime = QString::number(now.toMSecsSinceEpoch());
	roundInfo.remainingPings = mPings.size();
	roundInfo.pingResponses.resize(mPings.size());
	roundInfo.stateData = QStringLiteral("1;").append(roundInfo.startTime).append(";").append(cpuState);

	for (int i = 0; i < processesStates.size(); ++i) {
		mBytesWritten += mOutputFile.write(QStringLiteral("2;%1;%2\r\n").arg(roundInfo.startTime).arg(processesStates.at(i)).toUtf8());
	}

	mRemainingPings.insert(mPingCounter, roundInfo);

	// Pings
	if (showLog) {
		addLogItem(QString("Pinging (round #%1)...").arg(mPingCounter));
	}
	for (int i = 0; i < mPings.size(); ++i) {
		if (!QMetaObject::invokeMethod(mPings.at(i), "doPing", Qt::ConnectionType::QueuedConnection, Q_ARG(quint64, mPingCounter), Q_ARG(quint64, i))) {
			std::cerr << "Failed to invoke ping method!" << std::endl;
		}
	}

	++mPingCounter;
}

void MainWindow::onPingDone(quint64 roundId, quint64 pingId, Ping::PingResponse pingResponse) {
	bool const showLog = mUi->cboxShowLog->checkState() == Qt::Checked;
	if (showLog) {
		if (pingResponse.hasError) {
			addLogItem(QString("Received no ping reply from %1, error: %2 (round #%3).").arg(pingResponse.target).arg(pingResponse.errorCode).arg(roundId));
		} else {
			addLogItem(QString("Received ping reply from %1 with RTT %2 (round #%3).").arg(pingResponse.target).arg(pingResponse.roundTripTime).arg(roundId));
		}
	}

	if (!mRemainingPings.contains(roundId)) {
		std::cerr << "No RoundInfo for ping from iteration " << roundId << "!" << std::endl;
		throw;
	}
	RoundInfo& roundInfo = mRemainingPings.find(roundId).value();
	roundInfo.remainingPings--;
	roundInfo.pingResponses[pingId] = pingResponse.hasError ? QStringLiteral(";-1;").append(pingResponse.errorCode) : QStringLiteral(";%1;%2").arg(QString::number(pingResponse.roundTripTime)).arg(QString::number(pingResponse.ttl));

	if (roundInfo.remainingPings == 0) {
		if (showLog) {
			addLogItem(QStringLiteral("Round #%1 of pings done, finishing up.").arg(roundId));
		}

		if (mOutputFile.isOpen()) {
			// Type state
			QString stateOut = roundInfo.stateData;
			for (int i = 0; i < roundInfo.pingResponses.size(); ++i) {
				stateOut += roundInfo.pingResponses.at(i);
			}
			mBytesWritten += mOutputFile.write(stateOut.toUtf8());

			// Stats
			double const hours = mTimeStartRecord.msecsTo(QDateTime::currentDateTime()) / 1000.0 / 60.0 / 60.0;
			quint64 bytesPerHour(mBytesWritten / hours);
			mUi->lblLogStats->setText(QStringLiteral("%1 Bytes (~%2 Bytes per hour)").arg(mBytesWritten).arg(bytesPerHour));
		}
		mRemainingPings.remove(roundId);
	}
}
