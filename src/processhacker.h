#ifndef PROCESSHACKER_H
#define PROCESSHACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#define INITGUID

#include <Windows.h>
//#include <winternl.h>

typedef GUID* PGUID;

#include "phnt.h"

typedef struct _PH_LIST {
    /** The number of items in the list. */
    ULONG Count;
    /** The number of items for which storage is allocated. */
    ULONG AllocatedCount;
    /** The array of list items. */
    PVOID* Items;
} PH_LIST, * PPH_LIST;

NTSYSAPI
    VOID
    NTAPI
    RtlInitializeBitMap(
        _Out_ PRTL_BITMAP BitMapHeader,
        _In_ PULONG BitMapBuffer,
        _In_ ULONG SizeOfBitMap
    );

typedef struct _PH_STRINGREF {
    /** The length, in bytes, of the string. */
    SIZE_T Length;
    /** The buffer containing the contents of the string. */
    PWCH Buffer;
} PH_STRINGREF, * PPH_STRINGREF;

/**
 * A 16-bit string object, which supports UTF-16.
 *
 * \remarks The \a Length never includes the null terminator. Every string must have a null
 * terminator at the end, for compatibility reasons. The invariant is:
 * \code Buffer[Length / sizeof(WCHAR)] = 0 \endcode
 */
typedef struct _PH_STRING {
    // Header
    union {
        PH_STRINGREF sr;
        struct {
            /** The length, in bytes, of the string. */
            SIZE_T Length;
            /** The buffer containing the contents of the string. */
            PWCH Buffer;
        };
    };

    // Data
    union {
        WCHAR Data[1];
        struct {
            /** Reserved. */
            ULONG AllocationFlags;
            /** Reserved. */
            PVOID Allocation;
        };
    };
} PH_STRING, * PPH_STRING;

typedef struct _ETP_GPU_ADAPTER {
    LUID AdapterLuid;
    ULONG SegmentCount;
    ULONG NodeCount;
    ULONG FirstNodeIndex;

    PPH_STRING DeviceInterface;
    PPH_STRING Description;
    PPH_LIST NodeNameList;

    RTL_BITMAP ApertureBitMap;
    ULONG ApertureBitMapBuffer[1];
} ETP_GPU_ADAPTER, * PETP_GPU_ADAPTER;

FORCEINLINE VOID PhInitializeStringRef(PPH_STRINGREF String, PWSTR Buffer) {
    String->Length = wcslen(Buffer) * sizeof(WCHAR);
    String->Buffer = Buffer;
}

FORCEINLINE VOID PhInitializeEmptyStringRef(PPH_STRINGREF String) {
    String->Length = 0;
    String->Buffer = NULL;
}

FORCEINLINE BOOLEAN PhStringRefToUnicodeString(PPH_STRINGREF String, PUNICODE_STRING UnicodeString) {
    UnicodeString->Length = (USHORT)String->Length;
    UnicodeString->MaximumLength = (USHORT)String->Length + sizeof(UNICODE_NULL);
    UnicodeString->Buffer = String->Buffer;

    return String->Length <= UNICODE_STRING_MAX_BYTES;
}

/**
 * Determines the length of the specified string, in characters.
 *
 * \param String The string.
 */
SIZE_T PhCountStringZ(
    _In_ PWSTR String
) {
    return wcslen(String);
}

/**
 * Updates a string object's length with its true length as determined by an embedded null
 * terminator.
 *
 * \param String The string to modify.
 *
 * \remarks Use this function after modifying a string object's buffer manually.
 */
FORCEINLINE
VOID
PhTrimToNullTerminatorString(
    _Inout_ PPH_STRING String
) {
    String->Length = PhCountStringZ(String->Buffer) * sizeof(WCHAR);
}

#define BYTES_NEEDED_FOR_BITS(Bits) ((((Bits) + sizeof(ULONG) * 8 - 1) / 8) & ~(SIZE_T)(sizeof(ULONG) - 1)) // divide round up

#define WINDOWS_ANCIENT 0
#define WINDOWS_XP 51
#define WINDOWS_VISTA 60
#define WINDOWS_7 61
#define WINDOWS_8 62
#define WINDOWS_8_1 63
#define WINDOWS_10 100 // TH1
#define WINDOWS_10_TH2 101
#define WINDOWS_10_RS1 102
#define WINDOWS_10_RS2 103
#define WINDOWS_10_RS3 104
#define WINDOWS_10_RS4 105
#define WINDOWS_10_RS5 106
#define WINDOWS_10_19H1 107
#define WINDOWS_10_19H2 108
#define WINDOWS_10_20H1 109
#define WINDOWS_NEW ULONG_MAX

#include <cfgmgr32.h>
#include <devpkey.h>
#include <ntddvdeo.h>
#include <ntstatus.h>

// gpumon

extern BOOLEAN EtGpuEnabled;
extern BOOLEAN EtD3DEnabled;
extern PPH_LIST EtpGpuAdapterList;

extern ULONG EtGpuTotalNodeCount;
extern ULONG EtGpuTotalSegmentCount;
extern ULONG64 EtGpuDedicatedLimit;
extern ULONG64 EtGpuSharedLimit;

typedef struct _PH_UINT64_DELTA {
    ULONG64 Value;
    ULONG64 Delta;
} PH_UINT64_DELTA, * PPH_UINT64_DELTA;

extern PH_UINT64_DELTA EtClockTotalRunningTimeDelta;
extern LARGE_INTEGER EtClockTotalRunningTimeFrequency;
extern FLOAT EtGpuNodeUsage;
//extern PH_CIRCULAR_BUFFER_FLOAT EtGpuNodeHistory;
//extern PH_CIRCULAR_BUFFER_ULONG EtMaxGpuNodeHistory; // ID of max. GPU usage process
//extern PH_CIRCULAR_BUFFER_FLOAT EtMaxGpuNodeUsageHistory;

extern PPH_UINT64_DELTA EtGpuNodesTotalRunningTimeDelta;
//extern PPH_CIRCULAR_BUFFER_FLOAT EtGpuNodesHistory;

extern ULONG64 EtGpuDedicatedUsage;
extern ULONG64 EtGpuSharedUsage;
//extern PH_CIRCULAR_BUFFER_ULONG64 EtGpuDedicatedHistory;
//extern PH_CIRCULAR_BUFFER_ULONG64 EtGpuSharedHistory;

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

PPH_LIST PhCreateList(ULONG InitialCapacity);
void PhAddItemList(PPH_LIST List, PVOID Item);
PVOID PhReAllocate(PVOID Memory, SIZE_T SizeOld, SIZE_T SizeNew);

NTSTATUS PhQueryValueKey(HANDLE KeyHandle, PPH_STRINGREF ValueName, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID* Buffer);
ULONG64 PhQueryRegistryUlong64(HANDLE KeyHandle, PWSTR ValueName);
ULONG PhQueryRegistryUlong(HANDLE KeyHandle, PWSTR ValueName);

BOOLEAN EtpInitializeD3DStatistics();
PETP_GPU_ADAPTER EtpAllocateGpuAdapter(ULONG NumberOfSegments);
BOOLEAN EtpIsGpuSoftwareDevice(D3DKMT_HANDLE AdapterHandle);
PPH_STRING EtpQueryDeviceProperty(DEVINST DeviceHandle, CONST DEVPROPKEY* DeviceProperty);
ULONG64 EtpQueryGpuInstalledMemory(DEVINST DeviceHandle);
PETP_GPU_ADAPTER EtpAddDisplayAdapter(PWSTR DeviceInterface, D3DKMT_HANDLE AdapterHandle, LUID AdapterLuid, ULONG NumberOfSegments, ULONG NumberOfNodes);

void EtGpuMonitorInitialization();
BOOLEAN EtQueryDeviceProperties(PWSTR DeviceInterface, PPH_STRING* Description, PPH_STRING* DriverDate, PPH_STRING* DriverVersion, PPH_STRING* LocationInfo, ULONG64* InstalledMemory);
NTSTATUS EtQueryAdapterInformation(D3DKMT_HANDLE AdapterHandle, KMTQUERYADAPTERINFOTYPE InformationClass, PVOID Information, UINT32 InformationLength);
BOOLEAN EtCloseAdapterHandle(D3DKMT_HANDLE AdapterHandle);

#ifdef __cplusplus
}
#endif


#endif
