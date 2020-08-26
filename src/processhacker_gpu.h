#ifndef PROCESSHACKER_GPU_H
#define PROCESSHACKER_GPU_H

// gpumon

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

VOID EtGpuMonitorInitialization(
    VOID
);

NTSTATUS EtQueryAdapterInformation(
    _In_ D3DKMT_HANDLE AdapterHandle,
    _In_ KMTQUERYADAPTERINFOTYPE InformationClass,
    _Out_writes_bytes_opt_(InformationLength) PVOID Information,
    _In_ UINT32 InformationLength
);

BOOLEAN EtCloseAdapterHandle(
    _In_ D3DKMT_HANDLE AdapterHandle
);

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

VOID EtSaveGpuMonitorSettings(
    VOID
);

ULONG EtGetGpuAdapterCount(
    VOID
);

ULONG EtGetGpuAdapterIndexFromNodeIndex(
    _In_ ULONG NodeIndex
);

PPH_STRING EtGetGpuAdapterNodeDescription(
    _In_ ULONG Index,
    _In_ ULONG NodeIndex
);

PPH_STRING EtGetGpuAdapterDescription(
    _In_ ULONG Index
);

VOID EtQueryProcessGpuStatistics(
    _In_ HANDLE ProcessHandle,
    _Out_ PET_PROCESS_GPU_STATISTICS Statistics
);

#endif // !PROCESSHACKER_GPU_H
