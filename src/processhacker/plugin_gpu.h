#ifndef PLUGIN_GPU_H
#define PLUGIN_GPU_H

#ifdef __cplusplus
extern "C" {
#endif

//#include <phapp.h>
#include <ph.h>
//#include <phdk.h>
#include "dltmgr.h"
#include "circbuf.h"

    VOID pluginGpuInit(HINSTANCE Instance);
    VOID pluginGpuUnload();
    VOID updateProcesses();

    extern BOOLEAN EtGpuEnabled;
    extern BOOLEAN EtD3DEnabled;
    extern PPH_LIST EtpGpuAdapterList;

    extern ULONG EtGpuTotalNodeCount;
    extern ULONG EtGpuTotalSegmentCount;
    extern ULONG64 EtGpuDedicatedLimit;
    extern ULONG64 EtGpuSharedLimit;

    extern PH_UINT64_DELTA EtClockTotalRunningTimeDelta;
    extern LARGE_INTEGER EtClockTotalRunningTimeFrequency;
    extern FLOAT EtGpuNodeUsage;
    extern PH_CIRCULAR_BUFFER_FLOAT EtGpuNodeHistory;
    extern PH_CIRCULAR_BUFFER_ULONG EtMaxGpuNodeHistory; // ID of max. GPU usage process
    extern PH_CIRCULAR_BUFFER_FLOAT EtMaxGpuNodeUsageHistory;

    extern PPH_UINT64_DELTA EtGpuNodesTotalRunningTimeDelta;
    extern PPH_CIRCULAR_BUFFER_FLOAT EtGpuNodesHistory;

    extern ULONG64 EtGpuDedicatedUsage;
    extern ULONG64 EtGpuSharedUsage;
    extern PH_CIRCULAR_BUFFER_ULONG64 EtGpuDedicatedHistory;
    extern PH_CIRCULAR_BUFFER_ULONG64 EtGpuSharedHistory;

#ifdef __cplusplus
}
#endif


#endif
