﻿#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcessEnvironment>

#include <iostream>

#include "gpu_query.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), mUi(new Ui::MainWindow), mIsStarted(false), mPingCounter(0), mCpuLoad(), mGpuLoad(), mDebugCounter(0) {
    mUi->setupUi(this);

	this->setWindowTitle(QStringLiteral("ZeusOps Debug Utility v").append(QString::fromStdString(Version::versionWithTagString())));

	// Menu Items
	QObject::connect(mUi->actionAbout, SIGNAL(triggered()), this, SLOT(menuAboutAboutOnClick()));
	QObject::connect(mUi->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(menuAboutAboutQtOnClick()));
	QObject::connect(mUi->actionOpen_Log_Directory, SIGNAL(triggered()), this, SLOT(menuFileOpenLogDirectoryOnClick()));
	QObject::connect(mUi->actionQuit, SIGNAL(triggered()), this, SLOT(menuFileQuitOnClick()));


	QObject::connect(mUi->btnStartStop, SIGNAL(clicked()), this, SLOT(onButtonStartStopClick()));
	QObject::connect(&mTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));

	//CbObject::setMainWindow(this, &mCbObject);

	QStringList locations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
	if (locations.size() > 0) {
		QDir location(locations.at(0));
		if ((!location.exists(QStringLiteral("zeusDebug")) && !location.mkdir(QStringLiteral("zeusDebug"))) || !location.cd(QStringLiteral("zeusDebug"))) {
			// Fallback
			mUi->edtOutputDir->setText(locations.at(0));
		} else {
			mUi->edtOutputDir->setText(location.absolutePath());
		}
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

	mUi->logWidget->addItem(QString("[%1 UTC]: %2").arg(now.toString("dd.MM.yyyy hh:mm:ss.zzz")).arg(text));
}

QString MainWindow::formatSize(quint64 number) const {
	if (number > 1073741824uLL) {
		double const value = number / 1073741824.0;
		return QStringLiteral("%1 GBytes").arg(value, 0, 'f', 2);
	} else if (number > 1048576uLL) {
		double const value = number / 1048576.0;
		return QStringLiteral("%1 MBytes").arg(value, 0, 'f', 2);
	} else if (number > 1024uLL) {
		double const value = number / 1024.0;
		return QStringLiteral("%1 KBytes").arg(value, 0, 'f', 2);
	}
	return QStringLiteral("%1 Bytes").arg(number);
}

void MainWindow::onButtonStartStopClick() {
	if (!mEtwQuery.startTraceSession(2836)) {
		QMessageBox::warning(this, "Failed to start ETW Session", "Could not start ETW trace session, FPS data on game(s) will not be available.");
	}
	return;


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
		addLogItem(QStringLiteral("Stopped measurement after %1 iterations (%2 of log data written).").arg(mPingCounter).arg(formatSize(mBytesWritten)));
		clearStats();

		mUi->btnStartStop->setText(QStringLiteral("Start Measurement"));
		mUi->btnStartStop->setEnabled(true);
	} else {
		mUi->logWidget->clear();
		mUi->btnStartStop->setText(QStringLiteral("Starting..."));
		mUi->btnStartStop->setEnabled(false);

		mTimeStartRecord = QDateTime::currentDateTime();
		clearStats();
		mPingCounter = 0;

		// Try to open output
		QString const location = mUi->edtOutputDir->text();
		QDir topDir(location);
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
		mOutputFile.write(QStringLiteral("#     - GPU utilization, GPU dedicated memory used\r\n").toUtf8());

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
		loadStringDbg += QString("Core %1: %2").arg(i).arg(mCpuLoad.getCpuLoadOfCore(i), 2, 'f', 2);
		loadString += QString("%1").arg(mCpuLoad.getCpuLoadOfCore(i), 2, 'f', 2);
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
	updateStats();

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

			updateStats();
		}
		mRemainingPings.remove(roundId);
	}
}

void MainWindow::updateStats() {
	// Stats
	double const hours = mTimeStartRecord.msecsTo(QDateTime::currentDateTime()) / (1000.0 * 60.0 * 60.0);
	quint64 const bytesPerHour = (mBytesWritten / hours);
	mUi->lblLogStats->setText(QStringLiteral("%1 (~%2 per hour)").arg(formatSize(mBytesWritten)).arg(formatSize(bytesPerHour)));
}

void MainWindow::clearStats() {
	mUi->lblLogStats->setText(QStringLiteral(""));
	mBytesWritten = 0;
}

void MainWindow::menuAboutAboutOnClick() {
	QMessageBox::about(this, "ZeusDebug - About", QString("<h2>ZeusDebug</h2><br><br>%1<br>%2<br><br>A quick-and-dirty utility for tracking issues with a system while playing ARMA.<br><br>Copyright (C) 2020 by Philipp Berger<br>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.<br>See LICENSE for further information.<br><br>Don't be a jerk!").arg(QString::fromStdString(Version::longVersionString())).arg(QString::fromStdString(Version::buildInfo())));
}

void MainWindow::menuAboutAboutQtOnClick() {
	QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::menuFileOpenLogDirectoryOnClick() {
	const QFileInfo fileInfo(mUi->edtOutputDir->text());
	/*const QString explorer = QProcessEnvironment::systemEnvironment().searchInPath(QLatin1String("explorer.exe"));
	if (explorer.isEmpty()) {
		QMessageBox::warning(this, tr("Launching Windows Explorer Failed"), tr("Could not find explorer.exe in path to launch Windows Explorer."));
		return;
	}
	QStringList param;
	if (!fileInfo.isDir())
		param += QLatin1String("/select,");
	param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
	QProcess::startDetached(explorer.toString(), param);
		*/
}

void MainWindow::menuFileQuitOnClick() {
	QApplication::exit(0);
}
