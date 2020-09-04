#include "fps_info.h"

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
        result.insert(QStringLiteral("msPerFrame"), msPerFrame);
        result.insert(QStringLiteral("fps"), framesPerSecond);
        result.insert(QStringLiteral("fpsDisplayed"), fpsDisplayed);
        result.insert(QStringLiteral("latency"), latency);
        result.insert(QStringLiteral("presMode"), PresentModeToInt(presentMode));
    } else {
        result.insert(QStringLiteral("f:0"), msPerFrame);
        result.insert(QStringLiteral("f:1"), framesPerSecond);
        result.insert(QStringLiteral("f:2"), fpsDisplayed);
        result.insert(QStringLiteral("f:3"), latency);
        result.insert(QStringLiteral("f:4"), PresentModeToInt(presentMode));
    }
    return result;
}

FpsInfo FpsInfo::fromJsonObject(QJsonObject const& object, bool* okay) {
	FpsInfo result;
	bool ok = true;
	bool subOk = false;

	result.msPerFrame = (object.contains(QStringLiteral("msPerFrame")) ? object.value(QStringLiteral("msPerFrame")) : object.value(QStringLiteral("f:0"))).toString().toLongLong(&subOk); ok &= subOk;
	result.framesPerSecond = (object.contains(QStringLiteral("fps")) ? object.value(QStringLiteral("fps")) : object.value(QStringLiteral("f:1"))).toString().toLongLong(&subOk); ok &= subOk;
	result.fpsDisplayed = (object.contains(QStringLiteral("fpsDisplayed")) ? object.value(QStringLiteral("fpsDisplayed")) : object.value(QStringLiteral("f:2"))).toString().toLongLong(&subOk); ok &= subOk;
	result.latency = (object.contains(QStringLiteral("latency")) ? object.value(QStringLiteral("latency")) : object.value(QStringLiteral("f:3"))).toString().toLongLong(&subOk); ok &= subOk;
    result.presentMode = IntToPresentMode((object.contains(QStringLiteral("presMode")) ? object.value(QStringLiteral("presMode")) : object.value(QStringLiteral("f:4"))).toInt());

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
