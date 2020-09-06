#include "graphs_memory.h"

#include <algorithm>

#include <QColor>

#include "ticker_byteFormat.h"

bool GraphsMemory::createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QVector<double> const& dataMemoryLoad, QVector<double> const& dataMemoryTotal, QVector<double> const& dataMemoryFree) {
	// configure bottom axis to show date instead of number:
	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
	dateTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy\nUTC");
	dateTicker->setDateTimeSpec(Qt::UTC);
	plot->xAxis->setTicker(dateTicker);
	plot->xAxis->setLabel("Time (UTC)");

	QSharedPointer<QCPAxisTickerByteFormat> byteTicker(new QCPAxisTickerByteFormat());
	plot->yAxis->setTicker(byteTicker);
	/*
	QMap<double, QString> ticks;
	QVector<double> positions;
	QVector<QString> labels;
	for (int i = 0; i < 17; ++i) {
		double pos = i * 1024.0 * 1024.0 * 1024.0;
		QString const label = QString("%1 GB").arg(i);
		ticks.insert(pos, label);
		positions.append(pos);
		labels.append(label);
	}
	byteTicker->setTicks(positions, labels);
	//byteTicker->addTicks(ticks);
	plot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
	plot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
	*/

	plot->yAxis->setVisible(false);
	plot->yAxis->setTicks(false);
	plot->yAxis->setTickLabels(false);
	plot->yAxis->setVisible(true);
	plot->yAxis->setTicks(true);
	plot->yAxis->setTickLabels(true);

	{
		plot->addGraph(plot->xAxis, plot->yAxis);
		QColor color(Qt::red);
		plot->graph(0)->setPen(QPen(color));
		plot->graph(0)->setName("Free Memory");
		plot->graph(0)->setData(dataTimestamps, dataMemoryFree, true);
	}
	{
		plot->addGraph(plot->xAxis, plot->yAxis);
		QColor color(Qt::darkMagenta);
		plot->graph(1)->setPen(QPen(color));
		plot->graph(1)->setName("Total Memory");
		plot->graph(1)->setData(dataTimestamps, dataMemoryTotal, true);
	}
	{
		plot->addGraph(plot->xAxis, plot->yAxis2);
		QColor color(Qt::lightGray);
		plot->graph(2)->setPen(QPen(color));
		plot->graph(2)->setName("Memory Load in %");
		plot->graph(2)->setData(dataTimestamps, dataMemoryLoad, true);
	}

	if (dataTimestamps.isEmpty()) {
		return false;
	} else if ((dataTimestamps.size() != dataMemoryFree.size()) || (dataMemoryFree.size() != dataMemoryTotal.size()) || (dataMemoryTotal.size() != dataMemoryLoad.size())) {
		// sizes should all match
		// TODO: Error message.
		return false;
	}

	// Mins and Maxes
	double const minTimestamp = *std::min_element(dataTimestamps.cbegin(), dataTimestamps.cend());
	double const maxTimestamp = *std::max_element(dataTimestamps.cbegin(), dataTimestamps.cend());
	double const minFree = *std::min_element(dataMemoryFree.cbegin(), dataMemoryFree.cend());
	double const maxFree = *std::max_element(dataMemoryFree.cbegin(), dataMemoryFree.cend());
	double const minTotal = *std::min_element(dataMemoryTotal.cbegin(), dataMemoryTotal.cend());
	double const maxTotal = *std::max_element(dataMemoryTotal.cbegin(), dataMemoryTotal.cend());
	double const minLoad = *std::min_element(dataMemoryLoad.cbegin(), dataMemoryLoad.cend());
	double const maxLoad = *std::max_element(dataMemoryLoad.cbegin(), dataMemoryLoad.cend());

	plot->yAxis->setLabel("Amount in Bytes");
	plot->yAxis2->setLabel("Load in Percent");
	// make top and right axes visible but without ticks and labels:
	plot->xAxis2->setVisible(true);
	plot->yAxis2->setVisible(true);
	plot->xAxis2->setTicks(false);
	plot->yAxis2->setTicks(true);
	plot->xAxis2->setTickLabels(false);
	plot->yAxis2->setTickLabels(true);
	// set axis ranges to show all data:
	double const xMin = minTimestamp;
	double const xMax = maxTimestamp;
	double const xDiff = xMax - xMin;
	plot->xAxis->setRange(xMin - xDiff * 0.05, xMax + xDiff * 0.05);

	// Y1 (frames per second)
	double const yMin = std::min(minFree, minTotal);
	double const yMax = std::max(maxFree, maxTotal);
	double const yDiff = yMax - yMin;
	plot->yAxis->setRange(yMin, yMax + yDiff * 0.1);

	// Y2 (milliseconds)
	double const y2Min = minLoad;
	double const y2Max = maxLoad;
	double const y2Diff = y2Max - y2Min;
	plot->yAxis2->setRange(y2Min, y2Max + y2Diff * 0.1);

	// show legend with slightly transparent background brush:
	plot->legend->setVisible(true);
	plot->legend->setBrush(QColor(255, 255, 255, 150));
	plot->replot();

	return true;
}
