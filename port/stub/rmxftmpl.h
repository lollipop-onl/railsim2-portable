#pragma once

// Empty, not the SDK's ~3.2 KB of compiled .x templates: CXFile::Open hands
// this to RegisterTemplates on the resource path only, and no caller reaches
// it -- all 16 fRes arguments in the tree are FALSE, so CMesh::Load always
// takes the port/xfile.cpp branch. The byte count is 0 rather than
// sizeof(D3DRM_XTEMPLATES) so that registering the placeholder cannot look
// like it succeeded. Transcribing the real table is a prerequisite for
// reading resource .x files, not busywork left undone.
static const unsigned char D3DRM_XTEMPLATES[1] = {0};
#define D3DRM_XTEMPLATE_BYTES 0L
