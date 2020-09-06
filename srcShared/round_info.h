#ifndef ROUND_INFO_H
#define ROUND_INFO_H

#include <QDateTime>
#include <QMetaType>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <iostream>

/*
struct RoundInfo {
	quint64 remainingPings;
	QJsonObject outputObject;
	QString startTime;
	QJsonArray jsonPingResponses;
	QVector<QString> pingResponses;
};
*/

class RoundInfo {
public:
	RoundInfo();
	RoundInfo(QDateTime const& startTime, quint64 remainingPings, QJsonObject const& cpuState, QJsonArray const& processes, QJsonObject const& armaFps);
	
	quint64 getRemainingPings() const { return mRemainingPings; }
	void addPingResponde(QJsonObject const& pingResponse);

	QDateTime const& getStartTime() const { return mStartTime; }
	QJsonObject const& getCpuState() const { return mCpuState; }
	QJsonArray const& getProcessStates() const { return mProcesses; }
	QJsonObject const& getArmaFps() const { return mArmaFps; }

	QJsonDocument toJsonDocument(bool verbose) const;
	static RoundInfo fromJsonDocument(QJsonDocument const& document, bool* okay);
private:
	static const QString TAG_STARTTIME_L; static const QString TAG_STARTTIME_S;
	static const QString TAG_CPUINFO_L; static const QString TAG_CPUINFO_S;
	static const QString TAG_PROCESSES_L; static const QString TAG_PROCESSES_S;
	static const QString TAG_ARMAFPS_L; static const QString TAG_ARMAFPS_S;
	static const QString TAG_PINGRESPONSES_L; static const QString TAG_PINGRESPONSES_S;

	QDateTime mStartTime;
	quint64 mRemainingPings;
	QJsonObject mCpuState;
	QJsonArray mProcesses;
	QJsonObject mArmaFps;
	QJsonArray mPingResponses;
};

Q_DECLARE_METATYPE(RoundInfo)

#endif
