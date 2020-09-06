#include "graphs_fps.h"

#include <algorithm>

#include <QColor>

bool GraphsFps::createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QVector<double> const& dataArmaFps, QVector<double> const& dataArmaFpsDisplayed, QVector<double> const& dataArmaLatency) {
	{
		plot->addGraph(plot->xAxis, plot->yAxis);
		QColor color(Qt::red);
		plot->graph(0)->setPen(QPen(color));
		plot->graph(0)->setName("ARMA FPS");
		plot->graph(0)->setData(dataTimestamps, dataArmaFps, true);
	}
	{
		plot->addGraph(plot->xAxis, plot->yAxis);
		QColor color(Qt::darkMagenta);
		plot->graph(1)->setPen(QPen(color));
		plot->graph(1)->setName("ARMA FPS Displayed");
		plot->graph(1)->setData(dataTimestamps, dataArmaFpsDisplayed, true);
	}
	{
		plot->addGraph(plot->xAxis, plot->yAxis2);
		QColor color(Qt::lightGray);
		plot->graph(2)->setPen(QPen(color));
		plot->graph(2)->setName("ARMA Latency");
		plot->graph(2)->setData(dataTimestamps, dataArmaLatency, true);
	}

	if (dataTimestamps.isEmpty()) {
		return false;
	} else if ((dataTimestamps.size() != dataArmaFps.size()) || (dataArmaFps.size() != dataArmaFpsDisplayed.size()) || (dataArmaFpsDisplayed.size() != dataArmaLatency.size())) {
		// sizes should all match
		// TODO: Error message.
		return false;
	}

	// Mins and Maxes
	double const minTimestamp = *std::min_element(dataTimestamps.cbegin(), dataTimestamps.cend());
	double const maxTimestamp = *std::max_element(dataTimestamps.cbegin(), dataTimestamps.cend());
	double const minFps = *std::min_element(dataArmaFps.cbegin(), dataArmaFps.cend());
	double const maxFps = *std::max_element(dataArmaFps.cbegin(), dataArmaFps.cend());
	double const minFpsDisplayed = *std::min_element(dataArmaFpsDisplayed.cbegin(), dataArmaFpsDisplayed.cend());
	double const maxFpsDisplayed = *std::max_element(dataArmaFpsDisplayed.cbegin(), dataArmaFpsDisplayed.cend());
	double const minLatency = *std::min_element(dataArmaLatency.cbegin(), dataArmaLatency.cend());
	double const maxLatency = *std::max_element(dataArmaLatency.cbegin(), dataArmaLatency.cend());

	// configure bottom axis to show date instead of number:
	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
	dateTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy\nUTC");
	dateTicker->setDateTimeSpec(Qt::UTC);
	plot->xAxis->setTicker(dateTicker);
	plot->xAxis->setLabel("Time (UTC)");

	plot->yAxis->setLabel("Frames per Second");
	plot->yAxis2->setLabel("Latency in ms");
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
	double const yMin = std::min(minFps, minFpsDisplayed);
	double const yMax = std::max(maxFps, maxFpsDisplayed);
	double const yDiff = yMax - yMin;
	plot->yAxis->setRange(yMin, yMax + yDiff * 0.1);

	// Y2 (milliseconds)
	double const y2Min = minLatency;
	double const y2Max = maxLatency;
	double const y2Diff = y2Max - y2Min;
	plot->yAxis2->setRange(y2Min, y2Max + y2Diff * 0.1);

	// show legend with slightly transparent background brush:
	plot->legend->setVisible(true);
	plot->legend->setBrush(QColor(255, 255, 255, 150));
	plot->replot();

	return true;
}
