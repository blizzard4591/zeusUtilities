#ifndef PRESENT_MODE_H
#define PRESENT_MODE_H

enum class PresentMode {
    Unknown,
    Hardware_Legacy_Flip,
    Hardware_Legacy_Copy_To_Front_Buffer,
    /* Not detected:
    Hardware_Direct_Flip,
    */
    Hardware_Independent_Flip,
    Composed_Flip,
    Composed_Copy_GPU_GDI,
    Composed_Copy_CPU_GDI,
    Composed_Composition_Atlas,
    Hardware_Composed_Independent_Flip,
};

#endif
