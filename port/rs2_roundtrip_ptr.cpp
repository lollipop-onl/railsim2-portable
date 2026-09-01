// RS2_ROUNDTRIP: serialize g_AddressMap keys (file IDs), not heap pointers.

#include "stdafx.h"

#include "CSaveFile.h"
#include "port/rs2_ptr.h"

unsigned rs2_ptr32_serial(const void *p) {
	if (!p) return 0u;
	for (map<void *, void *>::const_iterator it = g_AddressMap.begin();
	     it != g_AddressMap.end(); ++it) {
		if (it->second == p) {
			return static_cast<unsigned>(
				reinterpret_cast<uintptr_t>(it->first));
		}
	}
	return static_cast<unsigned>(reinterpret_cast<uintptr_t>(p));
}
