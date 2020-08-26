#ifndef PROCESSHACKER_H
#define PROCESSHACKER_H

#ifdef __cplusplus
extern "C" {
#endif

#define INITGUID

#include <Windows.h>
#include <winternl.h>

typedef GUID* PGUID;

typedef struct _PH_LIST {
    /** The number of items in the list. */
    ULONG Count;
    /** The number of items for which storage is allocated. */
    ULONG AllocatedCount;
    /** The array of list items. */
    PVOID* Items;
} PH_LIST, * PPH_LIST;

typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;
    PULONG Buffer;
} RTL_BITMAP, * PRTL_BITMAP;

NTSYSAPI
    VOID
    NTAPI
    RtlInitializeBitMap(
        _Out_ PRTL_BITMAP BitMapHeader,
        _In_ PULONG BitMapBuffer,
        _In_ ULONG SizeOfBitMap
    );

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

typedef struct _PH_STRINGREF {
    /** The length, in bytes, of the string. */
    SIZE_T Length;
    /** The buffer containing the contents of the string. */
    PWCH Buffer;
} PH_STRINGREF, * PPH_STRINGREF;

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

#include "ntd3dkmt.h"
#include <cfgmgr32.h>
#include <devpkey.h>
#include <ntddvdeo.h>

#include "ntregapi.h"
#include "ntrtl.h"

ULONG WindowsVersion;

PPH_LIST EtpGpuAdapterList;
ULONG EtGpuTotalNodeCount = 0;
ULONG EtGpuTotalSegmentCount = 0;
ULONG EtGpuNextNodeIndex = 0;
ULONG64 EtGpuDedicatedLimit = 0;
ULONG64 EtGpuDedicatedUsage = 0;
ULONG64 EtGpuSharedLimit = 0;
ULONG64 EtGpuSharedUsage = 0;

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

#include <QDateTime>

QDateTime fromFileTime(PFILETIME filetime);

#endif
