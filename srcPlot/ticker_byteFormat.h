#ifndef TICKER_BYTEFORMAT_H
#define TICKER_BYTEFORMAT_H

#include "qcustomplot.h"

#include <iostream>

class QCPAxisTickerByteFormat : public QCPAxisTickerText {
	Q_GADGET
public:
	QCPAxisTickerByteFormat() : QCPAxisTickerText() {
		//
	}

	virtual ~QCPAxisTickerByteFormat() {
		//
	}
protected:
	virtual QString getTickLabel(double tick, const QLocale& locale, QChar formatChar, int precision) Q_DECL_OVERRIDE {
		std::cout << "Tick = " << tick << ", formatChar = " << formatChar.toLatin1() << ", precision = " << precision << std::endl;
		return QCPAxisTickerText::getTickLabel(tick, locale, formatChar, precision);
	}
};

#endif
