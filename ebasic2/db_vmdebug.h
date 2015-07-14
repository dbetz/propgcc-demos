#ifndef __DB_VMDEBUG_H__
#define __DB_VMDEBUG_H__

#include "db_types.h"

void DecodeFunction(VMUVALUE offset, const uint8_t *code, int len);
int DecodeInstruction(VMUVALUE base, const uint8_t *code, const uint8_t *lc);

#endif
