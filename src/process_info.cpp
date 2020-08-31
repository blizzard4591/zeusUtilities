#include "process_info.h"

ProcessInfo::ProcessInfo(PFULL_SYSTEM_PROCESS_INFORMATION processInformation) : ImageName(QString::fromStdWString(std::wstring(processInformation->ImageName.Buffer, processInformation->ImageName.Length / sizeof(WCHAR)))), UniqueProcessId(processInformation->UniqueProcessId), UserTime(processInformation->UserTime), KernelTime(processInformation->KernelTime), PeakVirtualSize(processInformation->PeakVirtualSize), VirtualSize(processInformation->VirtualSize), PeakWorkingSetSize(processInformation->PeakWorkingSetSize), WorkingSetSize(processInformation->WorkingSetSize), PagefileUsage(processInformation->PagefileUsage), PeakPagefileUsage(processInformation->PeakPagefileUsage), PrivatePageCount(processInformation->PrivatePageCount) {
    //
}

void ProcessInfo::update(PFULL_SYSTEM_PROCESS_INFORMATION processInformation) {
    UserTime = processInformation->UserTime;
    KernelTime = processInformation->KernelTime;

    PeakVirtualSize = processInformation->PeakVirtualSize;
    VirtualSize = processInformation->VirtualSize;
    PeakWorkingSetSize = processInformation->PeakWorkingSetSize;
    WorkingSetSize = processInformation->WorkingSetSize;
    PagefileUsage = processInformation->PagefileUsage;
    PeakPagefileUsage = processInformation->PeakPagefileUsage;
    PrivatePageCount = processInformation->PrivatePageCount;
}

void ProcessInfo::updateGpuData() {
    GpuData.commitMemory = 0;
    GpuData.dedicatedMemory = 0;
    GpuData.sharedMemory = 0;
    GpuData.utilization = 0.0;
}

void ProcessInfo::updateGpuData(GpuInfo const& gpuInfo) {
    GpuData = gpuInfo;
}

QString DeltaValueLI::toQString() {
    return QStringLiteral("%1;%2").arg(value.QuadPart).arg(delta);
}

QString DeltaValueST::toQString() {
    return QStringLiteral("%1;%2").arg(value).arg(delta);
}

QString ProcessInfo::toQString() {
    return QStringLiteral("%1;%2;%3;%4;%5;%6;%7;%8").arg((uint64_t)UniqueProcessId).arg(ImageName).arg(UserTime.toQString()).arg(KernelTime.toQString()).arg(WorkingSetSize.toQString()).arg(PeakWorkingSetSize.toQString()).arg(GpuData.utilization, 0, 'f', 3).arg(GpuData.dedicatedMemory);
}

std::ostream& operator<<(std::ostream& os, const DeltaValueLI& d) {
    os << d.value.QuadPart << "(d = " << d.delta << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DeltaValueST& d) {
    os << d.value << "(d = " << d.delta << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ProcessInfo& pi) {
    os << pi.ImageName.toStdString() << "(PID = " << (uint64_t)pi.UniqueProcessId;
    os << ", UserTime = " << pi.UserTime;
    os << ", KernelTime = " << pi.KernelTime;
    //os << ", PeakVirtualSize = " << pi.PeakVirtualSize;
    //os << ", VirtualSize = " << pi.VirtualSize;
    os << ", PeakWorkingSetSize = " << pi.PeakWorkingSetSize;
    os << ", WorkingSetSize = " << pi.WorkingSetSize;
    //os << ", PagefileUsage = " << pi.PagefileUsage;
    //os << ", PeakPagefileUsage = " << pi.PeakPagefileUsage;
    //os << ", PrivatePageCount = " << pi.PrivatePageCount;
    os << ")";
    return os;
}
