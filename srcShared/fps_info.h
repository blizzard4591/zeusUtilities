#ifndef FPS_INFO_H
#define FPS_INFO_H

#include "present_mode.h"

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
	static const QString TAG_MSPERFRAME_L; static const QString TAG_MSPERFRAME_S;
	static const QString TAG_FPS_L; static const QString TAG_FPS_S;
	static const QString TAG_FPSDISPLAYED_L; static const QString TAG_FPSDISPLAYED_S;
	static const QString TAG_LATENCY_L; static const QString TAG_LATENCY_S;
	static const QString TAG_PRESENTMODE_L; static const QString TAG_PRESENTMODE_S;

	static int PresentModeToInt(PresentMode const& presentMode);
	static PresentMode IntToPresentMode(int presentMode);
};

Q_DECLARE_METATYPE(FpsInfo)

#endif
