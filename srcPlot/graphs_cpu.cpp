#include "graphs_cpu.h"

#include <algorithm>

#include <QColor>

bool GraphsCpu::createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QVector<double> const& dataPercentUser, QVector<double> const& dataPercentKernel, QVector<double> const& dataPercentIdle) {
	{
		plot->addGraph(plot->xAxis, plot->yAxis);
		QColor color(Qt::red);
		plot->graph(0)->setPen(QPen(color));
		plot->graph(0)->setName("CPU % User");
		plot->graph(0)->setData(dataTimestamps, dataPercentUser, true);
	}
	{
		plot->addGraph(plot->xAxis, plot->yAxis);
		QColor color(Qt::darkMagenta);
		plot->graph(1)->setPen(QPen(color));
		plot->graph(1)->setName("CPU % Kernel");
		plot->graph(1)->setData(dataTimestamps, dataPercentKernel, true);
	}
	{
		plot->addGraph(plot->xAxis, plot->yAxis2);
		QColor color(Qt::lightGray);
		plot->graph(2)->setPen(QPen(color));
		plot->graph(2)->setName("CPU % Idle");
		plot->graph(2)->setData(dataTimestamps, dataPercentIdle, true);
	}

	if (dataTimestamps.isEmpty()) {
		return false;
	} else if ((dataTimestamps.size() != dataPercentUser.size()) || (dataPercentUser.size() != dataPercentKernel.size()) || (dataPercentKernel.size() != dataPercentIdle.size())) {
		// sizes should all match
		// TODO: Error message.
		return false;
	}

	// Mins and Maxes
	double const minTimestamp = *std::min_element(dataTimestamps.cbegin(), dataTimestamps.cend());
	double const maxTimestamp = *std::max_element(dataTimestamps.cbegin(), dataTimestamps.cend());
	
	// configure bottom axis to show date instead of number:
	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
	dateTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy\nUTC");
	dateTicker->setDateTimeSpec(Qt::UTC);
	plot->xAxis->setTicker(dateTicker);
	plot->xAxis->setLabel("Time (UTC)");

	plot->yAxis->setLabel("Time in Percent");
	/*plot->yAxis2->setLabel("Latency in ms");
	// make top and right axes visible but without ticks and labels:
	plot->xAxis2->setVisible(true);
	plot->yAxis2->setVisible(true);
	plot->xAxis2->setTicks(false);
	plot->yAxis2->setTicks(true);
	plot->xAxis2->setTickLabels(false);
	plot->yAxis2->setTickLabels(true);*/
	// set axis ranges to show all data:
	double const xMin = minTimestamp;
	double const xMax = maxTimestamp;
	double const xDiff = xMax - xMin;
	plot->xAxis->setRange(xMin - xDiff * 0.05, xMax + xDiff * 0.05);

	// Y1 (time in percent)
	plot->yAxis->setRange(0.0, 100.0);

	// show legend with slightly transparent background brush:
	plot->legend->setVisible(true);
	plot->legend->setBrush(QColor(255, 255, 255, 150));
	plot->replot();

	return true;
}
