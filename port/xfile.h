// Closed text X-File parser for Distribution assets (issue #28, parent #6).
// Does not replace CXFile / CMesh::Load.

#ifndef RS2_PORT_XFILE_H
#define RS2_PORT_XFILE_H

#include <string>
#include <vector>

struct Rs2XVec3 {
	float x;
	float y;
	float z;
};

struct Rs2XVec2 {
	float u;
	float v;
};

struct Rs2XColor {
	float r;
	float g;
	float b;
	float a;
};

struct Rs2XFace {
	std::vector<int> indices;
};

struct Rs2XMaterial {
	Rs2XColor diffuse;
	float power;
	Rs2XColor specular;
	Rs2XColor emissive;
	std::string texture;
};

struct Rs2XIndexedColor {
	int index;
	Rs2XColor color;
};

struct Rs2XMesh {
	std::string name;
	std::vector<Rs2XVec3> positions;
	std::vector<Rs2XFace> faces;
	std::vector<int> face_materials;
	std::vector<Rs2XMaterial> materials;
	std::vector<Rs2XVec3> normals;
	std::vector<Rs2XFace> normal_faces;
	std::vector<Rs2XVec2> uvs;
	std::vector<Rs2XIndexedColor> vertex_colors;
};

// Parse a whole file (including the 16-byte magic). On failure, *err is set.
bool rs2_xfile_parse(const std::string &bytes, Rs2XMesh *out, std::string *err);
bool rs2_xfile_parse_file(const char *path, Rs2XMesh *out, std::string *err);

#endif
