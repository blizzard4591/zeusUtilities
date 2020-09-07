#include "graphs_network.h"

#include <algorithm>

#include <QColor>

bool GraphsNetwork::createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QMap<QString, QVector<double>> const& dataPingRoundTripTime) {
	if (dataTimestamps.isEmpty()) {
		return false;
	}

	// Mins and Maxes
	double const minTimestamp = *std::min_element(dataTimestamps.cbegin(), dataTimestamps.cend());
	double const maxTimestamp = *std::max_element(dataTimestamps.cbegin(), dataTimestamps.cend());

	double minRtt = std::numeric_limits<double>::max();
	double maxRtt = std::numeric_limits<double>::min();

	QVector<QColor> colors;
	colors << Qt::red << Qt::black << Qt::darkGray << Qt::green << Qt::blue << Qt::cyan << Qt::magenta << Qt::yellow << Qt::darkRed << Qt::darkGreen << Qt::darkBlue << Qt::darkCyan << Qt::darkMagenta << Qt::darkYellow;

	int i = 0;
	for (auto it = dataPingRoundTripTime.constBegin(); it != dataPingRoundTripTime.constEnd(); ++it) {
		plot->addGraph(plot->xAxis, plot->yAxis);
		
		if (i >= colors.size()) {
			// We can not assign colors, so skip it.
			continue;
		}

		plot->graph(i)->setPen(QPen(colors.at(i)));
		plot->graph(i)->setName(QString("RTT to %1").arg(it.key()));
		plot->graph(i)->setData(dataTimestamps, it.value(), true);

		if (dataTimestamps.size() != it.value().size()) {
			// sizes should all match
			// TODO: Error message.
			return false;
		}

		minRtt = std::min(minRtt, *std::min_element(it.value().cbegin(), it.value().cend()));
		maxRtt = std::max(maxRtt, *std::max_element(it.value().cbegin(), it.value().cend()));

		++i;
	}

	// configure bottom axis to show date instead of number:
	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
	dateTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy\nUTC");
	dateTicker->setDateTimeSpec(Qt::UTC);
	plot->xAxis->setTicker(dateTicker);
	plot->xAxis->setLabel("Time (UTC)");

	plot->yAxis->setLabel("Round Trip Time in ms");
	plot->xAxis2->setVisible(true);
	plot->yAxis2->setVisible(true);
	plot->xAxis2->setTicks(false);
	plot->yAxis2->setTicks(true);
	plot->xAxis2->setTickLabels(false);
	plot->yAxis2->setTickLabels(false);
	// set axis ranges to show all data:
	double const xMin = minTimestamp;
	double const xMax = maxTimestamp;
	double const xDiff = xMax - xMin;
	plot->xAxis->setRange(xMin - xDiff * 0.05, xMax + xDiff * 0.05);

	// Y1 (milliseconds)
	double const yMin = minRtt;
	double const yMax = maxRtt;
	double const yDiff = yMax - yMin;
	plot->yAxis->setRange(yMin - yDiff * 0.05, yMax + yDiff * 0.05);

	// show legend with slightly transparent background brush:
	plot->legend->setVisible(true);
	plot->legend->setBrush(QColor(255, 255, 255, 150));
	plot->replot();

	return true;
}
