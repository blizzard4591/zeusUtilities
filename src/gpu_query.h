#ifndef GPU_QUERY_H
#define GPU_QUERY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

	LONG gpuQueriesInit();
	VOID gpuQueriesCleanup();

	LONG runGpuQueries();

#ifdef __cplusplus
}
#endif

#endif
