#ifndef TICKER_BYTEFORMAT_H
#define TICKER_BYTEFORMAT_H

#include "qcustomplot.h"

#include <iostream>

class QCP_LIB_DECL QCPAxisTickerByteFormat : public QCPAxisTicker {
public:
	QCPAxisTickerByteFormat();

	virtual void generate(const QCPRange& range, const QLocale& locale, QChar formatChar, int precision, QVector<double>& ticks, QVector<double>* subTicks, QVector<QString>* tickLabels) override;
protected:
	virtual QString getTickLabel(double tick, const QLocale& locale, QChar formatChar, int precision) Q_DECL_OVERRIDE;

	virtual double getTickStep(const QCPRange& range) override;
	double pickClosestBytes(double target, const QVector<double>& candidates) const;
	double getMantissaBytes(double input, double* magnitude = 0) const;
	double cleanMantissaBytes(double input) const;
};

#endif
