#include "process_info.h"

#define ASSIGN_IF_CONTAINED(target, fullName, shortName) if (object.contains(fullName) || object.contains(shortName)) target = DeltaValueST::fromJsonObject((object.contains(fullName) ? object.value(fullName) : object.value(shortName)).toObject(), &subOk); ok &= subOk
#define ASSIGN_SHOULD_CONTAIN(target, fullName, shortName) if (object.contains(fullName) || object.contains(shortName)) { target = (object.contains(fullName) ? object.value(fullName) : object.value(shortName)).toDouble(); } else ok = false
#define ASSIGN_SHOULD_CONTAIN_FULL(target, fullName) if (object.contains(fullName)) { target = object.value(fullName).toDouble(); } else ok = false

#define ASSIGN_SHOULD_CONTAIN_LI(target, fullName, shortName) if (object.contains(fullName) || object.contains(shortName)) { target = DeltaValueLI::fromJsonObject((object.contains(fullName) ? object.value(fullName) : object.value(shortName)).toObject(), &subOk); ok &= subOk; } else ok = false

const QString DeltaValueLI::TAG_V = QStringLiteral("iV");
const QString DeltaValueLI::TAG_D = QStringLiteral("iD");

const QString DeltaValueST::TAG_V = QStringLiteral("sV");
const QString DeltaValueST::TAG_D = QStringLiteral("sD");

const QString ProcessInfo::TAG_PID_L = QStringLiteral("pid");
const QString ProcessInfo::TAG_PID_S = QStringLiteral("2:0");
const QString ProcessInfo::TAG_IMAGENAME_L = QStringLiteral("imageName");
const QString ProcessInfo::TAG_IMAGENAME_S = QStringLiteral("2:1");

const QString ProcessInfo::TAG_USERTIME_L = QStringLiteral("userTime");
const QString ProcessInfo::TAG_USERTIME_S = QStringLiteral("2:2");
const QString ProcessInfo::TAG_KERNELTIME_L = QStringLiteral("kernelTime");
const QString ProcessInfo::TAG_KERNELTIME_S = QStringLiteral("2:3");

const QString ProcessInfo::TAG_PEAKVIRTSIZE_L = QStringLiteral("peakVirtualSize");
const QString ProcessInfo::TAG_PEAKVIRTSIZE_S = QStringLiteral("2:4");
const QString ProcessInfo::TAG_VIRTSIZE_L = QStringLiteral("virtualSize");
const QString ProcessInfo::TAG_VIRTSIZE_S = QStringLiteral("2:5");
const QString ProcessInfo::TAG_PEAKWORKINGSIZE_L = QStringLiteral("peakWorkingSetSize");
const QString ProcessInfo::TAG_PEAKWORKINGSIZE_S = QStringLiteral("2:6");
const QString ProcessInfo::TAG_WORKINGSIZE_L = QStringLiteral("workingSetSize");
const QString ProcessInfo::TAG_WORKINGSIZE_S = QStringLiteral("2:7");
const QString ProcessInfo::TAG_PAGEFILEUSAGE_L = QStringLiteral("pageFileUse");
const QString ProcessInfo::TAG_PAGEFILEUSAGE_S = QStringLiteral("2:8");
const QString ProcessInfo::TAG_PEAKPAGEFILEUSAGE_L = QStringLiteral("peakPageFileUse");
const QString ProcessInfo::TAG_PEAKPAGEFILEUSAGE_S = QStringLiteral("2:9");
const QString ProcessInfo::TAG_PRIVATEPAGECOUNT_L = QStringLiteral("privatePageCount");
const QString ProcessInfo::TAG_PRIVATEPAGECOUNT_S = QStringLiteral("2:A");

const QString ProcessInfo::TAG_GPUDATA_L = QStringLiteral("gpu");
const QString ProcessInfo::TAG_GPUDATA_S = QStringLiteral("2:B");

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

QJsonObject DeltaValueLI::toJsonObject(bool beVerbose) const {
    QJsonObject result;
    result.insert(TAG_V, QJsonValue(value.QuadPart));
    result.insert(TAG_D, QJsonValue(delta));
    return result;
}

DeltaValueLI DeltaValueLI::fromJsonObject(QJsonObject const& object, bool* okay) {
    bool ok = true;
    bool subOk = false;
    DeltaValueLI result;

    ASSIGN_SHOULD_CONTAIN_FULL(result.value.QuadPart, TAG_V);
    ASSIGN_SHOULD_CONTAIN_FULL(result.delta, TAG_D);
    if (okay != nullptr) {
        *okay = ok;
    }

    return result;
}

QString DeltaValueLI::toQString() const {
    return QStringLiteral("%1;%2").arg(value.QuadPart).arg(delta);
}

QJsonObject DeltaValueST::toJsonObject(bool beVerbose) const {
    QJsonObject result;
    result.insert(TAG_V, QJsonValue(static_cast<qint64>(value)));
    result.insert(TAG_D, QJsonValue(delta));
    return result;
}

DeltaValueST DeltaValueST::fromJsonObject(QJsonObject const& object, bool* okay) {
    bool ok = true;
    bool subOk = false;
    DeltaValueST result;
    ASSIGN_SHOULD_CONTAIN_FULL(result.value, TAG_V);
    ASSIGN_SHOULD_CONTAIN_FULL(result.delta, TAG_D);
    if (okay != nullptr) {
        *okay = ok;
    }

    return result;
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
        result.insert(TAG_PID_L, QJsonValue((qint64)UniqueProcessId));
        result.insert(TAG_IMAGENAME_L, QJsonValue(ImageName));
        result.insert(TAG_USERTIME_L, QJsonValue(UserTime.toJsonObject(beVerbose)));
        result.insert(TAG_KERNELTIME_L, QJsonValue(KernelTime.toJsonObject(beVerbose)));

        result.insert(TAG_WORKINGSIZE_L, QJsonValue(WorkingSetSize.toJsonObject(beVerbose)));
        result.insert(TAG_PEAKWORKINGSIZE_L, QJsonValue(PeakWorkingSetSize.toJsonObject(beVerbose)));
        result.insert(TAG_PAGEFILEUSAGE_L, QJsonValue(PagefileUsage.toJsonObject(beVerbose)));

        result.insert(TAG_GPUDATA_L, QJsonValue(GpuData.toJsonObject(beVerbose)));
    } else {
        result.insert(TAG_PID_S, QJsonValue((qint64)UniqueProcessId));
        result.insert(TAG_IMAGENAME_S, QJsonValue(ImageName));
        result.insert(TAG_USERTIME_S, QJsonValue(UserTime.toJsonObject(beVerbose)));
        result.insert(TAG_KERNELTIME_S, QJsonValue(KernelTime.toJsonObject(beVerbose)));

        result.insert(TAG_WORKINGSIZE_S, QJsonValue(WorkingSetSize.toJsonObject(beVerbose)));
        result.insert(TAG_PEAKWORKINGSIZE_S, QJsonValue(PeakWorkingSetSize.toJsonObject(beVerbose)));
        result.insert(TAG_PAGEFILEUSAGE_S, QJsonValue(PagefileUsage.toJsonObject(beVerbose)));

        result.insert(TAG_GPUDATA_S, QJsonValue(GpuData.toJsonObject(beVerbose)));
    }
    return result;
}

ProcessInfo ProcessInfo::fromJsonObject(QJsonObject const& object, bool* okay) {
    bool ok = true;
    bool subOk = false;

    quint64 pid = 0;
    ASSIGN_SHOULD_CONTAIN(pid, TAG_PID_L, TAG_PID_S);
    QString imageName;
    ASSIGN_SHOULD_CONTAIN(imageName, TAG_IMAGENAME_L, TAG_IMAGENAME_S);

    ProcessInfo result(imageName, (HANDLE)pid);
    
    ASSIGN_SHOULD_CONTAIN_LI(result.UserTime, TAG_USERTIME_L, TAG_USERTIME_S);
    ASSIGN_SHOULD_CONTAIN_LI(result.KernelTime, TAG_KERNELTIME_L, TAG_KERNELTIME_S);

    ASSIGN_IF_CONTAINED(result.PeakVirtualSize, TAG_PEAKVIRTSIZE_L, TAG_PEAKVIRTSIZE_S);
    ASSIGN_IF_CONTAINED(result.VirtualSize, TAG_VIRTSIZE_L, TAG_VIRTSIZE_S);
    ASSIGN_IF_CONTAINED(result.PeakWorkingSetSize, TAG_PEAKWORKINGSIZE_L, TAG_PEAKWORKINGSIZE_S);
    ASSIGN_IF_CONTAINED(result.WorkingSetSize, TAG_WORKINGSIZE_L, TAG_WORKINGSIZE_S);
    ASSIGN_IF_CONTAINED(result.PagefileUsage, TAG_PAGEFILEUSAGE_L, TAG_PAGEFILEUSAGE_S);
    ASSIGN_IF_CONTAINED(result.PeakPagefileUsage, TAG_PEAKPAGEFILEUSAGE_L, TAG_PEAKPAGEFILEUSAGE_S);
    ASSIGN_IF_CONTAINED(result.PrivatePageCount, TAG_PRIVATEPAGECOUNT_L, TAG_PRIVATEPAGECOUNT_S);

    if (okay != nullptr) {
        *okay = ok;
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
