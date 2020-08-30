#ifndef GPU_QUERY_H
#define GPU_QUERY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

	BOOL gpuCountersInit();
	void gpuCountersPrepareBefore();
	void gpuCountersClose();

	NTSTATUS queryGpuCounters();

#ifdef __cplusplus
}
#endif

#endif
