#include "gpu_info.h"

QJsonObject GpuInfo::toJsonObject(bool verbose) const {
	QJsonObject result;
	if (verbose) {
		result.insert(QStringLiteral("gpuUtil"), QJsonValue(utilization));
		result.insert(QStringLiteral("gpuDedi"), QJsonValue(static_cast<qint64>(dedicatedMemory)));
		result.insert(QStringLiteral("gpuShar"), QJsonValue(static_cast<qint64>(sharedMemory)));
		result.insert(QStringLiteral("gpuCom"), QJsonValue(static_cast<qint64>(commitMemory)));
	} else {
		result.insert(QStringLiteral("g:0"), QJsonValue(utilization));
		result.insert(QStringLiteral("g:1"), QJsonValue(static_cast<qint64>(dedicatedMemory)));
		result.insert(QStringLiteral("g:2"), QJsonValue(static_cast<qint64>(sharedMemory)));
		result.insert(QStringLiteral("g:3"), QJsonValue(static_cast<qint64>(commitMemory)));
	}
	return result;
}

GpuInfo GpuInfo::fromJsonObject(QJsonObject const& object, bool* okay) {
	GpuInfo result;
	bool ok = true;
	bool subOk = false;

	result.utilization = (object.contains(QStringLiteral("gpuUtil")) ? object.value(QStringLiteral("gpuUtil")) : object.value(QStringLiteral("g:0"))).toString().toLongLong(&subOk); ok &= subOk;
	result.dedicatedMemory = (object.contains(QStringLiteral("gpuDedi")) ? object.value(QStringLiteral("gpuDedi")) : object.value(QStringLiteral("g:1"))).toString().toLongLong(&subOk); ok &= subOk;
	result.sharedMemory = (object.contains(QStringLiteral("gpuShar")) ? object.value(QStringLiteral("gpuShar")) : object.value(QStringLiteral("g:2"))).toString().toLongLong(&subOk); ok &= subOk;
	result.commitMemory = (object.contains(QStringLiteral("gpuCom")) ? object.value(QStringLiteral("gpuCom")) : object.value(QStringLiteral("g:3"))).toString().toLongLong(&subOk); ok &= subOk;

	if (okay != nullptr) {
		*okay = ok;
	}

	return result;
}

std::ostream& operator<<(std::ostream& os, const GpuInfo& gi) {
	os << "Utilization = " << gi.utilization << ", dedicated memory = " << gi.dedicatedMemory << ", shared memory = " << gi.sharedMemory << ", commit memory = " << gi.commitMemory;
	return os;
}
