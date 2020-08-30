#include "gpu_info.h"

QString GpuInfo::toQString() {
	return QStringLiteral("%1;%2;%3;%4").arg(utilization).arg(dedicatedMemory).arg(sharedMemory).arg(commitMemory);
}

std::ostream& operator<<(std::ostream& os, const GpuInfo& gi) {
	os << "Utilization = " << gi.utilization << ", dedicated memory = " << gi.dedicatedMemory << ", shared memory = " << gi.sharedMemory << ", commit memory = " << gi.commitMemory;
	return os;
}
