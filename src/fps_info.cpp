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

QString FpsInfo::toQString() {
	return QStringLiteral("%1;%2;%3;%4").arg(msPerFrame, 0, 'f', 2).arg(framesPerSecond, 0, 'f', 2).arg(fpsDisplayed, 0, 'f', 2).arg(latency, 0, 'f', 2);
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
