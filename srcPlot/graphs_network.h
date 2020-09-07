#ifndef GRAPHS_NETWORK_H
#define GRAPHS_NETWORK_H

#include <cstdint>

#include <qcustomplot.h>

#include <QMap>
#include <QString>
#include <QVector>

class GraphsNetwork {
public:
	static bool createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QMap<QString, QVector<double>> const& dataPingRoundTripTime);
};

#endif
