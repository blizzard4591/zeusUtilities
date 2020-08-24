#include "cpuload.h"

#include <ntstatus.h>

#include <algorithm>
#include <iostream>

#include <string>
#include <QString>

CpuLoad::CpuLoad() : mIterationCount(0), mLastValues(nullptr), mCurrentValues(nullptr), mProcessorCount(0) {
    SYSTEM_INFO info = { 0 };
    GetSystemInfo(&info);
    mProcessorCount = info.dwNumberOfProcessors;

    mLastValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[mProcessorCount];
    mCurrentValues = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[mProcessorCount];

    for (DWORD i = 0; i < mProcessorCount; ++i) {
        mCpuLoadPerCore.push_back(0.0);
    }

    mProcessInformationSize = 1 * sizeof(SYSTEM_PROCESS_INFORMATION) + 10 * sizeof(SYSTEM_THREAD_INFORMATION);
    mProcessInformation = malloc(mProcessInformationSize);
}

CpuLoad::~CpuLoad() {
    if (mLastValues != nullptr) {
        delete[] mLastValues;
    }
    if (mCurrentValues != nullptr) {
        delete[] mCurrentValues;
    }
    if (mProcessInformation != nullptr) {
        delete mProcessInformation;
    }
}

double CpuLoad::getCpuLoadOfCore(std::size_t core) const {
    return mCpuLoadPerCore.at(core);
}

std::size_t CpuLoad::getCoreCount() const {
    return mProcessorCount;
}

std::string CpuLoad::ws2s(std::wstring const& wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

void CpuLoad::update() {
    // Make room for new data, swapping the current to last
    std::swap(mCurrentValues, mLastValues);

    ULONG size;
    NtQuerySystemInformation(SystemProcessorPerformanceInformation, mCurrentValues, sizeof(mCurrentValues[0]) * mProcessorCount, &size);
    if (mIterationCount > 0) {
        for (DWORD i = 0; i < mProcessorCount; ++i) {
            double current_percent = (mCurrentValues[i].IdleTime.QuadPart - mLastValues[i].IdleTime.QuadPart) * 100.0;
            current_percent /= ((mCurrentValues[i].KernelTime.QuadPart + mCurrentValues[i].UserTime.QuadPart) - (mLastValues[i].KernelTime.QuadPart + mLastValues[i].UserTime.QuadPart));
            current_percent = 100.0 - current_percent;
            mCpuLoadPerCore.at(i) = current_percent;
        }
    }

    // Per-process information
    ULONG requiredSize = 0;
    NTSTATUS result;
    
    do {
        if (requiredSize > 0) {
            delete mProcessInformation;
            mProcessInformationSize = static_cast<std::size_t>(1.5 * requiredSize);
            mProcessInformation = malloc(mProcessInformationSize);
            std::cout << "Debug: resizing process info buffer to " << mProcessInformationSize << " Bytes." << std::endl;
        }
        result = NtQuerySystemInformation(SystemProcessInformation, mProcessInformation, static_cast<ULONG>(mProcessInformationSize), &requiredSize);
    } while (result == STATUS_INFO_LENGTH_MISMATCH);
    
    
    if (result != STATUS_SUCCESS) {
        std::cerr << "Return code was: " << result << " and required size was " << requiredSize << "!" << std::endl;
    } else {
        ULONG currentOffset = 0;
        uint8_t* basePointer = static_cast<uint8_t*>(mProcessInformation);

        while (true) {
            PSYSTEM_PROCESS_INFORMATION pointer = reinterpret_cast<PSYSTEM_PROCESS_INFORMATION>(basePointer + currentOffset);
            std::wstring const wImageName(pointer->ImageName.Buffer, pointer->ImageName.Length / sizeof(WCHAR));
            QString const imageName = QString::fromStdWString(wImageName);

            //std::string imageName = ws2s(wImageName);
            std::cout << "#Threads: " << pointer->NumberOfThreads << ",Image:" << imageName.toStdString() << ",VirtSize:" << pointer->VirtualSize << std::endl;

            currentOffset += pointer->NextEntryOffset;
            if (pointer->NextEntryOffset == 0) {
                break;
            }
        }
    }

    ++mIterationCount;
}
