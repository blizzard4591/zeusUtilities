#ifndef PH_CALLBACK_H
#define PH_CALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "phapp.h"

void phToZeusCallback(PPH_PROCESS_ITEM processItem, PPH_PROCESS_NODE processNode, ULONG id, PPH_STRINGREF stringRef);

#ifdef __cplusplus
}
#endif

#endif
