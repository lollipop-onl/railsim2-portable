// D3DX8 math self-test (#204): known values from the D3DX8 formulas, left-handed,
// row vector times matrix.

#include <d3dx8.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {

const float kEps = 1e-5f;
const float kHalfPi = D3DX_PI / 2;

int g_failures = 0;

void expect(bool ok, const char *spec) {
	if (!ok) {
		std::fprintf(stderr, "self-test: %s\n", spec);
		++g_failures;
	}
}

bool near(float a, float b, float eps = kEps) { return std::fabs(a - b) <= eps; }

bool near(const D3DXVECTOR3 &a, const D3DXVECTOR3 &b) {
	return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}

bool near(const D3DXMATRIX &a, const D3DXMATRIX &b, float eps = kEps) {
	const float *pa = &a._11, *pb = &b._11;
	for (int i = 0; i < 16; ++i)
		if (!near(pa[i], pb[i], eps)) return false;
	return true;
}

bool near(const D3DXQUATERNION &a, const D3DXQUATERNION &b) {
	return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z) && near(a.w, b.w);
}

D3DXVECTOR3 coord(const D3DXVECTOR3 &v, const D3DXMATRIX &m) {
	D3DXVECTOR3 out;
	D3DXVec3TransformCoord(&out, &v, &m);
	return out;
}

void rotations() {
	D3DXMATRIX m;
	D3DXMatrixRotationX(&m, kHalfPi);
	expect(near(coord(D3DXVECTOR3(0, 1, 0), m), D3DXVECTOR3(0, 0, 1)), "RotationX(90deg) turns +y into +z");
	D3DXMatrixRotationY(&m, kHalfPi);
	expect(near(coord(D3DXVECTOR3(0, 0, 1), m), D3DXVECTOR3(1, 0, 0)), "RotationY(90deg) turns +z into +x");
	D3DXMatrixRotationZ(&m, kHalfPi);
	expect(near(coord(D3DXVECTOR3(1, 0, 0), m), D3DXVECTOR3(0, 1, 0)), "RotationZ(90deg) turns +x into +y");

	D3DXMATRIX axis, ref;
	const D3DXVECTOR3 long_x(2, 0, 0);
	D3DXMatrixRotationAxis(&axis, &long_x, 0.7f);
	D3DXMatrixRotationX(&ref, 0.7f);
	expect(near(axis, ref), "RotationAxis normalizes its axis and matches RotationX about +x");
	const D3DXVECTOR3 diag(1, 1, 1);
	D3DXMatrixRotationAxis(&axis, &diag, 2 * D3DX_PI / 3);
	expect(near(coord(D3DXVECTOR3(1, 0, 0), axis), D3DXVECTOR3(0, 1, 0)),
	       "RotationAxis(1,1,1 by 120deg) cycles +x into +y");

	D3DXMATRIX ypr, rx, ry, rz;
	D3DXMatrixRotationYawPitchRoll(&ypr, 0.3f, -0.4f, 1.1f);
	D3DXMatrixRotationZ(&rz, 1.1f);
	D3DXMatrixRotationX(&rx, -0.4f);
	D3DXMatrixRotationY(&ry, 0.3f);
	expect(near(ypr, rz * rx * ry), "YawPitchRoll applies roll(Z), then pitch(X), then yaw(Y)");

	const float half = 0.9f / 2;
	const D3DXQUATERNION qy(0, std::sin(half), 0, std::cos(half));
	D3DXMatrixRotationQuaternion(&m, &qy);
	D3DXMatrixRotationY(&ref, 0.9f);
	expect(near(m, ref), "RotationQuaternion of (sin(a/2) * +y, cos(a/2)) equals RotationY(a)");
}

void inverse() {
	D3DXMATRIX s, r, t, inv;
	D3DXMatrixScaling(&s, 2, 3, 4);
	D3DXMatrixRotationYawPitchRoll(&r, 0.5f, 0.2f, -0.3f);
	D3DXMatrixTranslation(&t, 5, -6, 7);
	const D3DXMATRIX m = s * r * t;
	float det = 0;
	expect(D3DXMatrixInverse(&inv, &det, &m) == &inv, "Inverse of a regular matrix returns its output pointer");
	expect(near(det, 24, 1e-4f), "Inverse reports the determinant (scale 2*3*4, rotation and move keep it)");
	expect(near(m * inv, D3DXMATRIX(), 1e-4f), "a matrix times its Inverse is the identity");

	D3DXMATRIX swap(0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	expect(D3DXMatrixInverse(&inv, &det, &swap) == &inv && near(det, -1), "swapping two axes has determinant -1");
	expect(near(inv, swap), "swapping two axes is its own Inverse");

	D3DXMATRIX singular(1, 2, 3, 4, 2, 4, 6, 8, 0, 0, 1, 0, 0, 0, 0, 1);
	D3DXMATRIX untouched;
	D3DXMatrixTranslation(&untouched, 9, 9, 9);
	D3DXMATRIX out = untouched;
	expect(D3DXMatrixInverse(&out, nullptr, &singular) == nullptr, "Inverse of a singular matrix returns NULL");
	expect(near(out, untouched), "Inverse of a singular matrix leaves the output alone");
}

void view_and_projection() {
	D3DXMATRIX view;
	const D3DXVECTOR3 eye(0, 0, -5), origin(0, 0, 0), up(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &eye, &origin, &up);
	expect(near(view, D3DXMATRIX(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 5, 1)),
	       "LookAtLH from -z toward the origin is a pure move by +5 along z");

	const D3DXVECTOR3 eye2(3, 4, 5), at2(3, 4, 12), up2(0, 1, 0);
	D3DXMatrixLookAtLH(&view, &eye2, &at2, &up2);
	expect(near(coord(eye2, view), D3DXVECTOR3(0, 0, 0)), "LookAtLH puts the eye at the view origin");
	expect(near(coord(at2, view), D3DXVECTOR3(0, 0, 7)), "LookAtLH puts the target on +z at its distance");
	const D3DXVECTOR3 side(4, 4, 12);
	expect(near(coord(side, view), D3DXVECTOR3(1, 0, 7)), "LookAtLH keeps +x to the right (left-handed)");

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, kHalfPi, 2, 1, 101);
	expect(near(proj, D3DXMATRIX(0.5f, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1.01f, 1, 0, 0, -1.01f, 0)),
	       "PerspectiveFovLH: yscale = cot(fovy/2), xscale = yscale/aspect, z row zf/(zf-zn), -zn*zf/(zf-zn)");
	expect(near(coord(D3DXVECTOR3(0, 0, 1), proj).z, 0), "PerspectiveFovLH maps the near plane to depth 0");
	expect(near(coord(D3DXVECTOR3(0, 0, 101), proj).z, 1), "PerspectiveFovLH maps the far plane to depth 1");

	D3DXMATRIX off;
	D3DXMatrixPerspectiveOffCenterLH(&off, -1, 1, -1, 1, 1, 101);
	D3DXMatrixPerspectiveFovLH(&proj, kHalfPi, 1, 1, 101);
	expect(near(off, proj), "PerspectiveOffCenterLH with a centred square volume equals PerspectiveFovLH(90deg, 1)");
	D3DXMatrixPerspectiveOffCenterLH(&off, 0, 2, 0, 4, 1, 101);
	expect(near(off._11, 1) && near(off._22, 0.5f) && near(off._31, -1) && near(off._32, -1),
	       "PerspectiveOffCenterLH shears by (l+r)/(l-r) and (t+b)/(b-t)");
	expect(near(coord(D3DXVECTOR3(2, 4, 1), off), D3DXVECTOR3(1, 1, 0)),
	       "PerspectiveOffCenterLH maps the near plane's right/top corner to (1, 1, 0)");
}

void transforms() {
	D3DXMATRIX m(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
	const D3DXVECTOR3 v = coord(D3DXVECTOR3(1, 1, 1), m);
	expect(!std::isfinite(v.x), "Vec3TransformCoord divides by w even when w is 0");

	D3DXMATRIX t;
	D3DXMatrixTranslation(&t, 5, 6, 7);
	D3DXVECTOR3 n;
	const D3DXVECTOR3 dir(1, 2, 3);
	D3DXVec3TransformNormal(&n, &dir, &t);
	expect(near(n, dir), "Vec3TransformNormal ignores the translation row");
}

void planes() {
	D3DXPLANE p;
	const D3DXVECTOR3 point(0, 2, 0), normal(0, 1, 0);
	D3DXPlaneFromPointNormal(&p, &point, &normal);
	expect(near(p.a, 0) && near(p.b, 1) && near(p.c, 0) && near(p.d, -2), "PlaneFromPointNormal: d = -dot(point, normal)");
	const D3DXVECTOR3 above(5, 3, 7);
	expect(near(D3DXPlaneDotCoord(&p, &above), 1), "PlaneDotCoord is the signed distance for a unit normal");

	const D3DXVECTOR3 v1(0, 0, 0), v2(0, 0, 2), v3(3, 0, 0);
	D3DXPlaneFromPoints(&p, &v1, &v2, &v3);
	expect(near(p.a, 0) && near(p.b, 1) && near(p.c, 0) && near(p.d, 0),
	       "PlaneFromPoints: normal is normalize(cross(v2-v1, v3-v1)), through v1");

	D3DXPLANE scaled{0, 3, 0, -6}, unit;
	D3DXPlaneNormalize(&unit, &scaled);
	expect(near(unit.b, 1) && near(unit.d, -2), "PlaneNormalize scales d with the normal");
}

void shadow() {
	D3DXPLANE ground;
	const D3DXVECTOR3 origin(0, 0, 0), up(0, 1, 0);
	D3DXPlaneFromPointNormal(&ground, &origin, &up);

	D3DXMATRIX m;
	const D3DXVECTOR4 overhead(0, 1, 0, 0);
	D3DXMatrixShadow(&m, &overhead, &ground);
	expect(near(coord(D3DXVECTOR3(3, 5, 7), m), D3DXVECTOR3(3, 0, 7)), "Shadow of a light straight above drops y");
	expect(near(m._44, 1), "Shadow of a directional light keeps w positive");

	const D3DXVECTOR4 slant(1, 1, 0, 0);
	D3DXMatrixShadow(&m, &slant, &ground);
	expect(near(coord(D3DXVECTOR3(3, 5, 7), m), D3DXVECTOR3(-2, 0, 7)),
	       "Shadow of a slanted directional light slides away from the light");

	const D3DXVECTOR4 lamp(0, 10, 0, 1);
	D3DXMatrixShadow(&m, &lamp, &ground);
	expect(near(coord(D3DXVECTOR3(1, 5, 0), m), D3DXVECTOR3(2, 0, 0)),
	       "Shadow of a point light projects along the line from the light");

	D3DXPLANE tall{0, 5, 0, 0};
	D3DXMATRIX from_tall;
	D3DXMatrixShadow(&from_tall, &lamp, &tall);
	D3DXMatrixShadow(&m, &lamp, &ground);
	expect(near(from_tall, m), "Shadow normalizes the plane first");
}

void slerp() {
	const float s45 = std::sin(D3DX_PI / 4), c45 = std::cos(D3DX_PI / 4);
	const D3DXQUATERNION id(0, 0, 0, 1), z90(0, 0, s45, c45), z90_neg(0, 0, -s45, -c45);
	const D3DXQUATERNION z45(0, 0, std::sin(D3DX_PI / 8), std::cos(D3DX_PI / 8));
	D3DXQUATERNION q;
	D3DXQuaternionSlerp(&q, &id, &z90, 0);
	expect(near(q, id), "Slerp at t = 0 is the first quaternion");
	D3DXQuaternionSlerp(&q, &id, &z90, 1);
	expect(near(q, z90), "Slerp at t = 1 is the second quaternion");
	D3DXQuaternionSlerp(&q, &id, &z90, 0.5f);
	expect(near(q, z45), "Slerp halfway between 0 and 90deg about z is 45deg about z");
	D3DXQuaternionSlerp(&q, &id, &z90_neg, 0.5f);
	expect(near(q, z45), "Slerp takes the short way when the quaternions point apart");

	const D3DXQUATERNION a(0, 0, 0, 1), b(0, 0, 0.0001f, 1);
	D3DXQuaternionSlerp(&q, &a, &b, 0.5f);
	expect(near(q, D3DXQUATERNION(0, 0, 0.00005f, 1)), "Slerp of nearly equal quaternions blends linearly");
}

void fvf_size() {
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZ) == 12, "FVF XYZ is 12 bytes");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1) == 28,
	       "FVF XYZRHW|DIFFUSE|TEX1 is 28 bytes (colour counts 4 on every host)");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1) == 32, "FVF XYZ|NORMAL|TEX1 is 32 bytes");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2) == 44,
	       "FVF XYZ|NORMAL|DIFFUSE|TEX2 is 44 bytes");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR) == 20,
	       "FVF SPECULAR adds 4 bytes");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZB1 | D3DFVF_NORMAL) == 28, "FVF XYZB1 carries one blend weight");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZB5) == 32, "FVF XYZB5 carries five blend weights");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZ | D3DFVF_PSIZE) == 16, "FVF PSIZE adds 4 bytes");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZ | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE1(1)) == 28,
	       "FVF TEXCOORDSIZE sets each texture set's float count");
	expect(D3DXGetFVFVertexSize(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE4(0)) == 28,
	       "FVF TEXCOORDSIZE4 gives four floats");
}

void bounds() {
	struct Vtx {
		float x, y, z, nx, ny, nz;
	};
	const Vtx vertices[] = {
		{1, -2, 3, 100, 100, 100},
		{-4, 5, 0, -100, -100, -100},
		{2, 0, -6, 0, 0, 0},
	};
	D3DXVECTOR3 lo, hi;
	expect(D3DXComputeBoundingBox(vertices, 3, D3DFVF_XYZ | D3DFVF_NORMAL, &lo, &hi) == S_OK,
	       "ComputeBoundingBox succeeds on valid input");
	expect(near(lo, D3DXVECTOR3(-4, -2, -6)) && near(hi, D3DXVECTOR3(2, 5, 3)),
	       "ComputeBoundingBox strides by the FVF size and reads positions only");
	expect(D3DXComputeBoundingBox(nullptr, 3, D3DFVF_XYZ, &lo, &hi) == D3DERR_INVALIDCALL,
	       "ComputeBoundingBox rejects a null vertex pointer");

	const D3DXVECTOR3 bmin(-1, -1, -1), bmax(1, 1, 1);
	const D3DXVECTOR3 west(-5, 0, 0), east(1, 0, 0), wdir(-1, 0, 0), off_axis(-5, 5, 0);
	const D3DXVECTOR3 corner(-5, -5, -5), diagonal(1, 1, 1), inside(0.5f, 0, 0);
	expect(D3DXBoxBoundProbe(&bmin, &bmax, &west, &east), "BoxBoundProbe hits a box ahead of the ray");
	expect(!D3DXBoxBoundProbe(&bmin, &bmax, &west, &wdir), "BoxBoundProbe misses a box behind the ray");
	expect(!D3DXBoxBoundProbe(&bmin, &bmax, &off_axis, &east), "BoxBoundProbe misses a box beside the ray");
	expect(D3DXBoxBoundProbe(&bmin, &bmax, &corner, &diagonal), "BoxBoundProbe hits along a diagonal");
	expect(D3DXBoxBoundProbe(&bmin, &bmax, &inside, &wdir), "BoxBoundProbe hits when the ray starts inside");

	const D3DXVECTOR3 centre(0, 0, 0), beside(-5, 2, 0), tangent(-5, 1, 0);
	expect(D3DXSphereBoundProbe(&centre, 1, &west, &east), "SphereBoundProbe hits a sphere ahead of the ray");
	expect(!D3DXSphereBoundProbe(&centre, 1, &west, &wdir), "SphereBoundProbe misses a sphere behind the ray");
	expect(!D3DXSphereBoundProbe(&centre, 1, &beside, &east), "SphereBoundProbe misses a sphere beside the ray");
	expect(!D3DXSphereBoundProbe(&centre, 1, &tangent, &east), "SphereBoundProbe does not count a grazing ray");
	expect(D3DXSphereBoundProbe(&centre, 1, &centre, &east), "SphereBoundProbe hits when the ray starts inside");
}

int self_test() {
	rotations();
	inverse();
	view_and_projection();
	transforms();
	planes();
	shadow();
	slerp();
	fvf_size();
	bounds();
	if (g_failures == 0) std::puts("rs2_d3dx_math_self_test: ok");
	return g_failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc == 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: %s --self-test\n", argv[0]);
	return 2;
}
