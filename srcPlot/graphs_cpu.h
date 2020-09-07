#ifndef GRAPHS_CPU_H
#define GRAPHS_CPU_H

#include <cstdint>

#include <qcustomplot.h>
#include <QVector>

class GraphsCpu {
public:
	static bool createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QVector<double> const& dataPercentUser, QVector<double> const& dataPercentKernel, QVector<double> const& dataPercentIdle);
};

#endif
