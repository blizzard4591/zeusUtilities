#include "main_window_plot.h"
#include "ui_main_window_plot.h"

#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringRef>
#include <QTextStream>
#include <QProcessEnvironment>

#include <iostream>

#include "cpu_info.h"
#include "gpu_info.h"
#include "fps_info.h"
#include "round_info.h"
#include "process_info.h"
#include "ping_response.h"

#include "version.h"

#include "graphs_fps.h"
#include "graphs_memory.h"
#include "graphs_cpu.h"
#include "graphs_network.h"

MainWindowPlot::MainWindowPlot(QWidget *parent) : QMainWindow(parent), mUi(new Ui::MainWindowPlot), mIsChangingPath(false) {
    mUi->setupUi(this);

	this->setWindowTitle(QStringLiteral("ZeusOps Debug Utility Plotter v").append(QString::fromStdString(Version::versionWithTagString())));

	// Menu Items
	QObject::connect(mUi->actionAbout, SIGNAL(triggered()), this, SLOT(menuAboutAboutOnClick()));
	QObject::connect(mUi->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(menuAboutAboutQtOnClick()));
	QObject::connect(mUi->actionOpen_Log_Directory, SIGNAL(triggered()), this, SLOT(menuFileOpenLogDirectoryOnClick()));
	QObject::connect(mUi->actionQuit, SIGNAL(triggered()), this, SLOT(menuFileQuitOnClick()));

	QObject::connect(mUi->edtOutputDir, SIGNAL(textChanged(QString const&)), this, SLOT(logFileLocationChanged()));
	QObject::connect(mUi->listWidgetLogFiles, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(listWidgetLogFilesOnCurrentItemChanged()));

	QStringList locations = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
	if (locations.size() > 0) {
		QDir location(locations.at(0));
		if ((!location.exists(QStringLiteral("zeusDebug"))) || !location.cd(QStringLiteral("zeusDebug"))) {
			// Fallback
			mUi->edtOutputDir->setText(locations.at(0));
		} else {
			mUi->edtOutputDir->setText(location.absolutePath());
		}
	}

	closeLogFile();
}

MainWindowPlot::~MainWindowPlot() {
	delete mUi;
}

QString MainWindowPlot::formatSize(quint64 number) const {
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

void MainWindowPlot::menuAboutAboutOnClick() {
	QMessageBox::about(this, "ZeusDebug - About", QString("<h2>ZeusDebug</h2><br><br>%1<br>%2<br><br>A quick-and-dirty utility for tracking issues with a system while playing ARMA.<br><br>Copyright (C) 2020 by Philipp Berger<br>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.<br>See LICENSE for further information.<br><br>Don't be a jerk!").arg(QString::fromStdString(Version::longVersionString())).arg(QString::fromStdString(Version::buildInfo())));
}

void MainWindowPlot::menuAboutAboutQtOnClick() {
	QMessageBox::aboutQt(this, "About Qt");
}

void MainWindowPlot::menuFileOpenLogDirectoryOnClick() {
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

void MainWindowPlot::menuFileQuitOnClick() {
	QApplication::exit(0);
}

void MainWindowPlot::logFileLocationChanged() {
	closeLogFile();

	mIsChangingPath = true;

	mLogFiles.clear();
	mUi->listWidgetLogFiles->clear();

	QDir dir(mUi->edtOutputDir->text());
	if (dir.exists()) {
		mLogFiles = dir.entryInfoList({ "*.txt" }, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
		for (int i = 0; i < mLogFiles.size(); ++i) {
			mUi->listWidgetLogFiles->addItem(mLogFiles.at(i).fileName());
		}
	}

	mIsChangingPath = false;
}

void MainWindowPlot::listWidgetLogFilesOnCurrentItemChanged() {
	if (mIsChangingPath) {
		return;
	}
	closeLogFile();

	int const currentRow = mUi->listWidgetLogFiles->currentRow();
	if (currentRow < mLogFiles.size()) {
		mCurrentLogFile.setFileName(mLogFiles.at(currentRow).absoluteFilePath());
		if (mCurrentLogFile.open(QFile::ReadOnly)) {
			QTextStream input(&mCurrentLogFile);
			QString const log = input.readAll();
			parseLog(log);
		} else {
			QMessageBox::warning(this, tr("Error while opening log file"), tr("Could not open selected log file!\nCheck that you have not selected a privileged folder and that your AntiVirus is not blocking access."));
			std::cerr << "Failed to open file: " << mLogFiles.at(currentRow).absoluteFilePath().toStdString() << std::endl;
		}
	}
}

void MainWindowPlot::parseLog(QString const& log) {

	QVector<double> dataTimestamps;
	QVector<double> dataMemoryLoad;
	QVector<double> dataMemoryTotal;
	QVector<double> dataMemoryFree;
	
	QVector<double> dataArmaFps;
	QVector<double> dataArmaFpsDisplayed;
	QVector<double> dataArmaLatency;

	QVector<double> dataPercentUser;
	QVector<double> dataPercentKernel;
	QVector<double> dataPercentIdle;

	QMap<QString, QVector<double>> dataPingRoundTripTime;

	bool hadError = false;
	QVector<QStringRef> const lines = log.splitRef(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
	for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
		QStringRef const& line = lines.at(lineIndex);
		if (line.startsWith(QStringLiteral("#"))) {
			continue;
		}
		
		// Fix a bug in earlier version
		QString lineReplaced = line.toString();
		static const QRegularExpression regex("\"([^\"]+)\":\"(\\d+);(\\d+)\"");
		static const QString replace("\"\\1\":{\"iV\":\\2,\"iD\":\\3}");
		lineReplaced.replace(regex, replace);

		QJsonDocument const roundDoc = QJsonDocument::fromJson(lineReplaced.toUtf8());
		if (roundDoc.isNull()) {
			hadError = true;
			QMessageBox::warning(this, tr("Error while parsing log file"), tr("Could not parse the selected log file!\nLine %1 is not a valid JSON document!").arg(lineIndex));
			std::cerr << "Failed to parse line in row #" << lineIndex << "!" << std::endl;
			continue;
		}

		bool okay = true;
		RoundInfo roundInfo = RoundInfo::fromJsonDocument(roundDoc, &okay);
		if (!okay) {
			hadError = true;
			QMessageBox::warning(this, tr("Error while parsing log file"), tr("Could not parse the selected log file!\nLine %1 has invalid round information!").arg(lineIndex));
			std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid round information!" << std::endl;
			continue;
		}

		QJsonObject const& cpuStateObject = roundInfo.getCpuState();
		CpuInfo cpuInfo = CpuInfo::fromJsonObject(cpuStateObject, &okay);
		if (!okay) {
			hadError = true;
			QMessageBox::warning(this, tr("Error while parsing log file"), tr("Could not parse the selected log file!\nLine %1 has invalid cpu state information!").arg(lineIndex));
			std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid cpu state information!" << std::endl;
			continue;
		}

		QJsonObject const& armaFps = roundInfo.getArmaFps();
		FpsInfo armaFpsInfo = FpsInfo::fromJsonObject(armaFps, &okay);
		if (!okay) {
			hadError = true;
			QMessageBox::warning(this, tr("Error while parsing log file"), tr("Could not parse the selected log file!\nLine %1 has invalid FPS information!").arg(lineIndex));
			std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid fps information!" << std::endl;
			continue;
		}

		QJsonArray const& processStates = roundInfo.getProcessStates();
		for (int i = 0; i < processStates.size(); ++i) {
			QJsonObject const pState = processStates.at(i).toObject();
			ProcessInfo const processInfo = ProcessInfo::fromJsonObject(pState, &okay);
			if (!okay) {
				QMessageBox::warning(this, tr("Error while parsing log file"), tr("Could not parse the selected log file!\nLine %1 has invalid process state information at sub-index %2!").arg(lineIndex).arg(i));
				std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid process state information at sub-index " << i << "!" << std::endl;
				continue;
			}
			if (processInfo.UniqueProcessId == 0) {
				continue;
			}
		}

		QJsonArray const& pingReponses = roundInfo.getPingResponses();
		for (int i = 0; i < pingReponses.size(); ++i) {
			QJsonObject const reponse = pingReponses.at(i).toObject();
			PingResponse const pingResponse = PingResponse::fromJsonObject(reponse, &okay);
			if (!okay) {
				hadError = true;
				QMessageBox::warning(this, tr("Error while parsing log file"), tr("Could not parse the selected log file!\nLine %1 has invalid ping response information at sub-index %2!").arg(lineIndex).arg(i));
				std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid ping response information at sub-index " << i << "!" << std::endl;
				continue;
			}
			if (!dataPingRoundTripTime.contains(pingResponse.target)) {
				if (lineIndex != 0) {
					hadError = true;
					QMessageBox::warning(this, tr("Error while parsing log file"), tr("Could not parse the selected log file!\nLine %1 has ping response information at sub-index %2 that introduces a new target!").arg(lineIndex).arg(i));
					std::cerr << "Failed to parse line in row #" << lineIndex << ", ping response information at sub-index " << i << " contains new target!" << std::endl;
					continue;
				}
				dataPingRoundTripTime.insert(pingResponse.target, QVector<double>());
			}
			dataPingRoundTripTime.find(pingResponse.target).value().append(pingResponse.hasError ? -1.0 : pingResponse.roundTripTime);
		}
		

		double const key = QCPAxisTickerDateTime::dateTimeToKey(QDateTime::fromMSecsSinceEpoch(roundInfo.getStartTime().toMSecsSinceEpoch()));
		dataTimestamps.append(key);

		// Memory
		dataMemoryLoad.append(cpuInfo.memoryLoad);
		dataMemoryTotal.append(cpuInfo.memoryTotal);
		dataMemoryFree.append(cpuInfo.memoryAvail);
		
		// Arma FPS
		dataArmaFps.append(armaFpsInfo.framesPerSecond);
		dataArmaFpsDisplayed.append(armaFpsInfo.fpsDisplayed);
		dataArmaLatency.append(armaFpsInfo.latency);

		// CPU Times (global)
		double const deltaSum = cpuInfo.idleTimeDelta + cpuInfo.kernelTimeDelta + cpuInfo.userTimeDelta;
		dataPercentUser.append((cpuInfo.userTimeDelta / deltaSum) * 100.0);
		dataPercentKernel.append((cpuInfo.kernelTimeDelta / deltaSum) * 100.0);
		dataPercentIdle.append((cpuInfo.idleTimeDelta / deltaSum) * 100.0);
	}

	if (hadError) {
		return;
	}

	GraphsFps::createGraphs(mUi->plotFps, dataTimestamps, dataArmaFps, dataArmaFpsDisplayed, dataArmaLatency);
	GraphsMemory::createGraphs(mUi->plotMemory, dataTimestamps, dataMemoryLoad, dataMemoryTotal, dataMemoryFree);
	GraphsCpu::createGraphs(mUi->plotCpu, dataTimestamps, dataPercentUser, dataPercentKernel, dataPercentIdle);
	GraphsNetwork::createGraphs(mUi->plotNetwork, dataTimestamps, dataPingRoundTripTime);

	logFileOpened();
}

void MainWindowPlot::logFileOpened() {
	mUi->tabWidget->setTabEnabled(0, true);
	mUi->tabWidget->setTabEnabled(1, true);
	mUi->tabWidget->setTabEnabled(2, true);
	mUi->tabWidget->setTabEnabled(3, true);
	mUi->tabWidget->setTabEnabled(4, true);
	mUi->tabWidget->setCurrentIndex(1);
}

void MainWindowPlot::logFileClosed() {
	mUi->tabWidget->setCurrentIndex(0);
	mUi->tabWidget->setTabEnabled(0, true);
	mUi->tabWidget->setTabEnabled(1, false);
	mUi->tabWidget->setTabEnabled(2, false);
	mUi->tabWidget->setTabEnabled(3, false);
	mUi->tabWidget->setTabEnabled(4, false);
}

void MainWindowPlot::closeLogFile() {
	if (mCurrentLogFile.isOpen()) {
		mCurrentLogFile.close();
	}
	logFileClosed();

	// CPU Graph Reset
	for (int g = 0; g < mUi->plotCpu->graphCount(); ++g) {
		mUi->plotCpu->graph(g)->data().clear();
	}
	mUi->plotCpu->clearGraphs();
	mUi->plotCpu->clearPlottables();
	mUi->plotCpu->replot();

	// FPS Graph Reset
	for (int g = 0; g < mUi->plotFps->graphCount(); ++g) {
		mUi->plotFps->graph(g)->data().clear();
	}
	mUi->plotFps->clearGraphs();
	mUi->plotFps->clearPlottables();
	mUi->plotFps->replot();

	// Memory Graph Reset
	for (int g = 0; g < mUi->plotMemory->graphCount(); ++g) {
		mUi->plotMemory->graph(g)->data().clear();
	}
	mUi->plotMemory->clearGraphs();
	mUi->plotMemory->clearPlottables();
	mUi->plotMemory->replot();

	// Network Graph Reset
	for (int g = 0; g < mUi->plotNetwork->graphCount(); ++g) {
		mUi->plotNetwork->graph(g)->data().clear();
	}
	mUi->plotNetwork->clearGraphs();
	mUi->plotNetwork->clearPlottables();
	mUi->plotNetwork->replot();
}
