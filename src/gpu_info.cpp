#include "gpu_info.h"

std::ostream& operator<<(std::ostream& os, const GpuInfo& gi) {
	os << "Utilization = " << gi.utilization << ", dedicated memory = " << gi.dedicatedMemory << ", shared memory = " << gi.sharedMemory << ", commit memory = " << gi.commitMemory;
	return os;
}
