#include "gpu_query.h"

#include <Pdh.h>
#include <PdhMsg.h>

typedef HANDLE       PDH_HQUERY;
typedef HANDLE       PDH_HCOUNTER;

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE;

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)
#define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))

extern VOID ParseGpuEngineUtilizationCounter(
    _In_ PWSTR InstanceName,
    _In_ DOUBLE InstanceValue
);

extern VOID ParseGpuProcessMemoryDedicatedUsageCounter(
    _In_ PWSTR InstanceName,
    _In_ ULONG64 InstanceValue
);

extern VOID ParseGpuProcessMemorySharedUsageCounter(
    _In_ PWSTR InstanceName,
    _In_ ULONG64 InstanceValue
);

extern VOID ParseGpuProcessMemoryCommitUsageCounter(
    _In_ PWSTR InstanceName,
    _In_ ULONG64 InstanceValue
);

extern VOID AquireGpuLock();
extern VOID ReleaseGpuLock();

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    _Field_size_bytes_part_(MaximumLength, Length) PWCH Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor; // PSECURITY_DESCRIPTOR;
    PVOID SecurityQualityOfService; // PSECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateEvent(
    _Out_ PHANDLE EventHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ EVENT_TYPE EventType,
    _In_ BOOLEAN InitialState
);

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenEvent(
    _Out_ PHANDLE EventHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
);

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitForSingleObject(
    _In_ HANDLE Handle,
    _In_ BOOLEAN Alertable,
    _In_opt_ PLARGE_INTEGER Timeout
);

NTSYSCALLAPI
NTSTATUS
NTAPI
NtClose(
    _In_ _Post_ptr_invalid_ HANDLE Handle
);

BOOLEAN GetCounterArrayBuffer(
    _In_ PDH_HCOUNTER CounterHandle,
    _In_ ULONG Format,
    _Out_ ULONG* ArrayCount,
    _Out_ PPDH_FMT_COUNTERVALUE_ITEM* Array
) {
    PDH_STATUS status;
    ULONG bufferLength = 0;
    ULONG bufferCount = 0;
    PPDH_FMT_COUNTERVALUE_ITEM buffer = NULL;

    status = PdhGetFormattedCounterArray(
        CounterHandle,
        Format,
        &bufferLength,
        &bufferCount,
        NULL
    );

    if (status == PDH_MORE_DATA) {
        buffer = malloc(bufferLength);

        status = PdhGetFormattedCounterArray(
            CounterHandle,
            Format,
            &bufferLength,
            &bufferCount,
            buffer
        );
    }

    if (status == ERROR_SUCCESS) {
        if (ArrayCount) {
            *ArrayCount = bufferCount;
        }

        if (Array) {
            *Array = buffer;
        }

        return TRUE;
    }

    if (buffer)
        free(buffer);

    return FALSE;
}

HANDLE gpuCounterQueryEvent = NULL;
PDH_HQUERY gpuPerfCounterQueryHandle = NULL;
PDH_HCOUNTER gpuPerfCounterRunningTimeHandle = NULL;
PDH_HCOUNTER gpuPerfCounterDedicatedUsageHandle = NULL;
PDH_HCOUNTER gpuPerfCounterSharedUsageHandle = NULL;
PDH_HCOUNTER gpuPerfCounterCommittedUsageHandle = NULL;
PPDH_FMT_COUNTERVALUE_ITEM buffer;
ULONG bufferCount;

LONG gpuQueriesInit() {
    if (!NT_SUCCESS(NtCreateEvent(&gpuCounterQueryEvent, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE)))
        return 1;

    if (PdhOpenQuery(NULL, 0, &gpuPerfCounterQueryHandle) != ERROR_SUCCESS)
        return 2;

    // \GPU Engine(*)\Running Time
    // \GPU Engine(*)\Utilization Percentage                                                           
    // \GPU Process Memory(*)\Shared Usage
    // \GPU Process Memory(*)\Dedicated Usage
    // \GPU Process Memory(*)\Non Local Usage
    // \GPU Process Memory(*)\Local Usage
    // \GPU Adapter Memory(*)\Shared Usage
    // \GPU Adapter Memory(*)\Dedicated Usage
    // \GPU Adapter Memory(*)\Total Committed
    // \GPU Local Adapter Memory(*)\Local Usage
    // \GPU Non Local Adapter Memory(*)\Non Local Usage

    if (PdhAddCounter(gpuPerfCounterQueryHandle, L"\\GPU Engine(*)\\Utilization Percentage", 0, &gpuPerfCounterRunningTimeHandle) != ERROR_SUCCESS)
        return 3;
    if (PdhAddCounter(gpuPerfCounterQueryHandle, L"\\GPU Process Memory(*)\\Shared Usage", 0, &gpuPerfCounterSharedUsageHandle) != ERROR_SUCCESS)
        return 4;
    if (PdhAddCounter(gpuPerfCounterQueryHandle, L"\\GPU Process Memory(*)\\Dedicated Usage", 0, &gpuPerfCounterDedicatedUsageHandle) != ERROR_SUCCESS)
        return 5;
    if (PdhAddCounter(gpuPerfCounterQueryHandle, L"\\GPU Process Memory(*)\\Total Committed", 0, &gpuPerfCounterCommittedUsageHandle) != ERROR_SUCCESS)
        return 6;

    if (PdhCollectQueryDataEx(gpuPerfCounterQueryHandle, 1, gpuCounterQueryEvent) != ERROR_SUCCESS)
        return 7;

    return 0;
}

VOID gpuQueriesCleanup() {
    if (gpuPerfCounterQueryHandle)
        PdhCloseQuery(gpuPerfCounterQueryHandle);
    if (gpuCounterQueryEvent)
        NtClose(gpuCounterQueryEvent);
}

LONG runGpuQueries() {
    if (NtWaitForSingleObject(gpuCounterQueryEvent, FALSE, NULL) != WAIT_OBJECT_0)
        return 1;

    AquireGpuLock();
    if (GetCounterArrayBuffer(
        gpuPerfCounterRunningTimeHandle,
        PDH_FMT_DOUBLE,
        &bufferCount,
        &buffer
    )) {
        //AquireGpuLock();
        for (ULONG i = 0; i < bufferCount; i++) {
            PPDH_FMT_COUNTERVALUE_ITEM entry = PTR_ADD_OFFSET(buffer, sizeof(PDH_FMT_COUNTERVALUE_ITEM) * i);

            if (entry->FmtValue.CStatus)
                continue;
            if (entry->FmtValue.doubleValue == 0)
                continue;

            ParseGpuEngineUtilizationCounter(entry->szName, entry->FmtValue.doubleValue);
        }
        //ReleaseGpuLock();

        free(buffer);
    }

    if (GetCounterArrayBuffer(
        gpuPerfCounterDedicatedUsageHandle,
        PDH_FMT_LARGE,
        &bufferCount,
        &buffer
    )) {
        //AquireGpuLock();
        for (ULONG i = 0; i < bufferCount; i++) {
            PPDH_FMT_COUNTERVALUE_ITEM entry = PTR_ADD_OFFSET(buffer, sizeof(PDH_FMT_COUNTERVALUE_ITEM) * i);

            if (entry->FmtValue.CStatus)
                continue;
            if (entry->FmtValue.largeValue == 0)
                continue;

            ParseGpuProcessMemoryDedicatedUsageCounter(entry->szName, entry->FmtValue.largeValue);
        }
        //ReleaseGpuLock();

        free(buffer);
    }

    if (GetCounterArrayBuffer(
        gpuPerfCounterSharedUsageHandle,
        PDH_FMT_LARGE,
        &bufferCount,
        &buffer
    )) {
        //AquireGpuLock();
        for (ULONG i = 0; i < bufferCount; i++) {
            PPDH_FMT_COUNTERVALUE_ITEM entry = PTR_ADD_OFFSET(buffer, sizeof(PDH_FMT_COUNTERVALUE_ITEM) * i);

            if (entry->FmtValue.CStatus)
                continue;
            if (entry->FmtValue.largeValue == 0)
                continue;

            ParseGpuProcessMemorySharedUsageCounter(entry->szName, entry->FmtValue.largeValue);
        }
        //ReleaseGpuLock();

        free(buffer);
    }

    if (GetCounterArrayBuffer(
        gpuPerfCounterCommittedUsageHandle,
        PDH_FMT_LARGE,
        &bufferCount,
        &buffer
    )) {
        //AquireGpuLock();
        for (ULONG i = 0; i < bufferCount; i++) {
            PPDH_FMT_COUNTERVALUE_ITEM entry = PTR_ADD_OFFSET(buffer, sizeof(PDH_FMT_COUNTERVALUE_ITEM) * i);

            if (entry->FmtValue.CStatus)
                continue;
            if (entry->FmtValue.largeValue == 0)
                continue;

            ParseGpuProcessMemoryCommitUsageCounter(entry->szName, entry->FmtValue.largeValue);
        }
        //ReleaseGpuLock();

        free(buffer);
    }
    ReleaseGpuLock();

    return 0;
}
