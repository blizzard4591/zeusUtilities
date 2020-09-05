#include "round_info.h"

#define ASSIGN_SHOULD_CONTAIN(target, fullName, shortName) if (object.contains(fullName) || object.contains(shortName)) { target = (object.contains(fullName) ? object.value(fullName) : object.value(shortName)).toString().toLongLong(&subOk); ok &= subOk; } else throw

const QString RoundInfo::TAG_STARTTIME_L = QStringLiteral("startTime");
const QString RoundInfo::TAG_STARTTIME_S = QStringLiteral("0:0");
const QString RoundInfo::TAG_CPUINFO_L = QStringLiteral("cpuState");
const QString RoundInfo::TAG_CPUINFO_S = QStringLiteral("0:1");
const QString RoundInfo::TAG_PROCESSES_L = QStringLiteral("processes");
const QString RoundInfo::TAG_PROCESSES_S = QStringLiteral("0:2");
const QString RoundInfo::TAG_ARMAFPS_L = QStringLiteral("armaFps");
const QString RoundInfo::TAG_ARMAFPS_S = QStringLiteral("0:3");
const QString RoundInfo::TAG_PINGRESPONSES_L = QStringLiteral("pings");
const QString RoundInfo::TAG_PINGRESPONSES_S = QStringLiteral("0:4");

RoundInfo::RoundInfo() : mStartTime(), mRemainingPings(0) {
	//
}

RoundInfo::RoundInfo(QDateTime const& startTime, quint64 remainingPings, QJsonObject const& cpuState, QJsonArray const& processes, QJsonObject const& armaFps) : mStartTime(startTime), mRemainingPings(remainingPings), mCpuState(cpuState), mProcesses(processes), mArmaFps(armaFps) {
    //
}

void RoundInfo::addPingResponde(QJsonObject const& pingResponse) {
    mPingResponses.append(pingResponse);
    --mRemainingPings;
}

QJsonDocument RoundInfo::toJsonDocument(bool verbose) const {
    QJsonObject result;
    if (verbose) {
        result.insert(TAG_STARTTIME_L, QString::number(mStartTime.toMSecsSinceEpoch()));
        result.insert(TAG_CPUINFO_L, mCpuState);
        result.insert(TAG_PROCESSES_L, mProcesses);
        result.insert(TAG_ARMAFPS_L, mArmaFps);
        result.insert(TAG_PINGRESPONSES_L, mPingResponses);
    } else {
        result.insert(TAG_STARTTIME_S, QString::number(mStartTime.toMSecsSinceEpoch()));
        result.insert(TAG_CPUINFO_S, mCpuState);
        result.insert(TAG_PROCESSES_S, mProcesses);
        result.insert(TAG_ARMAFPS_S, mArmaFps);
        result.insert(TAG_PINGRESPONSES_S, mPingResponses);
    }
    return QJsonDocument(result);
}

RoundInfo RoundInfo::fromJsonDocument(QJsonDocument const& document, bool* okay) {
    RoundInfo result;
    if (!document.isObject()) {
        *okay = false;
        return result;
    }
    QJsonObject const object = document.object();
	bool ok = true;
    if (object.contains(TAG_STARTTIME_L) || object.contains(TAG_STARTTIME_S)) { result.mStartTime = QDateTime::fromMSecsSinceEpoch((object.contains(TAG_STARTTIME_L) ? object.value(TAG_STARTTIME_L) : object.value(TAG_STARTTIME_S)).toString().toULongLong(&ok)); } else ok = false;

    if (object.contains(TAG_CPUINFO_L) || object.contains(TAG_CPUINFO_S)) { result.mCpuState = (object.contains(TAG_CPUINFO_L) ? object.value(TAG_CPUINFO_L) : object.value(TAG_CPUINFO_S)).toObject(); } else ok = false;
    if (object.contains(TAG_PROCESSES_L) || object.contains(TAG_PROCESSES_S)) { result.mProcesses = (object.contains(TAG_PROCESSES_L) ? object.value(TAG_PROCESSES_L) : object.value(TAG_PROCESSES_S)).toArray(); } else ok = false;
    if (object.contains(TAG_ARMAFPS_L) || object.contains(TAG_ARMAFPS_S)) { result.mArmaFps = (object.contains(TAG_ARMAFPS_L) ? object.value(TAG_ARMAFPS_L) : object.value(TAG_ARMAFPS_S)).toObject(); } else ok = false;
    if (object.contains(TAG_PINGRESPONSES_L) || object.contains(TAG_PINGRESPONSES_S)) { result.mPingResponses = (object.contains(TAG_PINGRESPONSES_L) ? object.value(TAG_PINGRESPONSES_L) : object.value(TAG_PINGRESPONSES_S)).toArray(); } else ok = false;

	if (okay != nullptr) {
		*okay = ok;
	}

	return result;
}
