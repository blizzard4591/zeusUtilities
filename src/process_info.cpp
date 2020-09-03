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

QString DeltaValueLI::toQString() const {
    return QStringLiteral("%1;%2").arg(value.QuadPart).arg(delta);
}

QString DeltaValueST::toQString() const {
    return QStringLiteral("%1;%2").arg(value).arg(delta);
}

QString ProcessInfo::toQString() {
    return QStringLiteral("%1;%2;%3;%4;%5;%6;%7;%8").arg((uint64_t)UniqueProcessId).arg(ImageName).arg(UserTime.toQString()).arg(KernelTime.toQString()).arg(WorkingSetSize.toQString()).arg(PeakWorkingSetSize.toQString()).arg(GpuData.utilization, 0, 'f', 3).arg(GpuData.dedicatedMemory);
}

QJsonObject ProcessInfo::toJsonObject(bool beVerbose) const {
    QJsonObject result;
    if (beVerbose) {
        result.insert(QStringLiteral("pid"), QJsonValue((qint64)UniqueProcessId));
        result.insert(QStringLiteral("imageName"), QJsonValue(ImageName));
        result.insert(QStringLiteral("userTime"), QJsonValue(UserTime.toQString()));
        result.insert(QStringLiteral("kernelTime"), QJsonValue(KernelTime.toQString()));

        result.insert(QStringLiteral("workingSetSize"), QJsonValue(WorkingSetSize.toQString()));
        result.insert(QStringLiteral("peakWorkingSetSize"), QJsonValue(PeakWorkingSetSize.toQString()));

        result.insert(QStringLiteral("gpuUtil"), QJsonValue(GpuData.utilization));
        result.insert(QStringLiteral("gpuDedicatedMemory"), QJsonValue(static_cast<qint64>(GpuData.dedicatedMemory)));
    } else {
        result.insert(QStringLiteral("2:0"), QJsonValue((qint64)UniqueProcessId));
        result.insert(QStringLiteral("2:1"), QJsonValue(ImageName));
        result.insert(QStringLiteral("2:2"), QJsonValue(UserTime.toQString()));
        result.insert(QStringLiteral("2:3"), QJsonValue(KernelTime.toQString()));

        result.insert(QStringLiteral("2:4"), QJsonValue(WorkingSetSize.toQString()));
        result.insert(QStringLiteral("2:5"), QJsonValue(PeakWorkingSetSize.toQString()));

        result.insert(QStringLiteral("2:6"), QJsonValue(GpuData.utilization));
        result.insert(QStringLiteral("2:7"), QJsonValue(static_cast<qint64>(GpuData.dedicatedMemory)));
    }
    return result;
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
