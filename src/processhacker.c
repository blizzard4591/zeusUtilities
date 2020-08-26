#include "processhacker.h"

void EtGpuMonitorInitialization() {

}

PPH_LIST PhCreateList(ULONG InitialCapacity) {
    PPH_LIST list;

    list = (PPH_LIST)malloc(sizeof(PH_LIST));

    // Initial capacity of 0 is not allowed.
    if (InitialCapacity == 0)
        InitialCapacity = 1;

    list->Count = 0;
    list->AllocatedCount = InitialCapacity;
    list->Items = (PVOID*)malloc(list->AllocatedCount * sizeof(PVOID));

    return list;
}

void PhAddItemList(PPH_LIST List, PVOID Item) {
    // See if we need to resize the list.
    if (List->Count == List->AllocatedCount) {
        ULONG SizeOld = List->AllocatedCount;
        List->AllocatedCount *= 2;
        List->Items = (PVOID*)PhReAllocate(List->Items, SizeOld * sizeof(PVOID), List->AllocatedCount * sizeof(PVOID));
    }

    List->Items[List->Count++] = Item;
}

PVOID PhReAllocate(PVOID Memory, SIZE_T SizeOld, SIZE_T SizeNew) {
    PVOID result = malloc(SizeNew);
    if (result == 0) {
        return result;
    }
    memcpy(result, Memory, SizeOld);
    free(Memory);

    return result;
}

bool EtpIsGpuSoftwareDevice(D3DKMT_HANDLE AdapterHandle) {
    D3DKMT_ADAPTERTYPE adapterType;

    memset(&adapterType, 0, sizeof(D3DKMT_ADAPTERTYPE));

    if (NT_SUCCESS(EtQueryAdapterInformation(
        AdapterHandle,
        KMTQAITYPE_ADAPTERTYPE,
        &adapterType,
        sizeof(D3DKMT_ADAPTERTYPE)
    ))) {
        if (adapterType.SoftwareDevice) // adapterType.HybridIntegrated
        {
            return true;
        }
    }

    return false;
}

bool EtCloseAdapterHandle(D3DKMT_HANDLE AdapterHandle) {
    D3DKMT_CLOSEADAPTER closeAdapter;

    memset(&closeAdapter, 0, sizeof(D3DKMT_CLOSEADAPTER));
    closeAdapter.AdapterHandle = AdapterHandle;

    return NT_SUCCESS(D3DKMTCloseAdapter(&closeAdapter));
}

NTSTATUS EtQueryAdapterInformation(D3DKMT_HANDLE AdapterHandle, KMTQUERYADAPTERINFOTYPE InformationClass, PVOID Information, UINT32 InformationLength) {
    D3DKMT_QUERYADAPTERINFO queryAdapterInfo;

    memset(&queryAdapterInfo, 0, sizeof(D3DKMT_QUERYADAPTERINFO));

    queryAdapterInfo.AdapterHandle = AdapterHandle;
    queryAdapterInfo.Type = InformationClass;
    queryAdapterInfo.PrivateDriverData = Information;
    queryAdapterInfo.PrivateDriverDataSize = InformationLength;

    return D3DKMTQueryAdapterInfo(&queryAdapterInfo);
}

PETP_GPU_ADAPTER EtpAllocateGpuAdapter(ULONG NumberOfSegments) {
    PETP_GPU_ADAPTER adapter;
    SIZE_T sizeNeeded;

    sizeNeeded = FIELD_OFFSET(ETP_GPU_ADAPTER, ApertureBitMapBuffer);
    sizeNeeded += BYTES_NEEDED_FOR_BITS(NumberOfSegments);

    adapter = (PETP_GPU_ADAPTER)malloc(sizeNeeded);
    memset(adapter, 0, sizeNeeded);

    return adapter;
}

QDateTime fromFileTime(PFILETIME fileTime) {
    // Definition of FILETIME from MSDN:
    // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
    QDateTime origin(QDate(1601, 1, 1), QTime(0, 0, 0, 0), Qt::UTC);

    LARGE_INTEGER time;
    time.HighPart = fileTime->dwHighDateTime;
    time.LowPart = fileTime->dwLowDateTime;

    qint64 const timeInMSecs = time.QuadPart / 10000;

    return origin.addMSecs(timeInMSecs);
}

PPH_STRING EtpQueryDeviceProperty(DEVINST DeviceHandle, CONST DEVPROPKEY* DeviceProperty) {
    CONFIGRET result;
    PBYTE buffer;
    ULONG bufferSize;
    DEVPROPTYPE propertyType;

    bufferSize = 0x80;
    buffer = (PBYTE)malloc(bufferSize);
    propertyType = DEVPROP_TYPE_EMPTY;

    if ((result = CM_Get_DevNode_PropertyW(
        DeviceHandle,
        DeviceProperty,
        &propertyType,
        buffer,
        &bufferSize,
        0
    )) != CR_SUCCESS) {
        free(buffer);
        buffer = (PBYTE)malloc(bufferSize);

        result = CM_Get_DevNode_PropertyW(
            DeviceHandle,
            DeviceProperty,
            &propertyType,
            buffer,
            &bufferSize,
            0
        );
    }

    if (result != CR_SUCCESS) {
        free(buffer);
        return QString();
    }

    switch (propertyType) {
    case DEVPROP_TYPE_STRING:
    {
        QString string;

        std::wstring const wString((PWCHAR)buffer, bufferSize);
        QString const string = QString::fromStdWString(wString);

        //PhTrimToNullTerminatorString(string);

        free(buffer);
        return string;
    }
    break;
    case DEVPROP_TYPE_FILETIME:
    {
        QString string;
        PFILETIME fileTime;
        LARGE_INTEGER time;
        SYSTEMTIME systemTime;

        fileTime = (PFILETIME)buffer;
        time.HighPart = fileTime->dwHighDateTime;
        time.LowPart = fileTime->dwLowDateTime;

        QDateTime const dateTime = fromFileTime((PFILETIME)buffer);
        string = dateTime.toString("dd.MM.yyyy hh:mm:ss");

        free(buffer);
        return string;
    }
    break;
    case DEVPROP_TYPE_UINT32:
    {
        QString string = QString::number(*(PULONG)buffer, 10);

        free(buffer);
        return string;
    }
    break;
    case DEVPROP_TYPE_UINT64:
    {
        QString string = QString::number(*(PULONG64)buffer, 10);

        free(buffer);
        return string;
    }
    break;
    }

    return QString();
}

/**
 * Gets a registry value of any type.
 *
 * \param KeyHandle A handle to the key.
 * \param ValueName The name of the value.
 * \param KeyValueInformationClass The information class to query.
 * \param Buffer A variable which receives a pointer to a buffer containing information about the
 * registry value. You must free the buffer with PhFree() when you no longer need it.
 */
NTSTATUS PhQueryValueKey(HANDLE KeyHandle, PPH_STRINGREF ValueName, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID* Buffer) {
    NTSTATUS status;
    UNICODE_STRING valueName;
    PVOID buffer;
    ULONG bufferSize;
    ULONG attempts = 16;

    if (ValueName) {
        if (!PhStringRefToUnicodeString(ValueName, &valueName))
            return STATUS_NAME_TOO_LONG;
    } else {
        RtlInitEmptyUnicodeString(&valueName, NULL, 0);
    }

    bufferSize = 0x100;
    buffer = malloc(bufferSize);

    do {
        status = NtQueryValueKey(
            KeyHandle,
            &valueName,
            KeyValueInformationClass,
            buffer,
            bufferSize,
            &bufferSize
        );

        if (NT_SUCCESS(status))
            break;

        if (status == STATUS_BUFFER_OVERFLOW) {
            free(buffer);
            buffer = malloc(bufferSize);
        } else {
            free(buffer);
            return status;
        }
    } while (--attempts);

    *Buffer = buffer;

    return status;
}

ULONG64 PhQueryRegistryUlong64(HANDLE KeyHandle, PWSTR ValueName) {
    ULONG64 ulong64 = ULLONG_MAX;
    PH_STRINGREF valueName;
    PKEY_VALUE_PARTIAL_INFORMATION buffer;

    if (ValueName)
        PhInitializeStringRef(&valueName, ValueName);
    else
        PhInitializeEmptyStringRef(&valueName);

    if (NT_SUCCESS(PhQueryValueKey(KeyHandle, &valueName, KeyValuePartialInformation, &buffer))) {
        if (buffer->Type == REG_DWORD) {
            if (buffer->DataLength == sizeof(ULONG))
                ulong64 = *(PULONG)buffer->Data;
        } else if (buffer->Type == REG_QWORD) {
            if (buffer->DataLength == sizeof(ULONG64))
                ulong64 = *(PULONG64)buffer->Data;
        }

        free(buffer);
    }

    return ulong64;
}

ULONG PhQueryRegistryUlong(HANDLE KeyHandle, PWSTR ValueName) {
    ULONG ulong = ULONG_MAX;
    PH_STRINGREF valueName;
    PKEY_VALUE_PARTIAL_INFORMATION buffer;

    if (ValueName)
        PhInitializeStringRef(&valueName, ValueName);
    else
        PhInitializeEmptyStringRef(&valueName);

    if (NT_SUCCESS(PhQueryValueKey(KeyHandle, &valueName, KeyValuePartialInformation, &buffer))) {
        if (buffer->Type == REG_DWORD) {
            if (buffer->DataLength == sizeof(ULONG))
                ulong = *(PULONG)buffer->Data;
        }

        free(buffer);
    }

    return ulong;
}

ULONG64 EtpQueryGpuInstalledMemory(DEVINST DeviceHandle) {
    ULONG64 installedMemory = ULLONG_MAX;
    HKEY keyHandle;

    if (CM_Open_DevInst_Key(
        DeviceHandle,
        KEY_READ,
        0,
        RegDisposition_OpenExisting,
        &keyHandle,
        CM_REGISTRY_SOFTWARE
    ) == CR_SUCCESS) {
        installedMemory = PhQueryRegistryUlong64(keyHandle, L"HardwareInformation.qwMemorySize");

        if (installedMemory == ULLONG_MAX)
            installedMemory = PhQueryRegistryUlong(keyHandle, L"HardwareInformation.MemorySize");

        if (installedMemory == ULONG_MAX) // HACK
            installedMemory = ULLONG_MAX;

        // Intel GPU devices incorrectly create the key with type REG_BINARY.
        if (installedMemory == ULLONG_MAX) {
            PH_STRINGREF valueName;
            PKEY_VALUE_PARTIAL_INFORMATION buffer;

            PhInitializeStringRef(&valueName, L"HardwareInformation.MemorySize");

            if (NT_SUCCESS(PhQueryValueKey(keyHandle, &valueName, KeyValuePartialInformation, &buffer))) {
                if (buffer->Type == REG_BINARY) {
                    if (buffer->DataLength == sizeof(ULONG))
                        installedMemory = *(PULONG)buffer->Data;
                }

                PhFree(buffer);
            }
        }

        NtClose(keyHandle);
    }

    return installedMemory;
}

bool EtQueryDeviceProperties(QString const& DeviceInterface, QString* Description, QString* DriverDate, QString* DriverVersion, QString* LocationInfo, ULONG64* InstalledMemory) {
    DEVPROPTYPE devicePropertyType;
    DEVINST deviceInstanceHandle;
    ULONG deviceInstanceIdLength = MAX_DEVICE_ID_LEN;
    WCHAR deviceInstanceId[MAX_DEVICE_ID_LEN];

    if (CM_Get_Device_Interface_PropertyW(
        DeviceInterface.toStdWString().c_str(),
        &DEVPKEY_Device_InstanceId,
        &devicePropertyType,
        (PBYTE)deviceInstanceId,
        &deviceInstanceIdLength,
        0
    ) != CR_SUCCESS) {
        return FALSE;
    }

    if (CM_Locate_DevNodeW(
        &deviceInstanceHandle,
        deviceInstanceId,
        CM_LOCATE_DEVNODE_NORMAL
    ) != CR_SUCCESS) {
        return FALSE;
    }

    if (Description)
        *Description = EtpQueryDeviceProperty(deviceInstanceHandle, &DEVPKEY_Device_DeviceDesc);
    if (DriverDate)
        *DriverDate = EtpQueryDeviceProperty(deviceInstanceHandle, &DEVPKEY_Device_DriverDate);
    if (DriverVersion)
        *DriverVersion = EtpQueryDeviceProperty(deviceInstanceHandle, &DEVPKEY_Device_DriverVersion);
    if (LocationInfo)
        *LocationInfo = EtpQueryDeviceProperty(deviceInstanceHandle, &DEVPKEY_Device_LocationInfo);
    if (InstalledMemory)
        *InstalledMemory = EtpQueryGpuInstalledMemory(deviceInstanceHandle);
    // EtpQueryDeviceProperty(deviceInstanceHandle, &DEVPKEY_Device_Manufacturer);

    // Undocumented device properties (Win10 only)
    //DEFINE_DEVPROPKEY(DEVPKEY_Gpu_Luid, 0x60b193cb, 0x5276, 0x4d0f, 0x96, 0xfc, 0xf1, 0x73, 0xab, 0xad, 0x3e, 0xc6, 2); // DEVPROP_TYPE_UINT64
    //DEFINE_DEVPROPKEY(DEVPKEY_Gpu_PhysicalAdapterIndex, 0x60b193cb, 0x5276, 0x4d0f, 0x96, 0xfc, 0xf1, 0x73, 0xab, 0xad, 0x3e, 0xc6, 3); // DEVPROP_TYPE_UINT32

    return TRUE;
}

PETP_GPU_ADAPTER EtpAddDisplayAdapter(QString const& DeviceInterface, D3DKMT_HANDLE AdapterHandle, LUID AdapterLuid, ULONG NumberOfSegments, ULONG NumberOfNodes) {
    PETP_GPU_ADAPTER adapter;

    adapter = EtpAllocateGpuAdapter(NumberOfSegments);
    adapter->DeviceInterface = DeviceInterface;
    adapter->AdapterLuid = AdapterLuid;
    adapter->NodeCount = NumberOfNodes;
    adapter->SegmentCount = NumberOfSegments;
    RtlInitializeBitMap(&adapter->ApertureBitMap, adapter->ApertureBitMapBuffer, NumberOfSegments);

    {
        QString description;

        if (EtQueryDeviceProperties(DeviceInterface, &description, NULL, NULL, NULL, NULL)) {
            adapter->Description = description;
        }
    }

    if (WindowsVersion >= WINDOWS_10_RS4) // Note: Changed to RS4 due to reports of BSODs on LTSB versions of RS3
    {
        adapter->NodeNameList = PhCreateList(adapter->NodeCount);

        for (ULONG i = 0; i < adapter->NodeCount; i++) {
            D3DKMT_NODEMETADATA metaDataInfo;

            memset(&metaDataInfo, 0, sizeof(D3DKMT_NODEMETADATA));
            metaDataInfo.NodeOrdinalAndAdapterIndex = MAKEWORD(i, 0);

            if (NT_SUCCESS(EtQueryAdapterInformation(
                AdapterHandle,
                KMTQAITYPE_NODEMETADATA,
                &metaDataInfo,
                sizeof(D3DKMT_NODEMETADATA)
            ))) {
                PhAddItemList(adapter->NodeNameList, EtpGetNodeEngineTypeString(metaDataInfo));
            } else {
                PhAddItemList(adapter->NodeNameList, PhReferenceEmptyString());
            }
        }
    }

    PhAddItemList(EtpGpuAdapterList, adapter);

    return adapter;
}

bool EtpInitializeD3DStatistics() {
    PPH_LIST deviceAdapterList;
    PZZSTR deviceInterfaceList;
    ULONG deviceInterfaceListLength = 0;
    PZZSTR deviceInterface;
    D3DKMT_OPENADAPTERFROMDEVICENAME openAdapterFromDeviceName;
    D3DKMT_QUERYSTATISTICS queryStatistics;

    if (CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, (PGUID)&GUID_DISPLAY_DEVICE_ARRIVAL, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS) {
        return false;
    }

    deviceInterfaceList = (PZZSTR)malloc(deviceInterfaceListLength * sizeof(WCHAR));
    if (deviceInterfaceList == 0) {
        return false;
    }

    memset(deviceInterfaceList, 0, deviceInterfaceListLength * sizeof(WCHAR));

    if (CM_Get_Device_Interface_List(
        (PGUID)&GUID_DISPLAY_DEVICE_ARRIVAL,
        NULL,
        deviceInterfaceList,
        deviceInterfaceListLength,
        CM_GET_DEVICE_INTERFACE_LIST_PRESENT
    ) != CR_SUCCESS) {
        free(deviceInterfaceList);
        return false;
    }

    deviceAdapterList = PhCreateList(10);

    for (deviceInterface = deviceInterfaceList; *deviceInterface; deviceInterface += strlen(deviceInterface) + 1) {
        PhAddItemList(deviceAdapterList, deviceInterface);
    }

    for (ULONG i = 0; i < deviceAdapterList->Count; i++) {
        memset(&openAdapterFromDeviceName, 0, sizeof(D3DKMT_OPENADAPTERFROMDEVICENAME));
        openAdapterFromDeviceName.DeviceName = deviceAdapterList->Items[i];

        if (!NT_SUCCESS(D3DKMTOpenAdapterFromDeviceName(&openAdapterFromDeviceName)))
            continue;

        if (WindowsVersion >= WINDOWS_10_RS4 && deviceAdapterList->Count > 1) // Note: Changed to RS4 due to reports of BSODs on LTSB versions of RS3
        {
            if (EtpIsGpuSoftwareDevice(openAdapterFromDeviceName.AdapterHandle)) {
                EtCloseAdapterHandle(openAdapterFromDeviceName.AdapterHandle);
                continue;
            }
        }

        if (WindowsVersion >= WINDOWS_10_RS4) // Note: Changed to RS4 due to reports of BSODs on LTSB versions of RS3
        {
            D3DKMT_SEGMENTSIZEINFO segmentInfo;

            memset(&segmentInfo, 0, sizeof(D3DKMT_SEGMENTSIZEINFO));

            if (NT_SUCCESS(EtQueryAdapterInformation(
                openAdapterFromDeviceName.AdapterHandle,
                KMTQAITYPE_GETSEGMENTSIZE,
                &segmentInfo,
                sizeof(D3DKMT_SEGMENTSIZEINFO)
            ))) {
                EtGpuDedicatedLimit += segmentInfo.DedicatedVideoMemorySize;
                EtGpuSharedLimit += segmentInfo.SharedSystemMemorySize;
            }
        }

        memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
        queryStatistics.Type = D3DKMT_QUERYSTATISTICS_ADAPTER;
        queryStatistics.AdapterLuid = openAdapterFromDeviceName.AdapterLuid;

        if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics))) {
            PETP_GPU_ADAPTER gpuAdapter;

            gpuAdapter = EtpAddDisplayAdapter(
                openAdapterFromDeviceName.DeviceName,
                openAdapterFromDeviceName.AdapterHandle,
                openAdapterFromDeviceName.AdapterLuid,
                queryStatistics.QueryResult.AdapterInformation.NbSegments,
                queryStatistics.QueryResult.AdapterInformation.NodeCount
            );

            gpuAdapter->FirstNodeIndex = EtGpuNextNodeIndex;
            EtGpuTotalNodeCount += gpuAdapter->NodeCount;
            EtGpuTotalSegmentCount += gpuAdapter->SegmentCount;
            EtGpuNextNodeIndex += gpuAdapter->NodeCount;

            for (ULONG ii = 0; ii < gpuAdapter->SegmentCount; ii++) {
                memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
                queryStatistics.Type = D3DKMT_QUERYSTATISTICS_SEGMENT;
                queryStatistics.AdapterLuid = gpuAdapter->AdapterLuid;
                queryStatistics.QuerySegment.SegmentId = ii;

                if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics))) {
                    ULONG64 commitLimit;
                    ULONG aperture;

                    if (WindowsVersion >= WINDOWS_8) {
                        commitLimit = queryStatistics.QueryResult.SegmentInformation.CommitLimit;
                        aperture = queryStatistics.QueryResult.SegmentInformation.Aperture;
                    } else {
                        commitLimit = queryStatistics.QueryResult.SegmentInformationV1.CommitLimit;
                        aperture = queryStatistics.QueryResult.SegmentInformationV1.Aperture;
                    }

                    if (WindowsVersion < WINDOWS_10_RS4) // Note: Changed to RS4 due to reports of BSODs on LTSB versions of RS3
                    {
                        if (aperture)
                            EtGpuSharedLimit += commitLimit;
                        else
                            EtGpuDedicatedLimit += commitLimit;
                    }

                    if (aperture)
                        RtlSetBits(&gpuAdapter->ApertureBitMap, ii, 1);
                }
            }
        }

        EtCloseAdapterHandle(openAdapterFromDeviceName.AdapterHandle);
    }

    PhDereferenceObject(deviceAdapterList);
    PhFree(deviceInterfaceList);

    if (EtGpuTotalNodeCount == 0)
        return false;

    return true;
}
