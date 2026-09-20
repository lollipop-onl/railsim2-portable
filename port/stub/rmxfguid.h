#pragma once

#include "windows.h"

// Zero, not the SDK's 3d82ab44-62da-11cf-ab39-0020af71e433: nothing in the
// tree ever reads this value. It is dereferenced only by CXFile::GetTopMesh,
// reached only from CMesh::Load(fRes=TRUE, ...), and every one of the 16
// fRes arguments in the tree is FALSE (15 CMeshList::Get calls plus
// lib/anim.cpp's direct CMesh::Load). The fRes=FALSE branch parses .x with
// port/xfile.cpp instead. Give this the real GUID before wiring up a
// resource-backed .x path: GetTopMesh breaks out only on a match and releases
// every other node, so a value that matches nothing runs the enumeration to
// its end and CMesh::Load reports "mesh is not found" for every resource .x.
static const GUID TID_D3DRMMesh = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
