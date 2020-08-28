#include "ph_callback.h"

#include "cb_object.h"

#include "procprv.h"

extern void EtGpuMonitorInitialization();

typedef struct _ET_PROCESS_GPU_STATISTICS {
    ULONG SegmentCount;
    ULONG NodeCount;

    ULONG64 DedicatedCommitted;
    ULONG64 SharedCommitted;

    ULONG64 BytesAllocated;
    ULONG64 BytesReserved;
    ULONG64 WriteCombinedBytesAllocated;
    ULONG64 WriteCombinedBytesReserved;
    ULONG64 CachedBytesAllocated;
    ULONG64 CachedBytesReserved;
    ULONG64 SectionBytesAllocated;
    ULONG64 SectionBytesReserved;

    ULONG64 RunningTime;
    ULONG64 ContextSwitches;
} ET_PROCESS_GPU_STATISTICS, * PET_PROCESS_GPU_STATISTICS;

extern VOID EtQueryProcessGpuStatistics(
    _In_ HANDLE ProcessHandle,
    _Out_ PET_PROCESS_GPU_STATISTICS Statistics
);

void phToZeusCallback(PPH_PROCESS_ITEM processItem, PPH_PROCESS_NODE processNode, ULONG id, PPH_STRINGREF stringRef) {
    static int isInit = 0;

    if (isInit == 0) {
        EtGpuMonitorInitialization();
        isInit = 1;
    }

    HANDLE proccessHandle = processItem->ProcessId;

    ET_PROCESS_GPU_STATISTICS stats;

    EtQueryProcessGpuStatistics(proccessHandle, &stats);

	callbackTest(id);
}
