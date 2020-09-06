#include "ticker_byteFormat.h"

#include <cmath>
#include <iostream>


QCPAxisTickerByteFormat::QCPAxisTickerByteFormat() : QCPAxisTicker() {
    //
}

QString QCPAxisTickerByteFormat::getTickLabel(double tick, const QLocale& locale, QChar formatChar, int precision) {
    Q_UNUSED(locale);
    Q_UNUSED(formatChar);
    Q_UNUSED(precision);
    std::cout << "Tick = " << tick << ", formatChar = " << formatChar.toLatin1() << ", precision = " << precision << std::endl;

    bool const isDivisible = fmod(tick, 1024.0) < 1.0;

    if (tick >= (1024.0 * 1024.0 * 1024.0 * 1024.0)) {
        return QStringLiteral("%1 TB").arg(tick / 1024.0 / 1024.0 / 1024.0 / 1024.0, 0, 'f', (isDivisible) ? 0 : 3);
    } else if (tick >= (1024.0 * 1024.0 * 1024.0)) {
        return QStringLiteral("%1 GB").arg(tick / 1024.0 / 1024.0 / 1024.0, 0, 'f', (isDivisible) ? 0 : 3);
    } else if (tick >= (1024.0 * 1024.0)) {
        return QStringLiteral("%1 MB").arg(tick / 1024.0 / 1024.0, 0, 'f', (isDivisible) ? 0 : 3);
    } else if (tick >= (1024.0)) {
        return QStringLiteral("%1 KB").arg(tick / 1024.0, 0, 'f', (isDivisible) ? 0 : 3);
    } else {
        return QString("%1 Bytes").arg(tick, 0, 'f', 0);
    }
}

void QCPAxisTickerByteFormat::generate(const QCPRange& range, const QLocale& locale, QChar formatChar, int precision, QVector<double>& ticks, QVector<double>* subTicks, QVector<QString>* tickLabels) {
    // generate (major) ticks:
    double tickStep = getTickStep(range);
    ticks = createTickVector(tickStep, range);
    trimTicks(range, ticks, true); // trim ticks to visible range plus one outer tick on each side (incase a subclass createTickVector creates more)

    // generate sub ticks between major ticks:
    if (subTicks) {
        if (ticks.size() > 0) {
            *subTicks = createSubTickVector(getSubTickCount(tickStep), ticks);
            trimTicks(range, *subTicks, false);
        } else
            *subTicks = QVector<double>();
    }

    // finally trim also outliers (no further clipping happens in axis drawing):
    trimTicks(range, ticks, false);
    // generate labels for visible ticks if requested:
    if (tickLabels)
        *tickLabels = createLabelVector(ticks, locale, formatChar, precision);
}

double QCPAxisTickerByteFormat::getTickStep(const QCPRange& range) {
    double exactStep = range.size() / (double)(mTickCount + 1e-10); // mTickCount ticks on average, the small addition is to prevent jitter on exact integers
    return cleanMantissaBytes(exactStep);
}

double QCPAxisTickerByteFormat::getMantissaBytes(double input, double* magnitude) const {
    const double mag = qPow(1024.0, qFloor(qLn(input) / qLn(1024.0)));
    if (magnitude) *magnitude = mag;
    return input / mag;
}

double QCPAxisTickerByteFormat::cleanMantissaBytes(double input) const {
    double magnitude;
    const double mantissa = getMantissaBytes(input, &magnitude);
    switch (mTickStepStrategy) {
    case tssReadability:
    {
        std::cout << "Magnitude is " << magnitude << " on input " << input << "." << std::endl;
        return pickClosestBytes(mantissa, QVector<double>() << 1.0 << 2.0 << 2.5 << 5.0 << 10.0) * magnitude;
    }
    case tssMeetTickCount:
    {
        // this gives effectively a mantissa of 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 6.0, 8.0, 10.0
        if (mantissa <= 5.0)
            return (int)(mantissa * 2) / 2.0 * magnitude; // round digit after decimal point to 0.5
        else
            return (int)(mantissa / 2.0) * 2.0 * magnitude; // round to first digit in multiples of 2
    }
    }
    return input;
}

double QCPAxisTickerByteFormat::pickClosestBytes(double target, const QVector<double>& candidates) const {
    if (candidates.size() == 1)
        return candidates.first();
    QVector<double>::const_iterator it = std::lower_bound(candidates.constBegin(), candidates.constEnd(), target);
    if (it == candidates.constEnd())
        return *(it - 1);
    else if (it == candidates.constBegin())
        return *it;
    else
        return target - *(it - 1) < *it - target ? *(it - 1) : *it;
}
