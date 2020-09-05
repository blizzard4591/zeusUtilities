#include "fps_info.h"

#include <QFile>
#include <QJsonDocument>

#define ASSIGN_SHOULD_CONTAIN(target, fullName, shortName) if (object.contains(fullName) || object.contains(shortName)) { target = (object.contains(fullName) ? object.value(fullName) : object.value(shortName)).toDouble(); } else ok = false

const QString FpsInfo::TAG_MSPERFRAME_L = QStringLiteral("msPerFrame");
const QString FpsInfo::TAG_MSPERFRAME_S = QStringLiteral("f:0");
const QString FpsInfo::TAG_FPS_L = QStringLiteral("fps");
const QString FpsInfo::TAG_FPS_S = QStringLiteral("f:1");
const QString FpsInfo::TAG_FPSDISPLAYED_L = QStringLiteral("fpsDisplayed");
const QString FpsInfo::TAG_FPSDISPLAYED_S = QStringLiteral("f:2");
const QString FpsInfo::TAG_LATENCY_L = QStringLiteral("latency");
const QString FpsInfo::TAG_LATENCY_S = QStringLiteral("f:3");
const QString FpsInfo::TAG_PRESENTMODE_L = QStringLiteral("presMode");
const QString FpsInfo::TAG_PRESENTMODE_S = QStringLiteral("f:4");

FpsInfo::FpsInfo() {
	reset();
}

void FpsInfo::reset() {
	msPerFrame = 0;
	framesPerSecond = 0;
	fpsDisplayed = 0;
	latency = 0;
	presentMode = PresentMode::Unknown;
}

QJsonObject FpsInfo::toJsonObject(bool verbose) const {
    QJsonObject result;
    if (verbose) {
        result.insert(TAG_MSPERFRAME_L, msPerFrame);
        result.insert(TAG_FPS_L, framesPerSecond);
        result.insert(TAG_FPSDISPLAYED_L, fpsDisplayed);
        result.insert(TAG_LATENCY_L, latency);
        result.insert(TAG_PRESENTMODE_L, PresentModeToInt(presentMode));
    } else {
        result.insert(TAG_MSPERFRAME_S, msPerFrame);
        result.insert(TAG_FPS_S, framesPerSecond);
        result.insert(TAG_FPSDISPLAYED_S, fpsDisplayed);
        result.insert(TAG_LATENCY_S, latency);
        result.insert(TAG_PRESENTMODE_S, PresentModeToInt(presentMode));
    }
    return result;
}

FpsInfo FpsInfo::fromJsonObject(QJsonObject const& object, bool* okay) {
	FpsInfo result;
    bool ok = true;

    ASSIGN_SHOULD_CONTAIN(result.msPerFrame, TAG_MSPERFRAME_L, TAG_MSPERFRAME_S);
    ASSIGN_SHOULD_CONTAIN(result.framesPerSecond, TAG_FPS_L, TAG_FPS_S);
    ASSIGN_SHOULD_CONTAIN(result.fpsDisplayed, TAG_FPSDISPLAYED_L, TAG_FPSDISPLAYED_S);
    ASSIGN_SHOULD_CONTAIN(result.latency, TAG_LATENCY_L, TAG_LATENCY_S);
    if (object.contains(TAG_PRESENTMODE_L) || object.contains(TAG_PRESENTMODE_S)) {
        result.presentMode = IntToPresentMode((object.contains(TAG_PRESENTMODE_L) ? object.value(TAG_PRESENTMODE_L) : object.value(TAG_PRESENTMODE_S)).toInt());
    } else {
        ok = false;
    }

	if (okay != nullptr) {
		*okay = ok;
	}

	return result;
}

std::ostream& operator<<(std::ostream& os, const FpsInfo& fi) {
	os << "ms per frame = " << fi.msPerFrame << ", FPS = " << fi.framesPerSecond << ", FPS displayed = " << fi.fpsDisplayed << ", latency = " << fi.latency << ", Presentation Mode = " << FpsInfo::PresentModeToString(fi.presentMode).toStdString();
	return os;
}

QString FpsInfo::PresentModeToString(PresentMode const& mode) {
    switch (mode) {
    case PresentMode::Hardware_Legacy_Flip: return QStringLiteral("Hardware: Legacy Flip");
    case PresentMode::Hardware_Legacy_Copy_To_Front_Buffer: return QStringLiteral("Hardware: Legacy Copy to front buffer");
    case PresentMode::Hardware_Independent_Flip: return QStringLiteral("Hardware: Independent Flip");
    case PresentMode::Composed_Flip: return QStringLiteral("Composed: Flip");
    case PresentMode::Composed_Copy_GPU_GDI: return QStringLiteral("Composed: Copy with GPU GDI");
    case PresentMode::Composed_Copy_CPU_GDI: return QStringLiteral("Composed: Copy with CPU GDI");
    case PresentMode::Composed_Composition_Atlas: return QStringLiteral("Composed: Composition Atlas");
    case PresentMode::Hardware_Composed_Independent_Flip: return QStringLiteral("Hardware Composed: Independent Flip");
    default: return QStringLiteral("Other");
    }
}

int FpsInfo::PresentModeToInt(PresentMode const& presentMode) {
    switch (presentMode) {
    case PresentMode::Unknown:
        return 0;
    case PresentMode::Hardware_Legacy_Flip:
        return 1;
    case PresentMode::Hardware_Legacy_Copy_To_Front_Buffer:
        return 2;
            /* Not detected:
            Hardware_Direct_Flip,
            */
    case PresentMode::Hardware_Independent_Flip:
        return 3;
    case PresentMode::Composed_Flip:
        return 4;
    case PresentMode::Composed_Copy_GPU_GDI:
        return 5;
    case PresentMode::Composed_Copy_CPU_GDI:
        return 6;
    case PresentMode::Composed_Composition_Atlas:
        return 7;
    case PresentMode::Hardware_Composed_Independent_Flip:
        return 8;
    default:
        return -1;
    }
}

PresentMode FpsInfo::IntToPresentMode(int presentMode) {
    switch (presentMode) {
    case 0:
        return PresentMode::Unknown;
    case 1:
        return PresentMode::Hardware_Legacy_Flip;
    case 2:
        return PresentMode::Hardware_Legacy_Copy_To_Front_Buffer;
    case 3:
        return PresentMode::Hardware_Independent_Flip;
    case 4:
        return PresentMode::Composed_Flip;
    case 5:
        return PresentMode::Composed_Copy_GPU_GDI;
    case 6:
        return PresentMode::Composed_Copy_CPU_GDI;
    case 7:
        return PresentMode::Composed_Composition_Atlas;
    case 8:
        return PresentMode::Hardware_Composed_Independent_Flip;
    default:
        return PresentMode::Unknown;
    }
}
