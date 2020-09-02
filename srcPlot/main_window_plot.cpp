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

#include "version.h"

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

	quint64 minTimestamp = 18446744073709551615uLL;
	quint64 maxTimestamp = 0;

	QVector<std::pair<double, double>> minsAndMaxes;
	for (int i = 0; i < 3; ++i) {
		minsAndMaxes.append(std::make_pair(18446744073709551615.0, 0.0));
	}

	QVector<QStringRef> const lines = log.splitRef(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
	for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
		QStringRef const& line = lines.at(lineIndex);
		if (line.startsWith(QStringLiteral("#"))) {
			continue;
		}
		QVector<QStringRef> const parts = line.split(QChar(';'));
		/*
		
		# Type 1: State information
		#    Fields:
		#     - Time in msecs since epoch
		#     - userDelta, kernelDelta, idleDelta (100 nanosecond resolution)
		#     - memory load, total mem, free mem, total page, free page, total virt, free virt (in bytes)
		#     - RTT and TTL for pings to all targets (8.8.8.8;ts.zeusops.com)
		# Type 2: Processes information
		#    Fields:
		#     - PID, ImageName
		#     - UserTime, KernelTime
		#     - WorkingSetSize, PeakWorkingSetSize (both as value and delta to last)
		#     - GPU utilization, GPU dedicated memory used
		*/
		if (parts.at(0).compare(QStringLiteral("1")) == 0) {
			// State
			quint64 const timestamp = parts.at(1).toULongLong();
			quint64 const userDelta = parts.at(2).toULongLong();
			quint64 const kernelDelta = parts.at(3).toULongLong();
			quint64 const idleDelta = parts.at(4).toULongLong();

			quint64 const memoryLoad = parts.at(5).toULongLong();
			quint64 const memTotal = parts.at(6).toULongLong();
			quint64 const memFree = parts.at(7).toULongLong();
			quint64 const pageTotal = parts.at(8).toULongLong();
			quint64 const pageFree = parts.at(9).toULongLong();
			quint64 const virtTotal = parts.at(10).toULongLong();
			quint64 const virtFree = parts.at(11).toULongLong();

			minTimestamp = std::min(minTimestamp, timestamp);
			maxTimestamp = std::max(maxTimestamp, timestamp);

			//mUi->customPlotWidget->addGraph();
			double const key = QCPAxisTickerDateTime::dateTimeToKey(QDateTime::fromMSecsSinceEpoch(timestamp));

			dataTimestamps.append(key);
			dataMemoryLoad.append(memoryLoad);
			updateMinMax(minsAndMaxes[0], memoryLoad);
			dataMemoryTotal.append(memTotal);
			updateMinMax(minsAndMaxes[1], memTotal);
			dataMemoryFree.append(memFree);
			updateMinMax(minsAndMaxes[2], memFree);
			//std::cout << "Parsed a line." << std::endl;
		} else if (parts.at(0).compare(QStringLiteral("2")) == 0) {
			// Process Information
		}
	}

	mUi->customPlotWidget->addGraph(mUi->customPlotWidget->xAxis, mUi->customPlotWidget->yAxis);
	mUi->customPlotWidget->addGraph(mUi->customPlotWidget->xAxis, mUi->customPlotWidget->yAxis);
	mUi->customPlotWidget->addGraph(mUi->customPlotWidget->xAxis, mUi->customPlotWidget->yAxis2);

	{
		QColor color(Qt::red);
		mUi->customPlotWidget->graph(0)->setLineStyle(QCPGraph::lsLine);
		//mUi->customPlotWidget->graph(0)->setPen(QPen(color.lighter(200)));
		//mUi->customPlotWidget->graph(0)->setBrush(QBrush(color));
		mUi->customPlotWidget->graph(0)->setName("Free Memory");
		mUi->customPlotWidget->graph(0)->setData(dataTimestamps, dataMemoryFree, true);
	}
	{
		QColor color(Qt::blue);
		//mUi->customPlotWidget->graph(1)->setLineStyle(QCPGraph::lsLine);
		mUi->customPlotWidget->graph(1)->setPen(QPen(color.lighter(200)));
		//mUi->customPlotWidget->graph(1)->setBrush(QBrush(color));
		mUi->customPlotWidget->graph(1)->setName("Total Memory");
		mUi->customPlotWidget->graph(1)->setData(dataTimestamps, dataMemoryTotal, true);
	}
	{
		QColor color(Qt::yellow);
		//mUi->customPlotWidget->graph(2)->setLineStyle(QCPGraph::lsLine);
		mUi->customPlotWidget->graph(2)->setPen(QPen(color.lighter(200)));
		//mUi->customPlotWidget->graph(2)->setBrush(QBrush(color));
		mUi->customPlotWidget->graph(2)->setName("Memory Load");
		mUi->customPlotWidget->graph(2)->setData(dataTimestamps, dataMemoryTotal, true);
	}

	// configure bottom axis to show date instead of number:
	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
	dateTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy");
	dateTicker->setDateTimeSpec(Qt::UTC);
	mUi->customPlotWidget->xAxis->setTicker(dateTicker);
	mUi->customPlotWidget->xAxis->setLabel("Time");

	mUi->customPlotWidget->yAxis->setLabel("Load Values");
	mUi->customPlotWidget->yAxis->setLabel("Utilization in %");
	// make top and right axes visible but without ticks and labels:
	mUi->customPlotWidget->xAxis2->setVisible(true);
	mUi->customPlotWidget->yAxis2->setVisible(true);
	mUi->customPlotWidget->xAxis2->setTicks(false);
	mUi->customPlotWidget->yAxis2->setTicks(false);
	mUi->customPlotWidget->xAxis2->setTickLabels(false);
	mUi->customPlotWidget->yAxis2->setTickLabels(false);
	// set axis ranges to show all data:
	double const xMin = QCPAxisTickerDateTime::dateTimeToKey(QDateTime::fromMSecsSinceEpoch(minTimestamp));
	double const xMax = QCPAxisTickerDateTime::dateTimeToKey(QDateTime::fromMSecsSinceEpoch(maxTimestamp));
	double const xDiff = xMax - xMin;
	mUi->customPlotWidget->xAxis->setRange(xMin - xDiff * 0.1, xMax + xDiff * 0.1);

	double const yMin = std::min(std::min(minsAndMaxes[0].first, minsAndMaxes[1].first), minsAndMaxes[2].first);
	double const yMax = std::max(std::max(minsAndMaxes[0].second, minsAndMaxes[1].second), minsAndMaxes[2].second);
	double const yDiff = yMax - yMin;
	mUi->customPlotWidget->yAxis->setRange(yMin - yDiff * 0.1, yMax + yDiff * 0.1);
	// show legend with slightly transparent background brush:
	mUi->customPlotWidget->legend->setVisible(true);
	mUi->customPlotWidget->legend->setBrush(QColor(255, 255, 255, 150));
	mUi->customPlotWidget->replot();
}
