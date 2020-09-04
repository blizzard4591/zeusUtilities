#include "cpu_info.h"

QJsonObject CpuInfo::toJsonObject(bool verbose) const {
	QJsonObject result;

	if (verbose) {
		result.insert(QStringLiteral("userDelta"), QJsonValue(static_cast<qint64>(userTimeDelta)));
		result.insert(QStringLiteral("kernelDelta"), QJsonValue(static_cast<qint64>(kernelTimeDelta)));
		result.insert(QStringLiteral("idleDelta"), QJsonValue(static_cast<qint64>(idleTimeDelta)));

		result.insert(QStringLiteral("memoryLoad"), QJsonValue(static_cast<qint64>(memoryLoad)));
		result.insert(QStringLiteral("memoryTotal"), QJsonValue(static_cast<qint64>(memoryTotal)));
		result.insert(QStringLiteral("memoryAvail"), QJsonValue(static_cast<qint64>(memoryAvail)));
		result.insert(QStringLiteral("pageTotal"), QJsonValue(static_cast<qint64>(pageTotal)));
		result.insert(QStringLiteral("pageAvail"), QJsonValue(static_cast<qint64>(pageAvail)));
		result.insert(QStringLiteral("virtualTotal"), QJsonValue(static_cast<qint64>(virtualTotal)));
		result.insert(QStringLiteral("virtualAvail"), QJsonValue(static_cast<qint64>(virtualAvail)));
	} else {
		result.insert(QStringLiteral("c:0"), QJsonValue(static_cast<qint64>(userTimeDelta)));
		result.insert(QStringLiteral("c:1"), QJsonValue(static_cast<qint64>(kernelTimeDelta)));
		result.insert(QStringLiteral("c:2"), QJsonValue(static_cast<qint64>(idleTimeDelta)));

		result.insert(QStringLiteral("c:3"), QJsonValue(static_cast<qint64>(memoryLoad)));
		result.insert(QStringLiteral("c:4"), QJsonValue(static_cast<qint64>(memoryTotal)));
		result.insert(QStringLiteral("c:5"), QJsonValue(static_cast<qint64>(memoryAvail)));
		result.insert(QStringLiteral("c:6"), QJsonValue(static_cast<qint64>(pageTotal)));
		result.insert(QStringLiteral("c:7"), QJsonValue(static_cast<qint64>(pageAvail)));
		result.insert(QStringLiteral("c:8"), QJsonValue(static_cast<qint64>(virtualTotal)));
		result.insert(QStringLiteral("c:9"), QJsonValue(static_cast<qint64>(virtualAvail)));
	}

	return result;
}

CpuInfo CpuInfo::fromJsonObject(QJsonObject const& object, bool* okay) {
	CpuInfo result;
	bool ok = true;
	bool subOk = false;

	result.userTimeDelta = (object.contains(QStringLiteral("userDelta")) ? object.value(QStringLiteral("userDelta")) : object.value(QStringLiteral("c:0"))).toString().toLongLong(&subOk); ok &= subOk;
	result.kernelTimeDelta = (object.contains(QStringLiteral("kernelDelta")) ? object.value(QStringLiteral("kernelDelta")) : object.value(QStringLiteral("c:1"))).toString().toLongLong(&subOk); ok &= subOk;
	result.idleTimeDelta = (object.contains(QStringLiteral("idleDelta")) ? object.value(QStringLiteral("idleDelta")) : object.value(QStringLiteral("c:2"))).toString().toLongLong(&subOk); ok &= subOk;
	
	result.memoryLoad = (object.contains(QStringLiteral("memoryLoad")) ? object.value(QStringLiteral("memoryLoad")) : object.value(QStringLiteral("c:3"))).toString().toLongLong(&subOk); ok &= subOk;
	result.memoryTotal = (object.contains(QStringLiteral("memoryTotal")) ? object.value(QStringLiteral("memoryTotal")) : object.value(QStringLiteral("c:4"))).toString().toLongLong(&subOk); ok &= subOk;
	result.memoryAvail = (object.contains(QStringLiteral("memoryAvail")) ? object.value(QStringLiteral("memoryAvail")) : object.value(QStringLiteral("c:5"))).toString().toLongLong(&subOk); ok &= subOk;
	result.pageTotal = (object.contains(QStringLiteral("pageTotal")) ? object.value(QStringLiteral("pageTotal")) : object.value(QStringLiteral("c:6"))).toString().toLongLong(&subOk); ok &= subOk;
	result.pageAvail = (object.contains(QStringLiteral("pageAvail")) ? object.value(QStringLiteral("pageAvail")) : object.value(QStringLiteral("c:7"))).toString().toLongLong(&subOk); ok &= subOk;
	result.virtualTotal = (object.contains(QStringLiteral("virtualTotal")) ? object.value(QStringLiteral("virtualTotal")) : object.value(QStringLiteral("c:8"))).toString().toLongLong(&subOk); ok &= subOk;
	result.virtualAvail = (object.contains(QStringLiteral("virtualAvail")) ? object.value(QStringLiteral("virtualAvail")) : object.value(QStringLiteral("c:9"))).toString().toLongLong(&subOk); ok &= subOk;

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
