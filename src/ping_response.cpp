#include "ping_response.h"

QJsonObject PingResponse::toJsonObject(bool verbose) {
	QJsonObject result;

	/*
	bool hasError;
    uint32_t errorCode;
    uint32_t roundTripTime;
    uint8_t ttl;
    QString target;
	*/
	if (verbose) {
		result.insert(QStringLiteral("hasError"), hasError);
		result.insert(QStringLiteral("errorCode"), static_cast<qint64>(errorCode));
		result.insert(QStringLiteral("roundTripTime"), static_cast<qint64>(roundTripTime));
		result.insert(QStringLiteral("ttl"), ttl);
		result.insert(QStringLiteral("target"), target);
	} else {
		result.insert(QStringLiteral("p:0"), hasError);
		result.insert(QStringLiteral("p:1"), static_cast<qint64>(errorCode));
		result.insert(QStringLiteral("p:2"), static_cast<qint64>(roundTripTime));
		result.insert(QStringLiteral("p:3"), ttl);
		result.insert(QStringLiteral("p:4"), target);
	}

	return result;
}
