// CMesh::Load file-path bridge (#45).

#include "rs2_cmesh_xfile.h"

bool rs2_cmesh_probe_xfile(const char *path, Rs2XMesh *out, std::string *err) {
	return rs2_xfile_parse_file(path, out, err);
}
