#include "main_window_plot.h"
#include "ui_main_window_plot.h"

#include <QDateTime>
#include <QDir>
#include <QMessageBox>
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

#include "version.h"

#include "graphs_fps.h"

MainWindowPlot::MainWindowPlot(QWidget *parent) : QMainWindow(parent), mUi(new Ui::MainWindowPlot) {
    mUi->setupUi(this);

	this->setWindowTitle(QStringLiteral("ZeusOps Debug Utility Plotter v").append(QString::fromStdString(Version::versionWithTagString())));

	// Menu Items
	QObject::connect(mUi->actionAbout, SIGNAL(triggered()), this, SLOT(menuAboutAboutOnClick()));
	QObject::connect(mUi->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(menuAboutAboutQtOnClick()));
	QObject::connect(mUi->actionOpen_Log_Directory, SIGNAL(triggered()), this, SLOT(menuFileOpenLogDirectoryOnClick()));
	QObject::connect(mUi->actionQuit, SIGNAL(triggered()), this, SLOT(menuFileQuitOnClick()));

	QObject::connect(mUi->btnOpenLog, SIGNAL(clicked(bool)), this, SLOT(listWidgetLogFilesOnCurrentItemChanged()));
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
	mUi->listWidgetLogFiles->clear();
	mLogFiles.clear();

	QDir dir(mUi->edtOutputDir->text());
	if (dir.exists()) {
		mLogFiles = dir.entryInfoList({ "*.txt" }, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
		for (int i = 0; i < mLogFiles.size(); ++i) {
			mUi->listWidgetLogFiles->addItem(mLogFiles.at(i).fileName());
		}
	}
}

void MainWindowPlot::listWidgetLogFilesOnCurrentItemChanged() {
	if (mCurrentLogFile.isOpen()) {
		mCurrentLogFile.close();
	}

	int const currentRow = mUi->listWidgetLogFiles->currentRow();
	if (currentRow < mLogFiles.size()) {
		mCurrentLogFile.setFileName(mLogFiles.at(currentRow).absoluteFilePath());
		if (mCurrentLogFile.open(QFile::ReadOnly)) {
			QTextStream input(&mCurrentLogFile);
			QString const log = input.readAll();
			parseLog(log);
		} else {
			std::cerr << "Failed to open file: " << mLogFiles.at(currentRow).absoluteFilePath().toStdString() << std::endl;
		}
	}
}

void updateMinMax(std::pair<double, double>& pair, double value) {
	pair.first = std::min(pair.first, value);
	pair.second = std::max(pair.second, value);
}

void MainWindowPlot::parseLog(QString const& log) {

	QVector<double> dataTimestamps;
	QVector<double> dataMemoryLoad;
	QVector<double> dataMemoryTotal;
	QVector<double> dataMemoryFree;
	
	QVector<double> dataArmaFps;
	QVector<double> dataArmaFpsDisplayed;
	QVector<double> dataArmaLatency;

	QVector<QStringRef> const lines = log.splitRef(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
	for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
		QStringRef const& line = lines.at(lineIndex);
		if (line.startsWith(QStringLiteral("#"))) {
			continue;
		}
		
		QJsonDocument const roundDoc = QJsonDocument::fromJson(line.toUtf8());
		if (roundDoc.isNull()) {
			std::cerr << "Failed to parse line in row #" << lineIndex << "!" << std::endl;
			continue;
		}

		bool okay = true;
		RoundInfo roundInfo = RoundInfo::fromJsonDocument(roundDoc, &okay);
		if (!okay) {
			std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid round information!" << std::endl;
			continue;
		}

		QJsonObject const& cpuStateObject = roundInfo.getCpuState();
		CpuInfo cpuInfo = CpuInfo::fromJsonObject(cpuStateObject, &okay);
		if (!okay) {
			std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid cpu state information!" << std::endl;
			continue;
		}

		QJsonObject const& armaFps = roundInfo.getArmaFps();
		FpsInfo armaFpsInfo = FpsInfo::fromJsonObject(armaFps, &okay);
		if (!okay) {
			std::cerr << "Failed to parse line in row #" << lineIndex << ", invalid fps information!" << std::endl;
			continue;
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
	}

	GraphsFps::createGraphs(mUi->plotFps, dataTimestamps, dataArmaFps, dataArmaFpsDisplayed, dataArmaLatency);
}
