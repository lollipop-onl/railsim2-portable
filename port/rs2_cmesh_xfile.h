// CMesh::Load file-path bridge to closed text X-File parser (#45, parent #6).

#pragma once

#include "xfile.h"

#include <string>

// Parse path via rs2_xfile_parse_file (CMesh::Load !fRes branch under RS2_PORTABLE_COMPILE_FIREWALL).
bool rs2_cmesh_probe_xfile(const char *path, Rs2XMesh *out, std::string *err);
