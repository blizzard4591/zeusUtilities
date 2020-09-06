#ifndef GRAPHS_MEMORY_H
#define GRAPHS_MEMORY_H

#include <cstdint>

#include <qcustomplot.h>
#include <QVector>

class GraphsMemory {
public:
	static bool createGraphs(QCustomPlot* plot, QVector<double> const& dataTimestamps, QVector<double> const& dataMemoryLoad, QVector<double> const& dataMemoryTotal, QVector<double> const& dataMemoryFree);
};

#endif
