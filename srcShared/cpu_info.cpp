#include "cpu_info.h"

#define ASSIGN_SHOULD_CONTAIN(target, fullName, shortName) if (object.contains(fullName) || object.contains(shortName)) { target = (object.contains(fullName) ? object.value(fullName) : object.value(shortName)).toString().toLongLong(&subOk); ok &= subOk; } else throw

const QString CpuInfo::TAG_USERDELTA_L = QStringLiteral("userDelta");
const QString CpuInfo::TAG_USERDELTA_S = QStringLiteral("c:0");
const QString CpuInfo::TAG_KERNELDELTA_L = QStringLiteral("kernelDelta");
const QString CpuInfo::TAG_KERNELDELTA_S = QStringLiteral("c:1");
const QString CpuInfo::TAG_IDLEDELTA_L = QStringLiteral("idleDelta");
const QString CpuInfo::TAG_IDLEDELTA_S = QStringLiteral("c:2");

const QString CpuInfo::TAG_MEMORYLOAD_L = QStringLiteral("memoryLoad");
const QString CpuInfo::TAG_MEMORYLOAD_S = QStringLiteral("c:3");
const QString CpuInfo::TAG_MEMORYTOTAL_L = QStringLiteral("memoryTotal");
const QString CpuInfo::TAG_MEMORYTOTAL_S = QStringLiteral("c:4");
const QString CpuInfo::TAG_MEMORYAVAIL_L = QStringLiteral("memoryAvail");
const QString CpuInfo::TAG_MEMORYAVAIL_S = QStringLiteral("c:5");
const QString CpuInfo::TAG_PAGETOTAL_L = QStringLiteral("pageTotal");
const QString CpuInfo::TAG_PAGETOTAL_S = QStringLiteral("c:6");
const QString CpuInfo::TAG_PAGEAVAIL_L = QStringLiteral("pageAvail");
const QString CpuInfo::TAG_PAGEAVAIL_S = QStringLiteral("c:7");
const QString CpuInfo::TAG_VIRTUALTOTAL_L = QStringLiteral("virtualTotal");
const QString CpuInfo::TAG_VIRTUALTOTAL_S = QStringLiteral("c:8");
const QString CpuInfo::TAG_VIRTUALAVAIL_L = QStringLiteral("virtualAvail");
const QString CpuInfo::TAG_VIRTUALAVAIL_S = QStringLiteral("c:9");

QJsonObject CpuInfo::toJsonObject(bool verbose) const {
	QJsonObject result;

	if (verbose) {
		result.insert(TAG_USERDELTA_L, QJsonValue(static_cast<qint64>(userTimeDelta)));
		result.insert(TAG_KERNELDELTA_L, QJsonValue(static_cast<qint64>(kernelTimeDelta)));
		result.insert(TAG_IDLEDELTA_L, QJsonValue(static_cast<qint64>(idleTimeDelta)));

		result.insert(TAG_MEMORYLOAD_L, QJsonValue(static_cast<qint64>(memoryLoad)));
		result.insert(TAG_MEMORYTOTAL_L, QJsonValue(static_cast<qint64>(memoryTotal)));
		result.insert(TAG_MEMORYAVAIL_L, QJsonValue(static_cast<qint64>(memoryAvail)));
		result.insert(TAG_PAGETOTAL_L, QJsonValue(static_cast<qint64>(pageTotal)));
		result.insert(TAG_PAGEAVAIL_L, QJsonValue(static_cast<qint64>(pageAvail)));
		result.insert(TAG_VIRTUALTOTAL_L, QJsonValue(static_cast<qint64>(virtualTotal)));
		result.insert(TAG_VIRTUALAVAIL_L, QJsonValue(static_cast<qint64>(virtualAvail)));
	} else {
		result.insert(TAG_USERDELTA_S, QJsonValue(static_cast<qint64>(userTimeDelta)));
		result.insert(TAG_KERNELDELTA_S, QJsonValue(static_cast<qint64>(kernelTimeDelta)));
		result.insert(TAG_IDLEDELTA_S, QJsonValue(static_cast<qint64>(idleTimeDelta)));

		result.insert(TAG_MEMORYLOAD_S, QJsonValue(static_cast<qint64>(memoryLoad)));
		result.insert(TAG_MEMORYTOTAL_S, QJsonValue(static_cast<qint64>(memoryTotal)));
		result.insert(TAG_MEMORYAVAIL_S, QJsonValue(static_cast<qint64>(memoryAvail)));
		result.insert(TAG_PAGETOTAL_S, QJsonValue(static_cast<qint64>(pageTotal)));
		result.insert(TAG_PAGEAVAIL_S, QJsonValue(static_cast<qint64>(pageAvail)));
		result.insert(TAG_VIRTUALTOTAL_S, QJsonValue(static_cast<qint64>(virtualTotal)));
		result.insert(TAG_VIRTUALAVAIL_S, QJsonValue(static_cast<qint64>(virtualAvail)));
	}

	return result;
}

CpuInfo CpuInfo::fromJsonObject(QJsonObject const& object, bool* okay) {
	CpuInfo result;
	bool ok = true;
	bool subOk = false;

	ASSIGN_SHOULD_CONTAIN(result.userTimeDelta, TAG_USERDELTA_L, TAG_USERDELTA_S);
	ASSIGN_SHOULD_CONTAIN(result.kernelTimeDelta, TAG_KERNELDELTA_L, TAG_KERNELDELTA_S);
	ASSIGN_SHOULD_CONTAIN(result.idleTimeDelta, TAG_IDLEDELTA_L, TAG_IDLEDELTA_S);
	
	ASSIGN_SHOULD_CONTAIN(result.memoryLoad, TAG_MEMORYLOAD_L, TAG_MEMORYLOAD_S);
	ASSIGN_SHOULD_CONTAIN(result.memoryTotal, TAG_MEMORYTOTAL_L, TAG_MEMORYTOTAL_S);
	ASSIGN_SHOULD_CONTAIN(result.memoryAvail, TAG_MEMORYAVAIL_L, TAG_MEMORYAVAIL_S);
	ASSIGN_SHOULD_CONTAIN(result.pageTotal, TAG_PAGETOTAL_L, TAG_PAGETOTAL_S);
	ASSIGN_SHOULD_CONTAIN(result.pageAvail, TAG_PAGEAVAIL_L, TAG_PAGEAVAIL_S);
	ASSIGN_SHOULD_CONTAIN(result.virtualTotal, TAG_VIRTUALTOTAL_L, TAG_VIRTUALTOTAL_S);
	ASSIGN_SHOULD_CONTAIN(result.virtualAvail, TAG_VIRTUALAVAIL_L, TAG_VIRTUALAVAIL_S);
	if (okay != nullptr) {
		*okay = ok;
	}

	return result;
}


std::ostream& operator<<(std::ostream& os, const CpuInfo& ci) {
	os << "User Time Delta = " << ci.userTimeDelta;
	os << ", Kernel Time Delta = " << ci.kernelTimeDelta;
	os << ", Idle Time Delta = " << ci.idleTimeDelta;
	os << ", Memory Load = " << ci.memoryLoad;
	os << ", Total Memory = " << ci.memoryTotal;
	os << ", Memory Available = " << ci.memoryAvail;
	os << ", Paging Memory Total = " << ci.pageTotal;
	os << ", Paging Memory Available = " << ci.pageAvail;
	os << ", Total Virtual Memory = " << ci.virtualTotal;
	os << ", Virtual Memory Available = " << ci.virtualAvail;
	return os;
}
