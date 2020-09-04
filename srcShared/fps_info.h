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
	static FpsInfo fromJsonObject(QJsonObject const& object, bool* okay);

	friend std::ostream& operator<<(std::ostream& os, const FpsInfo& fi);

	static QString PresentModeToString(PresentMode const& mode);
private:
	static int PresentModeToInt(PresentMode const& presentMode);
	static PresentMode IntToPresentMode(int presentMode);
};

Q_DECLARE_METATYPE(FpsInfo)

#endif
