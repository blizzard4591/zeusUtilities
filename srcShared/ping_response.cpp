#include "ping_response.h"

#define ASSIGN_SHOULD_CONTAIN(target, fullName, shortName) if (object.contains(fullName) || object.contains(shortName)) { target = (object.contains(fullName) ? object.value(fullName) : object.value(shortName)).toInt(); } else ok = false

const QString PingResponse::TAG_HASERROR_L = QStringLiteral("hasError");
const QString PingResponse::TAG_HASERROR_S = QStringLiteral("p:0");
const QString PingResponse::TAG_ERRORCODE_L = QStringLiteral("errorCode");
const QString PingResponse::TAG_ERRORCODE_S = QStringLiteral("p:1");
const QString PingResponse::TAG_RTT_L = QStringLiteral("roundTripTime");
const QString PingResponse::TAG_RTT_S = QStringLiteral("p:2");
const QString PingResponse::TAG_TTL_L = QStringLiteral("ttl");
const QString PingResponse::TAG_TTL_S = QStringLiteral("p:3");
const QString PingResponse::TAG_TARGET_L = QStringLiteral("target");
const QString PingResponse::TAG_TARGET_S = QStringLiteral("p:4");

QJsonObject PingResponse::toJsonObject(bool verbose) {
	QJsonObject result;

	if (verbose) {
		result.insert(TAG_HASERROR_L, hasError);
		result.insert(TAG_ERRORCODE_L, static_cast<qint64>(errorCode));
		result.insert(TAG_RTT_L, static_cast<qint64>(roundTripTime));
		result.insert(TAG_TTL_L, ttl);
		result.insert(TAG_TARGET_L, target);
	} else {
		result.insert(TAG_HASERROR_S, hasError);
		result.insert(TAG_ERRORCODE_S, static_cast<qint64>(errorCode));
		result.insert(TAG_RTT_S, static_cast<qint64>(roundTripTime));
		result.insert(TAG_TTL_S, ttl);
		result.insert(TAG_TARGET_S, target);
	}

	return result;
}

PingResponse PingResponse::fromJsonObject(QJsonObject const& object, bool* okay) {
	PingResponse result;
	bool ok = true;

	if (object.contains(TAG_HASERROR_L) || object.contains(TAG_HASERROR_S)) { result.hasError = (object.contains(TAG_HASERROR_L) ? object.value(TAG_HASERROR_L) : object.value(TAG_HASERROR_S)).toBool(); } else ok = false;
	ASSIGN_SHOULD_CONTAIN(result.errorCode, TAG_ERRORCODE_L, TAG_ERRORCODE_S);
	ASSIGN_SHOULD_CONTAIN(result.roundTripTime, TAG_RTT_L, TAG_RTT_S);
	ASSIGN_SHOULD_CONTAIN(result.ttl, TAG_TTL_L, TAG_TTL_S);
	if (object.contains(TAG_TARGET_L) || object.contains(TAG_TARGET_S)) { result.target = (object.contains(TAG_TARGET_L) ? object.value(TAG_TARGET_L) : object.value(TAG_TARGET_S)).toString(); } else ok = false;

	if (okay != nullptr) {
		*okay = ok;
	}

	return result;
}
