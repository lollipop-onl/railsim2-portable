// D3DX8 matrix / plane / quaternion / bound math the game calls (#204, parent #5).
// Conventions: docs/porting/d3dx-math.md.

#include <d3dx8.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>

static_assert(sizeof(D3DXMATRIX) == 16 * sizeof(float), "D3DXMATRIX is 16 packed floats");

D3DXMATRIX *D3DXMatrixRotationX(D3DXMATRIX *out, FLOAT angle) {
	const float c = std::cos(angle), s = std::sin(angle);
	*out = D3DXMATRIX(1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1);
	return out;
}

D3DXMATRIX *D3DXMatrixRotationY(D3DXMATRIX *out, FLOAT angle) {
	const float c = std::cos(angle), s = std::sin(angle);
	*out = D3DXMATRIX(c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 0, 0, 0, 1);
	return out;
}

D3DXMATRIX *D3DXMatrixRotationZ(D3DXMATRIX *out, FLOAT angle) {
	const float c = std::cos(angle), s = std::sin(angle);
	*out = D3DXMATRIX(c, s, 0, 0, -s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	return out;
}

D3DXMATRIX *D3DXMatrixRotationAxis(D3DXMATRIX *out, const D3DXVECTOR3 *axis, FLOAT angle) {
	D3DXVECTOR3 n;
	D3DXVec3Normalize(&n, axis);
	const float c = std::cos(angle), s = std::sin(angle), k = 1.0f - c;
	*out = D3DXMATRIX(
		k * n.x * n.x + c, k * n.x * n.y + s * n.z, k * n.x * n.z - s * n.y, 0,
		k * n.y * n.x - s * n.z, k * n.y * n.y + c, k * n.y * n.z + s * n.x, 0,
		k * n.z * n.x + s * n.y, k * n.z * n.y - s * n.x, k * n.z * n.z + c, 0,
		0, 0, 0, 1);
	return out;
}

D3DXMATRIX *D3DXMatrixRotationYawPitchRoll(D3DXMATRIX *out, FLOAT yaw, FLOAT pitch, FLOAT roll) {
	const float sy = std::sin(yaw), cy = std::cos(yaw);
	const float sp = std::sin(pitch), cp = std::cos(pitch);
	const float sr = std::sin(roll), cr = std::cos(roll);
	*out = D3DXMATRIX(
		cr * cy + sr * sp * sy, sr * cp, sr * sp * cy - cr * sy, 0,
		cr * sp * sy - sr * cy, cr * cp, sr * sy + cr * sp * cy, 0,
		cp * sy, -sp, cp * cy, 0,
		0, 0, 0, 1);
	return out;
}

D3DXMATRIX *D3DXMatrixRotationQuaternion(D3DXMATRIX *out, const D3DXQUATERNION *q) {
	const float x = q->x, y = q->y, z = q->z, w = q->w;
	*out = D3DXMATRIX(
		1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w), 0,
		2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w), 0,
		2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y), 0,
		0, 0, 0, 1);
	return out;
}

D3DXMATRIX *D3DXMatrixInverse(D3DXMATRIX *out, FLOAT *determinant, const D3DXMATRIX *m) {
	float a[4][4], inv[4][4];
	std::memcpy(a, m, sizeof(a));
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) inv[i][j] = (i == j) ? 1.0f : 0.0f;

	float det = 1.0f;
	for (int col = 0; col < 4; ++col) {
		int pivot = col;
		for (int row = col + 1; row < 4; ++row)
			if (std::fabs(a[row][col]) > std::fabs(a[pivot][col])) pivot = row;
		if (pivot != col) {
			std::swap(a[pivot], a[col]);
			std::swap(inv[pivot], inv[col]);
			det = -det;
		}
		const float p = a[col][col];
		det *= p;
		if (p == 0.0f) break;
		for (int j = 0; j < 4; ++j) {
			a[col][j] /= p;
			inv[col][j] /= p;
		}
		for (int row = 0; row < 4; ++row) {
			if (row == col) continue;
			const float f = a[row][col];
			for (int j = 0; j < 4; ++j) {
				a[row][j] -= f * a[col][j];
				inv[row][j] -= f * inv[col][j];
			}
		}
	}

	if (determinant) *determinant = det;
	if (det == 0.0f) return nullptr;
	std::memcpy(out, inv, sizeof(inv));
	return out;
}

D3DXMATRIX *D3DXMatrixLookAtLH(D3DXMATRIX *out, const D3DXVECTOR3 *eye, const D3DXVECTOR3 *at,
                               const D3DXVECTOR3 *up) {
	D3DXVECTOR3 z = *at - *eye, x, y;
	D3DXVec3Normalize(&z, &z);
	D3DXVec3Cross(&x, up, &z);
	D3DXVec3Normalize(&x, &x);
	D3DXVec3Cross(&y, &z, &x);
	*out = D3DXMATRIX(
		x.x, y.x, z.x, 0,
		x.y, y.y, z.y, 0,
		x.z, y.z, z.z, 0,
		-D3DXVec3Dot(&x, eye), -D3DXVec3Dot(&y, eye), -D3DXVec3Dot(&z, eye), 1);
	return out;
}

D3DXMATRIX *D3DXMatrixPerspectiveFovLH(D3DXMATRIX *out, FLOAT fovy, FLOAT aspect, FLOAT zn, FLOAT zf) {
	const float ys = 1.0f / std::tan(fovy / 2), xs = ys / aspect;
	const float q = zf / (zf - zn);
	*out = D3DXMATRIX(xs, 0, 0, 0, 0, ys, 0, 0, 0, 0, q, 1, 0, 0, -zn * q, 0);
	return out;
}

D3DXMATRIX *D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX *out, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
                                             FLOAT zf) {
	const float q = zf / (zf - zn);
	*out = D3DXMATRIX(
		2 * zn / (r - l), 0, 0, 0,
		0, 2 * zn / (t - b), 0, 0,
		(l + r) / (l - r), (t + b) / (b - t), q, 1,
		0, 0, -zn * q, 0);
	return out;
}

D3DXMATRIX *D3DXMatrixShadow(D3DXMATRIX *out, const D3DXVECTOR4 *light, const D3DXPLANE *plane) {
	D3DXPLANE p;
	D3DXPlaneNormalize(&p, plane);
	const float d = p.a * light->x + p.b * light->y + p.c * light->z + p.d * light->w;
	const float pl[4] = {p.a, p.b, p.c, p.d};
	const float l[4] = {light->x, light->y, light->z, light->w};
	float m[4][4];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j) m[i][j] = (i == j ? d : 0.0f) - pl[i] * l[j];
	std::memcpy(out, m, sizeof(m));
	return out;
}

FLOAT D3DXPlaneDotCoord(const D3DXPLANE *p, const D3DXVECTOR3 *v) {
	return p->a * v->x + p->b * v->y + p->c * v->z + p->d;
}

D3DXPLANE *D3DXPlaneNormalize(D3DXPLANE *out, const D3DXPLANE *p) {
	const float len = std::sqrt(p->a * p->a + p->b * p->b + p->c * p->c);
	if (len > 0)
		*out = D3DXPLANE{p->a / len, p->b / len, p->c / len, p->d / len};
	else
		*out = D3DXPLANE{0, 0, 0, 0};
	return out;
}

D3DXPLANE *D3DXPlaneFromPointNormal(D3DXPLANE *out, const D3DXVECTOR3 *point, const D3DXVECTOR3 *normal) {
	*out = D3DXPLANE{normal->x, normal->y, normal->z, -D3DXVec3Dot(point, normal)};
	return out;
}

D3DXPLANE *D3DXPlaneFromPoints(D3DXPLANE *out, const D3DXVECTOR3 *v1, const D3DXVECTOR3 *v2,
                               const D3DXVECTOR3 *v3) {
	const D3DXVECTOR3 e1 = *v2 - *v1, e2 = *v3 - *v1;
	D3DXVECTOR3 n;
	D3DXVec3Cross(&n, &e1, &e2);
	D3DXVec3Normalize(&n, &n);
	return D3DXPlaneFromPointNormal(out, v1, &n);
}

D3DXQUATERNION *D3DXQuaternionSlerp(D3DXQUATERNION *out, const D3DXQUATERNION *q1, const D3DXQUATERNION *q2,
                                    FLOAT t) {
	float dot = q1->x * q2->x + q1->y * q2->y + q1->z * q2->z + q1->w * q2->w;
	float sign = 1.0f;
	if (dot < 0.0f) {
		sign = -1.0f;
		dot = -dot;
	}
	float k1 = 1.0f - t, k2 = t;
	// Near-parallel inputs fall back to lerp: sin(theta) underflows toward 0
	// and the ratio below would amplify rounding into garbage.
	if (1.0f - dot > 0.001f) {
		const float theta = std::acos(dot), st = std::sin(theta);
		k1 = std::sin(theta * k1) / st;
		k2 = std::sin(theta * k2) / st;
	}
	k2 *= sign;
	*out = D3DXQUATERNION(k1 * q1->x + k2 * q2->x, k1 * q1->y + k2 * q2->y, k1 * q1->z + k2 * q2->z,
	                      k1 * q1->w + k2 * q2->w);
	return out;
}

UINT D3DXGetFVFVertexSize(DWORD fvf) {
	UINT size = 0;
	switch (fvf & D3DFVF_POSITION_MASK) {
	case D3DFVF_XYZ: size = 3 * sizeof(float); break;
	case D3DFVF_XYZRHW: size = 4 * sizeof(float); break;
	case D3DFVF_XYZB1: size = 4 * sizeof(float); break;
	case D3DFVF_XYZB2: size = 5 * sizeof(float); break;
	case D3DFVF_XYZB3: size = 6 * sizeof(float); break;
	case D3DFVF_XYZB4: size = 7 * sizeof(float); break;
	case D3DFVF_XYZB5: size = 8 * sizeof(float); break;
	}
	if (fvf & D3DFVF_NORMAL) size += 3 * sizeof(float);
	if (fvf & D3DFVF_PSIZE) size += sizeof(float);
	// Win32 D3DCOLOR width; the stub's DWORD is unsigned long, 8 bytes on LP64.
	if (fvf & D3DFVF_DIFFUSE) size += sizeof(std::uint32_t);
	if (fvf & D3DFVF_SPECULAR) size += sizeof(std::uint32_t);

	static const UINT coord_floats[4] = {2, 3, 4, 1};
	const DWORD tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	for (DWORD i = 0; i < tex_count; ++i)
		size += coord_floats[(fvf >> (16 + i * 2)) & 3] * sizeof(float);
	return size;
}

HRESULT D3DXComputeBoundingBox(const void *points, DWORD count, DWORD fvf, D3DXVECTOR3 *min, D3DXVECTOR3 *max) {
	if (!points || !min || !max) return D3DERR_INVALIDCALL;
	const UINT stride = D3DXGetFVFVertexSize(fvf);
	const unsigned char *p = static_cast<const unsigned char *>(points);
	D3DXVECTOR3 v;
	std::memcpy(&v, p, sizeof(v));
	*min = *max = v;
	for (DWORD i = 1; i < count; ++i) {
		std::memcpy(&v, p + i * stride, sizeof(v));
		min->x = std::fmin(min->x, v.x);
		min->y = std::fmin(min->y, v.y);
		min->z = std::fmin(min->z, v.z);
		max->x = std::fmax(max->x, v.x);
		max->y = std::fmax(max->y, v.y);
		max->z = std::fmax(max->z, v.z);
	}
	return S_OK;
}

namespace {

// Ray-parameter interval over one slab. A zero direction component divides to
// +/-inf on purpose: the interval is then everything or nothing, which is how
// d3dx8 treats a ray parallel to the slab.
void slab(float lo, float hi, float pos, float dir, float *t0, float *t1) {
	const float inv = 1.0f / dir;
	if (inv >= 0.0f) {
		*t0 = (lo - pos) * inv;
		*t1 = (hi - pos) * inv;
	} else {
		*t0 = (hi - pos) * inv;
		*t1 = (lo - pos) * inv;
	}
}

}  // namespace

BOOL D3DXBoxBoundProbe(const D3DXVECTOR3 *min, const D3DXVECTOR3 *max, const D3DXVECTOR3 *ray_pos,
                       const D3DXVECTOR3 *ray_dir) {
	float tmin, tmax, t0, t1;
	slab(min->x, max->x, ray_pos->x, ray_dir->x, &tmin, &tmax);
	if (tmax < 0.0f) return FALSE;

	slab(min->y, max->y, ray_pos->y, ray_dir->y, &t0, &t1);
	if (t1 < 0.0f || tmin > t1 || t0 > tmax) return FALSE;
	if (t0 > tmin) tmin = t0;
	if (t1 < tmax) tmax = t1;

	slab(min->z, max->z, ray_pos->z, ray_dir->z, &t0, &t1);
	if (t1 < 0.0f || tmin > t1 || t0 > tmax) return FALSE;
	return TRUE;
}

BOOL D3DXSphereBoundProbe(const D3DXVECTOR3 *center, FLOAT radius, const D3DXVECTOR3 *ray_pos,
                          const D3DXVECTOR3 *ray_dir) {
	const D3DXVECTOR3 diff = *ray_pos - *center;
	const float a = D3DXVec3Dot(ray_dir, ray_dir);
	const float b = D3DXVec3Dot(&diff, ray_dir);
	const float c = D3DXVec3Dot(&diff, &diff) - radius * radius;
	const float disc = b * b - a * c;
	if (disc <= 0.0f || std::sqrt(disc) <= b) return FALSE;
	return TRUE;
}
