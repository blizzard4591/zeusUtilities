#ifndef GRAPHS_FPS_H
#define GRAPHS_FPS_H

#include <cstdint>

#include <qcustomplot.h>
#include <QVector>

class GraphsFps {
public:
	static bool createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QVector<double> const& dataArmaFps, QVector<double> const& dataArmaFpsDisplayed, QVector<double> const& dataArmaLatency);
};

#endif
