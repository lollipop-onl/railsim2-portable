// ReadMapVector / SaveMapVector for rs2_roundtrip (#54).
// Vendored from CProfilePlugin.cpp (layout I/O only; no profile render).

#include "stdafx.h"

#include "port/rs2_float.h"

char *ReadMapVector(
	char *str,
	char *pref,
	vector<float> &mapv
){
	char *eee, *tmp;
	if(tmp = Assignment(str, pref)){
		str = tmp;
		do{
			if(mapv.size() && !(str = Character2(eee = str, ','))) throw CSynErr(eee);
			float texv;
			if(!(str = ConstFloat(eee = str, &texv))) throw CSynErr(eee);
			mapv.push_back(texv);
		} while(!(tmp = Character2(str, ';')));
		str = tmp;
	}
	return str;
}

void SaveMapVector(
	FILE *df,
	char *pref,
	vector<float> &mapv
){
	if(!mapv.size()) return;
	fprintf(df, pref);
	int i;
	for(i = 0; i<mapv.size(); i++) fprintf(df, i ? ", " RS2_FLOAT_FMT : RS2_FLOAT_FMT, mapv[i]);
	fprintf(df, ";\n");
}

// Two 32-bit pointer words packed in an 8-byte slot (DepartureTime on Win32).
char *rs2_asgn_pointer32_pair(
	char *str,
	char *read,
	void *dest8
) {
	uint32_t *dw = static_cast<uint32_t *>(dest8);
	char *eee;
	if (!(str = Assignment(eee = str, read))) return NULL;
	void *p = NULL;
	if (!(str = HexPointer(eee = str, &p))) return NULL;
	dw[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
	if (!(str = Character2(eee = str, ','))) return NULL;
	if (!(str = HexPointer(eee = str, &p))) return NULL;
	dw[1] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
	return Character2(eee = str, ';');
}
