/*

#include "plugin_gpu.h"


#include "exttools.h"

#include <settings.h>

#define PH_RECORD_MAX_USAGE

extern PPH_OBJECT_TYPE PhProcessItemType;
extern PPH_LIST PhProcessRecordList;
extern PH_QUEUED_LOCK PhProcessRecordListLock;

extern ULONG PhStatisticsSampleCount;
extern BOOLEAN PhEnablePurgeProcessRecords;
extern BOOLEAN PhEnableCycleCpuUsage;

extern PVOID PhProcessInformation; // only can be used if running on same thread as process provider
extern ULONG PhProcessInformationSequenceNumber;
extern SYSTEM_PERFORMANCE_INFORMATION PhPerfInformation;
extern PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION PhCpuInformation;
extern SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION PhCpuTotals;
extern ULONG PhTotalProcesses;
extern ULONG PhTotalThreads;
extern ULONG PhTotalHandles;

extern ULONG64 PhCpuTotalCycleDelta;
extern PLARGE_INTEGER PhCpuIdleCycleTime; // cycle time for Idle
extern PLARGE_INTEGER PhCpuSystemCycleTime; // cycle time for DPCs and Interrupts
extern PH_UINT64_DELTA PhCpuIdleCycleDelta;
extern PH_UINT64_DELTA PhCpuSystemCycleDelta;

extern FLOAT PhCpuKernelUsage;
extern FLOAT PhCpuUserUsage;
extern PFLOAT PhCpusKernelUsage;
extern PFLOAT PhCpusUserUsage;

extern PH_UINT64_DELTA PhCpuKernelDelta;
extern PH_UINT64_DELTA PhCpuUserDelta;
extern PH_UINT64_DELTA PhCpuIdleDelta;

extern PPH_UINT64_DELTA PhCpusKernelDelta;
extern PPH_UINT64_DELTA PhCpusUserDelta;
extern PPH_UINT64_DELTA PhCpusIdleDelta;

extern PH_UINT64_DELTA PhIoReadDelta;
extern PH_UINT64_DELTA PhIoWriteDelta;
extern PH_UINT64_DELTA PhIoOtherDelta;

extern PH_CIRCULAR_BUFFER_FLOAT PhCpuKernelHistory;
extern PH_CIRCULAR_BUFFER_FLOAT PhCpuUserHistory;
//extern PH_CIRCULAR_BUFFER_FLOAT PhCpuOtherHistory;

extern PPH_CIRCULAR_BUFFER_FLOAT PhCpusKernelHistory;
extern PPH_CIRCULAR_BUFFER_FLOAT PhCpusUserHistory;
//extern PPH_CIRCULAR_BUFFER_FLOAT PhCpusOtherHistory;

extern PH_CIRCULAR_BUFFER_ULONG64 PhIoReadHistory;
extern PH_CIRCULAR_BUFFER_ULONG64 PhIoWriteHistory;
extern PH_CIRCULAR_BUFFER_ULONG64 PhIoOtherHistory;

extern PH_CIRCULAR_BUFFER_ULONG PhCommitHistory;
extern PH_CIRCULAR_BUFFER_ULONG PhPhysicalHistory;

extern PH_CIRCULAR_BUFFER_ULONG PhMaxCpuHistory;
extern PH_CIRCULAR_BUFFER_ULONG PhMaxIoHistory;
#ifdef PH_RECORD_MAX_USAGE
extern PH_CIRCULAR_BUFFER_FLOAT PhMaxCpuUsageHistory;
extern PH_CIRCULAR_BUFFER_ULONG64 PhMaxIoReadOtherHistory;
extern PH_CIRCULAR_BUFFER_ULONG64 PhMaxIoWriteHistory;
#endif

#define PROCESS_ID_BUCKETS 64
#define PROCESS_ID_TO_BUCKET_INDEX(ProcessId) ((HandleToUlong(ProcessId) / 4) & (PROCESS_ID_BUCKETS - 1))

static HANDLE ModuleProcessId = NULL;

static BOOLEAN PhProcessStatisticsInitialized = FALSE;
static ULONG PhTimeSequenceNumber = 0;
static PH_CIRCULAR_BUFFER_ULONG PhTimeHistory;

extern PSYSTEM_PROCESS_INFORMATION PhDpcsProcessInformation;
extern PSYSTEM_PROCESS_INFORMATION PhInterruptsProcessInformation;

extern ULONG64 PhCpuTotalCycleDelta; // real cycle time delta for this period
extern PLARGE_INTEGER PhCpuIdleCycleTime; // cycle time for Idle
extern PLARGE_INTEGER PhCpuSystemCycleTime; // cycle time for DPCs and Interrupts
extern PH_UINT64_DELTA PhCpuIdleCycleDelta;
extern PH_UINT64_DELTA PhCpuSystemCycleDelta;
//PPH_UINT64_DELTA PhCpusIdleCycleDelta;

extern FLOAT PhCpuKernelUsage;
extern FLOAT PhCpuUserUsage;
extern PFLOAT PhCpusKernelUsage;
extern PFLOAT PhCpusUserUsage;

extern PH_UINT64_DELTA PhCpuKernelDelta;
extern PH_UINT64_DELTA PhCpuUserDelta;
extern PH_UINT64_DELTA PhCpuIdleDelta;

extern PPH_UINT64_DELTA PhCpusKernelDelta;
extern PPH_UINT64_DELTA PhCpusUserDelta;
extern PPH_UINT64_DELTA PhCpusIdleDelta;

extern PPH_OBJECT_TYPE PhProcessItemType;

extern PPH_HASH_ENTRY PhProcessHashSet[256];
extern ULONG PhProcessHashSetCount;
extern PH_QUEUED_LOCK PhProcessHashSetLock;

typedef struct _PH_PROCESS_QUERY_DATA {
    SLIST_ENTRY ListEntry;
    ULONG Stage;
    PPH_PROCESS_ITEM ProcessItem;
} PH_PROCESS_QUERY_DATA, * PPH_PROCESS_QUERY_DATA;

typedef struct _PH_PROCESS_QUERY_S1_DATA {
    PH_PROCESS_QUERY_DATA Header;

    PPH_STRING CommandLine;

    HICON SmallIcon;
    HICON LargeIcon;
    PH_IMAGE_VERSION_INFO VersionInfo;

    HANDLE ConsoleHostProcessId;
    PPH_STRING PackageFullName;
    PPH_STRING UserName;

    union {
        ULONG Flags;
        struct {
            ULONG IsDotNet : 1;
            ULONG Spare1 : 1;
            ULONG IsInJob : 1;
            ULONG IsInSignificantJob : 1;
            ULONG IsBeingDebugged : 1;
            ULONG IsImmersive : 1;
            ULONG IsFilteredHandle : 1;
            ULONG Spare : 25;
        };
    };
} PH_PROCESS_QUERY_S1_DATA, * PPH_PROCESS_QUERY_S1_DATA;

typedef struct _PH_PROCESS_QUERY_S2_DATA {
    PH_PROCESS_QUERY_DATA Header;

    VERIFY_RESULT VerifyResult;
    PPH_STRING VerifySignerName;

    BOOLEAN IsPacked;
    ULONG ImportFunctions;
    ULONG ImportModules;
    PH_IMAGE_VERSION_INFO VersionInfo; // LXSS only
} PH_PROCESS_QUERY_S2_DATA, * PPH_PROCESS_QUERY_S2_DATA;

typedef struct _PH_SID_FULL_NAME_CACHE_ENTRY {
    PSID Sid;
    PPH_STRING FullName;
} PH_SID_FULL_NAME_CACHE_ENTRY, * PPH_SID_FULL_NAME_CACHE_ENTRY;

// Till here: process stuff

// Here: Plugin init

extern PH_CALLBACK_REGISTRATION PluginLoadCallbackRegistration;
extern PH_CALLBACK_REGISTRATION PluginUnloadCallbackRegistration;
extern PH_CALLBACK_REGISTRATION PluginShowOptionsCallbackRegistration;
extern PH_CALLBACK_REGISTRATION PluginMenuItemCallbackRegistration;
extern PH_CALLBACK_REGISTRATION PluginTreeNewMessageCallbackRegistration;
extern PH_CALLBACK_REGISTRATION PluginPhSvcRequestCallbackRegistration;
extern PH_CALLBACK_REGISTRATION MainWindowShowingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ProcessesUpdatedCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ProcessPropertiesInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION HandlePropertiesInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ProcessMenuInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ThreadMenuInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ModuleMenuInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ProcessTreeNewInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION NetworkTreeNewInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION SystemInformationInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION MiniInformationInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION TrayIconsInitializingCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ProcessItemsUpdatedCallbackRegistration;
extern PH_CALLBACK_REGISTRATION NetworkItemsUpdatedCallbackRegistration;
extern PH_CALLBACK_REGISTRATION ProcessStatsEventCallbackRegistration;

extern VOID NTAPI LoadCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI UnloadCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ShowOptionsCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI MenuItemCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI TreeNewMessageCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI PhSvcRequestCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI MainWindowShowingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ProcessesUpdatedCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ProcessPropertiesInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI HandlePropertiesInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ProcessMenuInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ThreadMenuInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ModuleMenuInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ProcessTreeNewInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI NetworkTreeNewInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI SystemInformationInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI MiniInformationInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI TrayIconsInitializingCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ProcessItemsUpdatedCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI NetworkItemsUpdatedCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ProcessStatsEventCallback(
    _In_opt_ PVOID Parameter,
    _In_opt_ PVOID Context
);

extern VOID NTAPI ProcessItemCreateCallback(
    _In_ PVOID Object,
    _In_ PH_EM_OBJECT_TYPE ObjectType,
    _In_ PVOID Extension
);

extern VOID NTAPI ProcessItemDeleteCallback(
    _In_ PVOID Object,
    _In_ PH_EM_OBJECT_TYPE ObjectType,
    _In_ PVOID Extension
);

extern VOID NTAPI NetworkItemCreateCallback(
    _In_ PVOID Object,
    _In_ PH_EM_OBJECT_TYPE ObjectType,
    _In_ PVOID Extension
);

extern VOID NTAPI NetworkItemDeleteCallback(
    _In_ PVOID Object,
    _In_ PH_EM_OBJECT_TYPE ObjectType,
    _In_ PVOID Extension
);

VOID updateProcesses() {
    static ULONG runCount = 0;
    static PSYSTEM_PROCESS_INFORMATION pidBuckets[PROCESS_ID_BUCKETS];

    // Note about locking:
    //
    // Since this is the only function that is allowed to modify the process hashtable, locking is
    // not needed for shared accesses. However, exclusive accesses need locking.

    PVOID processes;
    PSYSTEM_PROCESS_INFORMATION process;
    ULONG bucketIndex;

    ULONG64 sysTotalTime; // total time for this update period
    ULONG64 sysTotalCycleTime = 0; // total cycle time for this update period
    ULONG64 sysIdleCycleTime = 0; // total idle cycle time for this update period
    FLOAT maxCpuValue = 0;
    PPH_PROCESS_ITEM maxCpuProcessItem = NULL;
    ULONG64 maxIoValue = 0;
    PPH_PROCESS_ITEM maxIoProcessItem = NULL;

    // Pre-update tasks

    if (runCount % 512 == 0) // yes, a very long time
    {
        if (PhEnablePurgeProcessRecords)
            PhPurgeProcessRecords();

        PhpFlushSidFullNameCache();

        PhFlushImageVersionInfoCache();
    }

    if (!PhProcessStatisticsInitialized) {
        PhpInitializeProcessStatistics();
        PhProcessStatisticsInitialized = TRUE;
    }

    PhpUpdatePerfInformation();

    if (PhEnableCycleCpuUsage) {
        PhpUpdateCpuInformation(FALSE, &sysTotalTime);
        PhpUpdateCpuCycleInformation(&sysIdleCycleTime);
    } else {
        PhpUpdateCpuInformation(TRUE, &sysTotalTime);
    }

    if (runCount != 0) {
        PhTimeSequenceNumber++;
    }

    // Get the process list.

    PhTotalProcesses = 0;
    PhTotalThreads = 0;
    PhTotalHandles = 0;

    if (!NT_SUCCESS(PhEnumProcesses(&processes)))
        return;

    // Notes on cycle-based CPU usage:
    //
    // Cycle-based CPU usage is a bit tricky to calculate because we cannot get the total number of
    // cycles consumed by all processes since system startup - we can only get total number of
    // cycles per process. This means there are two ways to calculate the system-wide cycle time
    // delta:
    //
    // 1. Each update, sum the cycle times of all processes, and calculate the system-wide delta
    //    from this. Process Explorer seems to do this.
    // 2. Each update, calculate the cycle time delta for each individual process, and sum these
    //    deltas to create the system-wide delta. We use this here.
    //
    // The first method is simpler but has a problem when a process exits and its cycle time is no
    // longer counted in the system-wide total. This may cause the delta to be negative and all
    // other calculations to become invalid. Process Explorer simply ignored this fact and treated
    // the system-wide delta as unsigned (and therefore huge when negative), leading to all CPU
    // usages being displayed as "< 0.01".
    //
    // The second method is used here, but the adjustments must be done before the main new/modified
    // pass. We need take into account new, existing and terminated processes.

    // Create the PID hash set. This contains the process information structures returned by
    // PhEnumProcesses, distinct from the process item hash set. Note that we use the
    // UniqueProcessKey field as the next node pointer to avoid having to allocate extra memory.

    memset(pidBuckets, 0, sizeof(pidBuckets));

    process = PH_FIRST_PROCESS(processes);

    do {
        PhTotalProcesses++;
        PhTotalThreads += process->NumberOfThreads;
        PhTotalHandles += process->HandleCount;

        if (process->UniqueProcessId == SYSTEM_IDLE_PROCESS_ID) {
            process->CycleTime = PhCpuIdleCycleDelta.Value;
            process->KernelTime = PhCpuTotals.IdleTime;
        }

        bucketIndex = PROCESS_ID_TO_BUCKET_INDEX(process->UniqueProcessId);
        process->UniqueProcessKey = (ULONG_PTR)pidBuckets[bucketIndex];
        pidBuckets[bucketIndex] = process;

        if (PhEnableCycleCpuUsage) {
            PPH_PROCESS_ITEM processItem;

            if (WindowsVersion >= WINDOWS_10_RS3 && !PhIsExecutingInWow64()) {
                if ((processItem = PhpLookupProcessItem(process->UniqueProcessId)) && processItem->ProcessSequenceNumber == PH_PROCESS_EXTENSION(process)->ProcessSequenceNumber)
                    sysTotalCycleTime += process->CycleTime - processItem->CycleTimeDelta.Value; // existing process
                else
                    sysTotalCycleTime += process->CycleTime; // new process
            } else {
                if ((processItem = PhpLookupProcessItem(process->UniqueProcessId)) && processItem->CreateTime.QuadPart == process->CreateTime.QuadPart)
                    sysTotalCycleTime += process->CycleTime - processItem->CycleTimeDelta.Value; // existing process
                else
                    sysTotalCycleTime += process->CycleTime; // new process
            }
        }
    } while (process = PH_NEXT_PROCESS(process));

    // Add the fake processes to the PID list.
    //
    // On Windows 7 the two fake processes are merged into "Interrupts" since we can only get cycle
    // time information both DPCs and Interrupts combined.

    if (PhEnableCycleCpuUsage) {
        PhInterruptsProcessInformation->KernelTime.QuadPart = PhCpuTotals.DpcTime.QuadPart + PhCpuTotals.InterruptTime.QuadPart;
        PhInterruptsProcessInformation->CycleTime = PhCpuSystemCycleDelta.Value;
        sysTotalCycleTime += PhCpuSystemCycleDelta.Delta;
    } else {
        PhDpcsProcessInformation->KernelTime = PhCpuTotals.DpcTime;
        PhInterruptsProcessInformation->KernelTime = PhCpuTotals.InterruptTime;
    }

    // Look for dead processes.
    {
        PPH_LIST processesToRemove = NULL;
        ULONG i;
        PPH_HASH_ENTRY entry;
        PPH_PROCESS_ITEM processItem;
        PSYSTEM_PROCESS_INFORMATION processEntry;

        for (i = 0; i < PH_HASH_SET_SIZE(PhProcessHashSet); i++) {
            for (entry = PhProcessHashSet[i]; entry; entry = entry->Next) {
                BOOLEAN processRemoved = FALSE;

                processItem = CONTAINING_RECORD(entry, PH_PROCESS_ITEM, HashEntry);

                // Check if the process still exists. Note that we take into account PID re-use by
                // checking CreateTime as well.

                if (processItem->ProcessId == DPCS_PROCESS_ID) {
                    processEntry = PhDpcsProcessInformation;
                } else if (processItem->ProcessId == INTERRUPTS_PROCESS_ID) {
                    processEntry = PhInterruptsProcessInformation;
                } else {
                    processEntry = pidBuckets[PROCESS_ID_TO_BUCKET_INDEX(processItem->ProcessId)];

                    while (processEntry && processEntry->UniqueProcessId != processItem->ProcessId)
                        processEntry = (PSYSTEM_PROCESS_INFORMATION)processEntry->UniqueProcessKey;
                }

                if (WindowsVersion >= WINDOWS_10_RS3 && !PhIsExecutingInWow64()) {
                    if (!processEntry || PH_PROCESS_EXTENSION(processEntry)->ProcessSequenceNumber != processItem->ProcessSequenceNumber)
                        processRemoved = TRUE;
                } else {
                    if (!processEntry || processEntry->CreateTime.QuadPart != processItem->CreateTime.QuadPart)
                        processRemoved = TRUE;
                }

                if (processRemoved) {
                    LARGE_INTEGER exitTime;

                    processItem->State |= PH_PROCESS_ITEM_REMOVED;
                    exitTime.QuadPart = 0;

                    if (processItem->QueryHandle) {
                        KERNEL_USER_TIMES times;
                        ULONG64 finalCycleTime;

                        if (NT_SUCCESS(PhGetProcessTimes(processItem->QueryHandle, &times))) {
                            exitTime = times.ExitTime;
                        }

                        if (PhEnableCycleCpuUsage) {
                            if (NT_SUCCESS(PhGetProcessCycleTime(processItem->QueryHandle, &finalCycleTime))) {
                                // Adjust deltas for the terminated process because this doesn't get
                                // picked up anywhere else.
                                //
                                // Note that if we don't have sufficient access to the process, the
                                // worst that will happen is that the CPU usages of other processes
                                // will get inflated. (See above; if we were using the first
                                // technique, we could get negative deltas, which is much worse.)
                                sysTotalCycleTime += finalCycleTime - processItem->CycleTimeDelta.Value;
                            }
                        }
                    }

                    // If we don't have a valid exit time, use the current time.
                    if (exitTime.QuadPart == 0)
                        PhQuerySystemTime(&exitTime);

                    processItem->Record->Flags |= PH_PROCESS_RECORD_DEAD;
                    processItem->Record->ExitTime = exitTime;

                    // Raise the process removed event.
                    PhInvokeCallback(PhGetGeneralCallback(GeneralCallbackProcessProviderRemovedEvent), processItem);

                    if (!processesToRemove)
                        processesToRemove = PhCreateList(2);

                    PhAddItemList(processesToRemove, processItem);
                }
            }
        }

        // Lock only if we have something to do.
        if (processesToRemove) {
            PhAcquireQueuedLockExclusive(&PhProcessHashSetLock);

            for (i = 0; i < processesToRemove->Count; i++) {
                PhpRemoveProcessItem((PPH_PROCESS_ITEM)processesToRemove->Items[i]);
            }

            PhReleaseQueuedLockExclusive(&PhProcessHashSetLock);
            PhDereferenceObject(processesToRemove);
        }
    }

    // Go through the queued process query data.
    PhFlushProcessQueryData();

    if (sysTotalTime == 0)
        sysTotalTime = -1; // max. value
    if (sysTotalCycleTime == 0)
        sysTotalCycleTime = -1;

    PhCpuTotalCycleDelta = sysTotalCycleTime;

    // Look for new processes and update existing ones.
    process = PH_FIRST_PROCESS(processes);

    while (process) {
        PPH_PROCESS_ITEM processItem;

        processItem = PhpLookupProcessItem(process->UniqueProcessId);

        if (!processItem) {
            PPH_PROCESS_RECORD processRecord;
            BOOLEAN isSuspended;
            BOOLEAN isPartiallySuspended;
            ULONG contextSwitches;

            // Create the process item and fill in basic information.
            processItem = PhCreateProcessItem(process->UniqueProcessId);
            PhpFillProcessItem(processItem, process);
            PhpFillProcessItemExtension(processItem, process);
            processItem->TimeSequenceNumber = PhTimeSequenceNumber;

            processRecord = PhpCreateProcessRecord(processItem);
            PhpAddProcessRecord(processRecord);
            processItem->Record = processRecord;

            PhpGetProcessThreadInformation(process, &isSuspended, &isPartiallySuspended, &contextSwitches);
            PhpUpdateDynamicInfoProcessItem(processItem, process);

            // Initialize the deltas.
            PhUpdateDelta(&processItem->CpuKernelDelta, process->KernelTime.QuadPart);
            PhUpdateDelta(&processItem->CpuUserDelta, process->UserTime.QuadPart);
            PhUpdateDelta(&processItem->IoReadDelta, process->ReadTransferCount.QuadPart);
            PhUpdateDelta(&processItem->IoWriteDelta, process->WriteTransferCount.QuadPart);
            PhUpdateDelta(&processItem->IoOtherDelta, process->OtherTransferCount.QuadPart);
            PhUpdateDelta(&processItem->IoReadCountDelta, process->ReadOperationCount.QuadPart);
            PhUpdateDelta(&processItem->IoWriteCountDelta, process->WriteOperationCount.QuadPart);
            PhUpdateDelta(&processItem->IoOtherCountDelta, process->OtherOperationCount.QuadPart);
            PhUpdateDelta(&processItem->ContextSwitchesDelta, contextSwitches);
            PhUpdateDelta(&processItem->PageFaultsDelta, process->PageFaultCount);
            PhUpdateDelta(&processItem->CycleTimeDelta, process->CycleTime);
            PhUpdateDelta(&processItem->PrivateBytesDelta, process->PagefileUsage);

            processItem->IsSuspended = isSuspended;
            processItem->IsPartiallySuspended = isPartiallySuspended;

            // If this is the first run of the provider, queue the
            // process query tasks. Otherwise, perform stage 1
            // processing now and queue stage 2 processing.
            if (runCount > 0) {
                PH_PROCESS_QUERY_S1_DATA data;

                memset(&data, 0, sizeof(PH_PROCESS_QUERY_S1_DATA));
                data.Header.Stage = 1;
                data.Header.ProcessItem = processItem;
                PhpProcessQueryStage1(&data);
                PhpFillProcessItemStage1(&data);
                PhSetEvent(&processItem->Stage1Event);
            } else {
                PhpQueueProcessQueryStage1(processItem);
            }

            // Add pending service items to the process item.
            PhUpdateProcessItemServices(processItem);

            // Add the process item to the hashtable.
            PhAcquireQueuedLockExclusive(&PhProcessHashSetLock);
            PhpAddProcessItem(processItem);
            PhReleaseQueuedLockExclusive(&PhProcessHashSetLock);

            // Raise the process added event.
            PhInvokeCallback(PhGetGeneralCallback(GeneralCallbackProcessProviderAddedEvent), processItem);

            // (Ref: for the process item being in the hashtable.)
            // Instead of referencing then dereferencing we simply don't do anything.
            // Dereferenced in PhpRemoveProcessItem.
        } else {
            BOOLEAN modified = FALSE;
            BOOLEAN isSuspended;
            BOOLEAN isPartiallySuspended;
            ULONG contextSwitches;
            FLOAT newCpuUsage;
            FLOAT kernelCpuUsage;
            FLOAT userCpuUsage;

            PhpGetProcessThreadInformation(process, &isSuspended, &isPartiallySuspended, &contextSwitches);
            PhpUpdateDynamicInfoProcessItem(processItem, process);
            PhpFillProcessItemExtension(processItem, process);

            // Update the deltas.
            PhUpdateDelta(&processItem->CpuKernelDelta, process->KernelTime.QuadPart);
            PhUpdateDelta(&processItem->CpuUserDelta, process->UserTime.QuadPart);
            PhUpdateDelta(&processItem->IoReadDelta, process->ReadTransferCount.QuadPart);
            PhUpdateDelta(&processItem->IoWriteDelta, process->WriteTransferCount.QuadPart);
            PhUpdateDelta(&processItem->IoOtherDelta, process->OtherTransferCount.QuadPart);
            PhUpdateDelta(&processItem->IoReadCountDelta, process->ReadOperationCount.QuadPart);
            PhUpdateDelta(&processItem->IoWriteCountDelta, process->WriteOperationCount.QuadPart);
            PhUpdateDelta(&processItem->IoOtherCountDelta, process->OtherOperationCount.QuadPart);
            PhUpdateDelta(&processItem->ContextSwitchesDelta, contextSwitches);
            PhUpdateDelta(&processItem->PageFaultsDelta, process->PageFaultCount);
            PhUpdateDelta(&processItem->CycleTimeDelta, process->CycleTime);
            PhUpdateDelta(&processItem->PrivateBytesDelta, process->PagefileUsage);

            processItem->TimeSequenceNumber++;
            PhAddItemCircularBuffer_ULONG64(&processItem->IoReadHistory, processItem->IoReadDelta.Delta);
            PhAddItemCircularBuffer_ULONG64(&processItem->IoWriteHistory, processItem->IoWriteDelta.Delta);
            PhAddItemCircularBuffer_ULONG64(&processItem->IoOtherHistory, processItem->IoOtherDelta.Delta);

            PhAddItemCircularBuffer_SIZE_T(&processItem->PrivateBytesHistory, processItem->VmCounters.PagefileUsage);
            //PhAddItemCircularBuffer_SIZE_T(&processItem->WorkingSetHistory, processItem->VmCounters.WorkingSetSize);

            if (InterlockedExchange(&processItem->JustProcessed, 0) != 0)
                modified = TRUE;

            if (PhEnableCycleCpuUsage) {
                FLOAT totalDelta;

                newCpuUsage = (FLOAT)processItem->CycleTimeDelta.Delta / sysTotalCycleTime;

                // Calculate the kernel/user CPU usage based on the kernel/user time. If the kernel
                // and user deltas are both zero, we'll just have to use an estimate. Currently, we
                // split the CPU usage evenly across the kernel and user components, except when the
                // total user time is zero, in which case we assign it all to the kernel component.

                totalDelta = (FLOAT)(processItem->CpuKernelDelta.Delta + processItem->CpuUserDelta.Delta);

                if (totalDelta != 0) {
                    kernelCpuUsage = newCpuUsage * ((FLOAT)processItem->CpuKernelDelta.Delta / totalDelta);
                    userCpuUsage = newCpuUsage * ((FLOAT)processItem->CpuUserDelta.Delta / totalDelta);
                } else {
                    if (processItem->UserTime.QuadPart != 0) {
                        kernelCpuUsage = newCpuUsage / 2;
                        userCpuUsage = newCpuUsage / 2;
                    } else {
                        kernelCpuUsage = newCpuUsage;
                        userCpuUsage = 0;
                    }
                }
            } else {
                kernelCpuUsage = (FLOAT)processItem->CpuKernelDelta.Delta / sysTotalTime;
                userCpuUsage = (FLOAT)processItem->CpuUserDelta.Delta / sysTotalTime;
                newCpuUsage = kernelCpuUsage + userCpuUsage;
            }

            processItem->CpuUsage = newCpuUsage;
            processItem->CpuKernelUsage = kernelCpuUsage;
            processItem->CpuUserUsage = userCpuUsage;

            PhAddItemCircularBuffer_FLOAT(&processItem->CpuKernelHistory, kernelCpuUsage);
            PhAddItemCircularBuffer_FLOAT(&processItem->CpuUserHistory, userCpuUsage);

            // Max. values

            if (processItem->ProcessId) {
                if (maxCpuValue < newCpuUsage) {
                    maxCpuValue = newCpuUsage;
                    maxCpuProcessItem = processItem;
                }

                // I/O for Other is not included because it is too generic.
                if (maxIoValue < processItem->IoReadDelta.Delta + processItem->IoWriteDelta.Delta) {
                    maxIoValue = processItem->IoReadDelta.Delta + processItem->IoWriteDelta.Delta;
                    maxIoProcessItem = processItem;
                }
            }

            // Token information
            if (
                processItem->QueryHandle &&
                processItem->ProcessId != SYSTEM_PROCESS_ID // System token can't be opened (dmex)
                ) {
                HANDLE tokenHandle;

                if (NT_SUCCESS(PhOpenProcessToken(
                    processItem->QueryHandle,
                    TOKEN_QUERY,
                    &tokenHandle
                ))) {
                    PTOKEN_USER tokenUser;
                    TOKEN_ELEVATION_TYPE elevationType;
                    MANDATORY_LEVEL integrityLevel;
                    PWSTR integrityString;

                    // User
                    if (NT_SUCCESS(PhGetTokenUser(tokenHandle, &tokenUser))) {
                        if (!RtlEqualSid(processItem->Sid, tokenUser->User.Sid)) {
                            PSID processSid;

                            // HACK (dmex)
                            processSid = processItem->Sid;
                            processItem->Sid = PhAllocateCopy(tokenUser->User.Sid, RtlLengthSid(tokenUser->User.Sid));
                            PhFree(processSid);

                            PhMoveReference(&processItem->UserName, PhpGetSidFullNameCachedSlow(processItem->Sid));

                            modified = TRUE;
                        }

                        PhFree(tokenUser);
                    }

                    // Elevation
                    if (NT_SUCCESS(PhGetTokenElevationType(tokenHandle, &elevationType))) {
                        if (processItem->ElevationType != elevationType) {
                            processItem->ElevationType = elevationType;
                            processItem->IsElevated = elevationType == TokenElevationTypeFull;
                            modified = TRUE;
                        }
                    }

                    // Integrity
                    if (NT_SUCCESS(PhGetTokenIntegrityLevel(tokenHandle, &integrityLevel, &integrityString))) {
                        if (processItem->IntegrityLevel != integrityLevel) {
                            processItem->IntegrityLevel = integrityLevel;
                            processItem->IntegrityString = integrityString;
                            modified = TRUE;
                        }
                    }

                    NtClose(tokenHandle);
                }
            }

            // Job
            if (processItem->QueryHandle) {
                NTSTATUS status;
                BOOLEAN isInSignificantJob = FALSE;
                BOOLEAN isInJob = FALSE;

                if (KphIsConnected()) {
                    HANDLE jobHandle = NULL;

                    status = KphOpenProcessJob(
                        processItem->QueryHandle,
                        JOB_OBJECT_QUERY,
                        &jobHandle
                    );

                    if (NT_SUCCESS(status) && status != STATUS_PROCESS_NOT_IN_JOB) {
                        JOBOBJECT_BASIC_LIMIT_INFORMATION basicLimits;

                        isInJob = TRUE;

                        if (NT_SUCCESS(PhGetJobBasicLimits(jobHandle, &basicLimits))) {
                            isInSignificantJob = basicLimits.LimitFlags != JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
                        }
                    }

                    if (jobHandle)
                        NtClose(jobHandle);
                } else {
                    status = NtIsProcessInJob(processItem->QueryHandle, NULL);

                    if (NT_SUCCESS(status))
                        isInJob = status == STATUS_PROCESS_IN_JOB;
                }

                if (processItem->IsInSignificantJob != isInSignificantJob) {
                    processItem->IsInSignificantJob = isInSignificantJob;
                    modified = TRUE;
                }

                if (processItem->IsInJob != isInJob) {
                    processItem->IsInJob = isInJob;
                    modified = TRUE;
                }
            }

            // Debugged
            if (processItem->QueryHandle && !processItem->IsSubsystemProcess && !processItem->IsProtectedHandle) {
                BOOLEAN isBeingDebugged = FALSE;

                PhGetProcessIsBeingDebugged(processItem->QueryHandle, &isBeingDebugged);

                if (processItem->IsBeingDebugged != isBeingDebugged) {
                    processItem->IsBeingDebugged = isBeingDebugged;
                    modified = TRUE;
                }
            }

            // Suspended
            if (processItem->IsSuspended != isSuspended) {
                processItem->IsSuspended = isSuspended;
                modified = TRUE;
            }

            processItem->IsPartiallySuspended = isPartiallySuspended;

            // .NET
            if (processItem->UpdateIsDotNet) {
                BOOLEAN isDotNet;
                ULONG flags = 0;

                if (NT_SUCCESS(PhGetProcessIsDotNetEx(processItem->ProcessId, NULL, 0, &isDotNet, &flags))) {
                    processItem->IsDotNet = isDotNet;

                    // This check is needed for the DotNetTools plugin. (dmex)
                    if (!isDotNet && (flags & PH_CLR_JIT_PRESENT))
                        processItem->IsDotNet = TRUE;

                    modified = TRUE;
                }

                processItem->UpdateIsDotNet = FALSE;
            }

            // Immersive
            if (processItem->QueryHandle && WindowsVersion >= WINDOWS_8 && IsImmersiveProcess && !processItem->IsSubsystemProcess) {
                BOOLEAN isImmersive;

                isImmersive = !!IsImmersiveProcess(processItem->QueryHandle);

                if (processItem->IsImmersive != isImmersive) {
                    processItem->IsImmersive = isImmersive;
                    modified = TRUE;
                }
            }

            if (processItem->QueryHandle && processItem->IsHandleValid) {
                OBJECT_BASIC_INFORMATION basicInfo;
                BOOLEAN filteredHandle = FALSE;

                if (NT_SUCCESS(PhGetHandleInformationEx(
                    NtCurrentProcess(),
                    processItem->QueryHandle,
                    ULONG_MAX,
                    0,
                    NULL,
                    &basicInfo,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                ))) {
                    if ((basicInfo.GrantedAccess & PROCESS_QUERY_INFORMATION) != PROCESS_QUERY_INFORMATION) {
                        filteredHandle = TRUE;
                    }
                } else {
                    filteredHandle = TRUE;
                }

                if (processItem->IsProtectedHandle != filteredHandle) {
                    processItem->IsProtectedHandle = filteredHandle;
                    modified = TRUE;
                }
            }

            if (modified) {
                PhInvokeCallback(PhGetGeneralCallback(GeneralCallbackProcessProviderModifiedEvent), processItem);
            }

            // No reference added by PhpLookupProcessItem.
        }

        // Trick ourselves into thinking that the fake processes
        // are on the list.
        if (process == PhInterruptsProcessInformation) {
            process = NULL;
        } else if (process == PhDpcsProcessInformation) {
            process = PhInterruptsProcessInformation;
        } else {
            process = PH_NEXT_PROCESS(process);

            if (process == NULL) {
                if (PhEnableCycleCpuUsage)
                    process = PhInterruptsProcessInformation;
                else
                    process = PhDpcsProcessInformation;
            }
        }
    }

    if (PhProcessInformation)
        PhFree(PhProcessInformation);

    PhProcessInformation = processes;

    // History cannot be updated on the first run because the deltas are invalid. For example, the
    // I/O "deltas" will be huge because they are currently the raw accumulated values.
    if (runCount != 0) {
        if (PhEnableCycleCpuUsage)
            PhpUpdateCpuCycleUsageInformation(sysTotalCycleTime, sysIdleCycleTime);

        PhpUpdateSystemHistory();

        // Note that we need to add a reference to the records of these processes, to make it
        // possible for others to get the name of a max. CPU or I/O process.

        if (maxCpuProcessItem) {
            PhAddItemCircularBuffer_ULONG(&PhMaxCpuHistory, HandleToUlong(maxCpuProcessItem->ProcessId));
#ifdef PH_RECORD_MAX_USAGE
            PhAddItemCircularBuffer_FLOAT(&PhMaxCpuUsageHistory, maxCpuProcessItem->CpuUsage);
#endif

            if (!(maxCpuProcessItem->Record->Flags & PH_PROCESS_RECORD_STAT_REF)) {
                PhReferenceProcessRecord(maxCpuProcessItem->Record);
                maxCpuProcessItem->Record->Flags |= PH_PROCESS_RECORD_STAT_REF;
            }
        } else {
            PhAddItemCircularBuffer_ULONG(&PhMaxCpuHistory, PtrToUlong(NULL));
#ifdef PH_RECORD_MAX_USAGE
            PhAddItemCircularBuffer_FLOAT(&PhMaxCpuUsageHistory, 0);
#endif
        }

        if (maxIoProcessItem) {
            PhAddItemCircularBuffer_ULONG(&PhMaxIoHistory, HandleToUlong(maxIoProcessItem->ProcessId));
#ifdef PH_RECORD_MAX_USAGE
            PhAddItemCircularBuffer_ULONG64(&PhMaxIoReadOtherHistory,
                                            maxIoProcessItem->IoReadDelta.Delta + maxIoProcessItem->IoOtherDelta.Delta);
            PhAddItemCircularBuffer_ULONG64(&PhMaxIoWriteHistory, maxIoProcessItem->IoWriteDelta.Delta);
#endif

            if (!(maxIoProcessItem->Record->Flags & PH_PROCESS_RECORD_STAT_REF)) {
                PhReferenceProcessRecord(maxIoProcessItem->Record);
                maxIoProcessItem->Record->Flags |= PH_PROCESS_RECORD_STAT_REF;
            }
        } else {
            PhAddItemCircularBuffer_ULONG(&PhMaxIoHistory, PtrToUlong(NULL));
#ifdef PH_RECORD_MAX_USAGE
            PhAddItemCircularBuffer_ULONG64(&PhMaxIoReadOtherHistory, 0);
            PhAddItemCircularBuffer_ULONG64(&PhMaxIoWriteHistory, 0);
#endif
        }
    }

    PhInvokeCallback(PhGetGeneralCallback(GeneralCallbackProcessProviderUpdatedEvent), NULL);
    runCount++;
}

VOID pluginGpuInit(HINSTANCE Instance) {
    PPH_PLUGIN_INFORMATION info;
    PH_SETTING_CREATE settings[] =
    {
        { StringSettingType, SETTING_NAME_DISK_TREE_LIST_COLUMNS, L"" },
        { IntegerPairSettingType, SETTING_NAME_DISK_TREE_LIST_SORT, L"4,2" }, // 4, DescendingSortOrder
        { IntegerSettingType, SETTING_NAME_ENABLE_GPUPERFCOUNTERS, L"0" },
        { IntegerSettingType, SETTING_NAME_ENABLE_DISKEXT, L"1" },
        { IntegerSettingType, SETTING_NAME_ENABLE_ETW_MONITOR, L"1" },
        { IntegerSettingType, SETTING_NAME_ENABLE_GPU_MONITOR, L"1" },
        { IntegerSettingType, SETTING_NAME_ENABLE_SYSINFO_GRAPHS, L"1" },
        { StringSettingType, SETTING_NAME_GPU_NODE_BITMAP, L"01000000" },
        { IntegerSettingType, SETTING_NAME_GPU_LAST_NODE_COUNT, L"0" },
        { IntegerPairSettingType, SETTING_NAME_UNLOADED_WINDOW_POSITION, L"0,0" },
        { ScalableIntegerPairSettingType, SETTING_NAME_UNLOADED_WINDOW_SIZE, L"@96|350,270" },
        { StringSettingType, SETTING_NAME_UNLOADED_COLUMNS, L"" },
        { IntegerPairSettingType, SETTING_NAME_MODULE_SERVICES_WINDOW_POSITION, L"0,0" },
        { ScalableIntegerPairSettingType, SETTING_NAME_MODULE_SERVICES_WINDOW_SIZE, L"@96|850,490" },
        { StringSettingType, SETTING_NAME_MODULE_SERVICES_COLUMNS, L"" },
        { IntegerPairSettingType, SETTING_NAME_GPU_NODES_WINDOW_POSITION, L"0,0" },
        { ScalableIntegerPairSettingType, SETTING_NAME_GPU_NODES_WINDOW_SIZE, L"@96|850,490" },
        { IntegerPairSettingType, SETTING_NAME_WSWATCH_WINDOW_POSITION, L"0,0" },
        { ScalableIntegerPairSettingType, SETTING_NAME_WSWATCH_WINDOW_SIZE, L"@96|325,266" },
        { StringSettingType, SETTING_NAME_WSWATCH_COLUMNS, L"" },
        { StringSettingType, SETTING_NAME_TRAYICON_GUIDS, L"" },
        { IntegerSettingType, SETTING_NAME_ENABLE_FAHRENHEIT, L"0" }
    };

    PluginInstance = PhRegisterPlugin(PLUGIN_NAME, Instance, &info);

    if (!PluginInstance)
        return FALSE;

    info->DisplayName = L"Extended Tools";
    info->Author = L"dmex, wj32";
    info->Description = L"Extended functionality for Windows 7 and above, including ETW monitoring, GPU monitoring and a Disk tab.";
    info->Url = L"https://wj32.org/processhacker/forums/viewtopic.php?t=1114";

    PhRegisterCallback(
        PhGetPluginCallback(PluginInstance, PluginCallbackLoad),
        LoadCallback,
        NULL,
        &PluginLoadCallbackRegistration
    );
    PhRegisterCallback(
        PhGetPluginCallback(PluginInstance, PluginCallbackUnload),
        UnloadCallback,
        NULL,
        &PluginUnloadCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackOptionsWindowInitializing),
        ShowOptionsCallback,
        NULL,
        &PluginShowOptionsCallbackRegistration
    );
    PhRegisterCallback(
        PhGetPluginCallback(PluginInstance, PluginCallbackMenuItem),
        MenuItemCallback,
        NULL,
        &PluginMenuItemCallbackRegistration
    );
    PhRegisterCallback(
        PhGetPluginCallback(PluginInstance, PluginCallbackTreeNewMessage),
        TreeNewMessageCallback,
        NULL,
        &PluginTreeNewMessageCallbackRegistration
    );
    PhRegisterCallback(
        PhGetPluginCallback(PluginInstance, PluginCallbackPhSvcRequest),
        PhSvcRequestCallback,
        NULL,
        &PluginPhSvcRequestCallbackRegistration
    );

    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackMainWindowShowing),
        MainWindowShowingCallback,
        NULL,
        &MainWindowShowingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackProcessesUpdated),
        ProcessesUpdatedCallback,
        NULL,
        &ProcessesUpdatedCallbackRegistration
    );

    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackProcessPropertiesInitializing),
        ProcessPropertiesInitializingCallback,
        NULL,
        &ProcessPropertiesInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackHandlePropertiesInitializing),
        HandlePropertiesInitializingCallback,
        NULL,
        &HandlePropertiesInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackProcessMenuInitializing),
        ProcessMenuInitializingCallback,
        NULL,
        &ProcessMenuInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackThreadMenuInitializing),
        ThreadMenuInitializingCallback,
        NULL,
        &ThreadMenuInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackModuleMenuInitializing),
        ModuleMenuInitializingCallback,
        NULL,
        &ModuleMenuInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackProcessTreeNewInitializing),
        ProcessTreeNewInitializingCallback,
        NULL,
        &ProcessTreeNewInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackNetworkTreeNewInitializing),
        NetworkTreeNewInitializingCallback,
        NULL,
        &NetworkTreeNewInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackSystemInformationInitializing),
        SystemInformationInitializingCallback,
        NULL,
        &SystemInformationInitializingCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackMiniInformationInitializing),
        MiniInformationInitializingCallback,
        NULL,
        &MiniInformationInitializingCallbackRegistration
    );

    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackTrayIconsInitializing),
        TrayIconsInitializingCallback,
        NULL,
        &TrayIconsInitializingCallbackRegistration
    );

    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackProcessProviderUpdatedEvent),
        ProcessItemsUpdatedCallback,
        NULL,
        &ProcessItemsUpdatedCallbackRegistration
    );
    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackNetworkProviderUpdatedEvent),
        NetworkItemsUpdatedCallback,
        NULL,
        &NetworkItemsUpdatedCallbackRegistration
    );

    PhRegisterCallback(
        PhGetGeneralCallback(GeneralCallbackProcessStatsNotifyEvent),
        ProcessStatsEventCallback,
        NULL,
        &ProcessStatsEventCallbackRegistration
    );

    PhPluginSetObjectExtension(
        PluginInstance,
        EmProcessItemType,
        sizeof(ET_PROCESS_BLOCK),
        ProcessItemCreateCallback,
        ProcessItemDeleteCallback
    );
    PhPluginSetObjectExtension(
        PluginInstance,
        EmNetworkItemType,
        sizeof(ET_NETWORK_BLOCK),
        NetworkItemCreateCallback,
        NetworkItemDeleteCallback
    );

    PhAddSettings(settings, RTL_NUMBER_OF(settings));

	//EtEtwStatisticsInitialization();
	EtGpuMonitorInitialization();
}

VOID pluginGpuUnload() {
	EtEtwStatisticsUninitialization();
}

*/
