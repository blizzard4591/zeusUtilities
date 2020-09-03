#ifndef FPS_INFO_H
#define FPS_INFO_H

#include "etw_trace_consumer.h"

#include <QMetaType>
#include <QJsonObject>
#include <QString>

#include <iostream>

class FpsInfo {
public:
	FpsInfo();
	void reset();

	double msPerFrame;
	double framesPerSecond;
	double fpsDisplayed;
	double latency;
	PresentMode presentMode;

	QJsonObject toJsonObject(bool verbose) const;
	friend std::ostream& operator<<(std::ostream& os, const FpsInfo& fi);

	static QString PresentModeToString(PresentMode const& mode);
};

Q_DECLARE_METATYPE(FpsInfo)

#endif
