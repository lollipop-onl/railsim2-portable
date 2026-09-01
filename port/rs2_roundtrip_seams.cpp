// Link seams for rs2_roundtrip (RS2_ROUNDTRIP=1).
// Minimal no-op bodies for symbols not provided by game TUs in the roundtrip
// allowlist. Do not use outside the roundtrip harness.

#include "stdafx.h"

#include "RailMap.h"
#include "HighTimer.h"
#include "CCursor.h"
#include "CCustomizer.h"
#include "CEffector.h"
#include "CModelPlugin.h"
#include "CNamedObject.h"
#include "CNeutralMode.h"
#include "CPluginTree.h"
#include "CProfilePlugin.h"
#include "CRailBuildMode.h"
#include "CRailWay.h"
#include "CSoundEffector.h"
#include "CTextureAnimation.h"
#include "CTrainGroupTemplate.h"
#include "CTrainPlugin.h"
#include "CGameMode.h"
#include "CStringTexture.h"
#include "CRailBuilder.h"
#include "CVertexDump.h"
#include "CSkinPlugin.h"
#include "CLensFlare.h"

void PushUndoStack();
void PushChatLog(char *, D3DCOLOR color);

// --- free functions ---

BOOL BeginScene(D3DCOLOR c) {
	(void)c;
	return FALSE;
}

void CastShadow(CObject *obj) { (void)obj; }

void Draw2DLine(int x1, int y1, int x2, int y2, D3DCOLOR c1, D3DCOLOR c2) {
	(void)x1;
	(void)y1;
	(void)x2;
	(void)y2;
	(void)c1;
	(void)c2;
}

void Fill2DRect(int x1, int y1, int x2, int y2, D3DCOLOR c) {
	(void)x1;
	(void)y1;
	(void)x2;
	(void)y2;
	(void)c;
}

void PushChatLog(char *chat, D3DCOLOR color) {
	(void)chat;
	(void)color;
}

void RailMapLine(VEC3 p1, D3DCOLOR c1, VEC3 p2, D3DCOLOR c2, bool shadow, bool bold) {
	(void)p1;
	(void)c1;
	(void)p2;
	(void)c2;
	(void)shadow;
	(void)bold;
}

void RailMapText(VEC3 pos, char *text, D3DCOLOR color) {
	(void)pos;
	(void)text;
	(void)color;
}

void SetDirLight(VEC3 dir, D3DCOLORVALUE cv) {
	(void)dir;
	(void)cv;
}

bool InitHighTimer() { return false; }

void PushUndoStack() {}

void NormalizeMatrix(MTX4 *mtx) { (void)mtx; }

void devResetMaterial() {}

void Draw3DPointAs2DRect(VEC3 pos, D3DCOLOR c, int size) {
	(void)pos;
	(void)c;
	(void)size;
}

void EnqueuePointControl(void *point_id, int point_opt) {
	(void)point_id;
	(void)point_opt;
}

void Draw3DLineWithShadow(VEC3 pos1, VEC3 pos2, D3DCOLOR c1, D3DCOLOR c2) {
	(void)pos1;
	(void)pos2;
	(void)c1;
	(void)c2;
}

void EnqueueSwitchControl(int switch_id, int switch_opt) {
	(void)switch_id;
	(void)switch_opt;
}

void EnqueueSetTrainControl(
	void *set_group, void *set_rail, int set_side, float set_sumlen, int set_flag) {
	(void)set_group;
	(void)set_rail;
	(void)set_side;
	(void)set_sumlen;
	(void)set_flag;
}

void EnqueueMergeTrainControl(
	void *merge_group1, void *merge_group2, int merge_side1, int merge_side2) {
	(void)merge_group1;
	(void)merge_group2;
	(void)merge_side1;
	(void)merge_side2;
}

void MoveVW(VEC3 v) { (void)v; }

void DrawBox(BOX8 *pB, D3DCOLOR c) {
	(void)pB;
	(void)c;
}

void SetView(VEC3 pos, VEC3 dir, VEC3 up) {
	(void)pos;
	(void)dir;
	(void)up;
}

char *FlashOut(int n) {
	(void)n;
	return nullptr;
}

LONG GetWheel() { return 0; }

VEC3 WorldToScreen(VEC3 pos) {
	(void)pos;
	return VEC3(0.0f, 0.0f, -1.0f);
}

// --- static members ---

MAT8 CObject::matShadow = {{0, 0, 0, 0.5f}};

CGameMode *CGameMode::ms_ActiveMode = nullptr;

VEC3 CBodyObject::ms_TiltDir;

set<CSoundState *> CSoundEffector::ms_PlayList;

// --- CCursor ---

bool CCursor::CheckDrag() { return false; }

void CCursor::Lock() {}

void CCursor::Release() {}

// --- CCustomizerContainer ---

CCustomizerContainer::CCustomizerContainer() : m_Customizer(nullptr) {}

CCustomizerContainer::CCustomizerContainer(const CCustomizerContainer &src) {
	(void)src;
	m_Customizer = nullptr;
}

CCustomizerContainer::~CCustomizerContainer() {}

char *CCustomizerContainer::Read(char *str, CModelPlugin *mpi) {
	(void)mpi;
	return str;
}

// --- CEffectorContainer ---

CEffectorContainer::CEffectorContainer() : m_Effector(nullptr) {}

CEffectorContainer::CEffectorContainer(const CEffectorContainer &src) {
	(void)src;
	m_Effector = nullptr;
}

CEffectorContainer::~CEffectorContainer() {}

char *CEffectorContainer::Read(char *str, CModelPlugin *mpi) {
	(void)mpi;
	return str;
}

// --- CFreeObjectContainer ---

CFreeObjectContainer::~CFreeObjectContainer() {}

// --- CMesh ---

void CMesh::ResetMatFlag(DWORD def) { (void)def; }

// --- CModelPlugin ---

CModelPlugin::CModelPlugin(char *id) : CPlugin(id) {
	m_SelectSwitchID = 0;
	m_PartsNum = 0;
	m_SelectSwitch = nullptr;
	m_LinkInst = nullptr;
}

void CModelPlugin::CopySwitch(vector<int> &sopt) { (void)sopt; }

CModelSwitch *CModelPlugin::FindModelSwitch(const string &name) {
	(void)name;
	return nullptr;
}

CModelSwitch *CModelPlugin::GetModelSwitch(int index) {
	(void)index;
	return nullptr;
}

void CModelPlugin::LoadSoundWave(CModelInst *minst) { (void)minst; }

void CModelPlugin::SetMoverState(CModelInst *minst) { (void)minst; }

void CModelPlugin::SetPartsInst(CModelInst *minst) { (void)minst; }

bool CModelPlugin::SetSwitch(vector<int> &sopt) {
	(void)sopt;
	return false;
}

void CModelPlugin::SetSwitch(CModelInst *minst) { (void)minst; }

void CModelPlugin::CheckGroupCommonSwitch(
	CModelInst *minst, CModelSwitch *sw, int val) {
	(void)minst;
	(void)sw;
	(void)val;
}

void CModelPlugin::CheckGroupCommonSwitchAll(CModelInst *minst1, CModelInst *minst2) {
	(void)minst1;
	(void)minst2;
}

// --- CNeutralMode ---

void CNeutralMode::DeleteModelInst(CModelInst *minst) { (void)minst; }

// --- CObject ---

CObject::CObject() : m_pMesh(nullptr), m_pParent(nullptr), m_scale(1.0f) {
	D3DXMatrixIdentity(&m_mtx);
}

void CObject::SetMesh(CMesh *pM, VEC3 p, float s) {
	(void)pM;
	(void)p;
	(void)s;
}

void CObject::Move(VEC3 v) { (void)v; }

void CObject::RotZ(float v) { (void)v; }

void CObject::RotAxis(VEC3 ax, float v) {
	(void)ax;
	(void)v;
}

void CObject::SetDir(VEC3 dir, VEC3 up) {
	(void)dir;
	(void)up;
}

void CObject::Render() {}

void CObject::RenderSC(MAT8 *pMat) { (void)pMat; }

float CObject::GetRadius() { return 0.0f; }

VEC3 CObject::GetCenter() { return VEC3(0.0f, 0.0f, 0.0f); }

BOX8 CObject::GetBox() { return BOX8(); }

// --- CPluginTree ---

void CPluginTree::SelectPlugin(CPlugin *t) { (void)t; }

// --- CProfile / CWireframe ---

CProfile::~CProfile() {}

CWireframe::~CWireframe() {}

// --- CProfilePlugin ---

void CProfilePlugin::Dump(
	VEC3 &p1, VEC3 &r1, VEC3 &u1, VEC3 &ip1, VEC3 &ir1, VEC3 &iu1, VEC3 &p2,
	VEC3 &r2, VEC3 &u2, VEC3 &ip2, VEC3 &ir2, VEC3 &iu2, float len, int prev) {
	(void)p1;
	(void)r1;
	(void)u1;
	(void)ip1;
	(void)ir1;
	(void)iu1;
	(void)p2;
	(void)r2;
	(void)u2;
	(void)ip2;
	(void)ir2;
	(void)iu2;
	(void)len;
	(void)prev;
}

void CProfilePlugin::Render(
	VEC3 &p1, VEC3 &r1, VEC3 &u1, VEC3 &d1, VEC3 &ip1, VEC3 &ir1, VEC3 &iu1,
	VEC3 &p2, VEC3 &r2, VEC3 &u2, VEC3 &d2, VEC3 &ip2, VEC3 &ir2, VEC3 &iu2,
	int close, float len, MAT8 *altmat) {
	(void)p1;
	(void)r1;
	(void)u1;
	(void)d1;
	(void)ip1;
	(void)ir1;
	(void)iu1;
	(void)p2;
	(void)r2;
	(void)u2;
	(void)d2;
	(void)ip2;
	(void)ir2;
	(void)iu2;
	(void)close;
	(void)len;
	(void)altmat;
}

void CProfilePlugin::ResetMapTemp() {}

void CProfilePlugin::CopyMapTemp(vector<float> &mapv) { (void)mapv; }

void CProfilePlugin::AddMapTemp(float dist) { (void)dist; }

void CProfilePlugin::SetMapTemp(vector<float> &mapv) { (void)mapv; }

// --- CProfilePluginList ---

void CProfilePluginList::ClearDump() {}

void CProfilePluginList::PrepareVertex() {}

void CProfilePluginList::RenderAll() {}

// --- CRailBuildMode ---

void CRailBuildMode::ResetBuilder() {}

// --- CRailLinkTemp ---

CRailLinkTemp::CRailLinkTemp(
	int side, float sumlen, VEC3 &pos, VEC3 &right, VEC3 &up, VEC3 &dir,
	CRailWay *link, IRailSplitter sit) {
	m_Side = side;
	m_SumLen = sumlen;
	m_SpliceItr = sit;
	m_Pos = pos;
	m_Right = right;
	m_Up = up;
	m_Dir = dir;
	m_Link = link;
}

// --- CSoundEffector / CSoundState ---

void CSoundEffector::InitPlayList() { ms_PlayList.clear(); }

void CSoundState::Confirm(CSoundEffector *eff, bool enabled) {
	(void)eff;
	(void)enabled;
}

// --- CStringTexture / CStringDrawer ---

void CStringDrawer::RenderLeft(int x, int y, D3DCOLOR c, int mw, int mh) {
	(void)x;
	(void)y;
	(void)c;
	(void)mw;
	(void)mh;
}

CStringDrawer *CStringTexture::DrawString(const char *str, D3DCOLOR sdw) {
	(void)str;
	(void)sdw;
	return nullptr;
}

// --- CTextureAnimation ---

CTexAnimFrame::~CTexAnimFrame() {}

char *CTexAnimState::Read(char *str) {
	char *eee;
	if (!(str = Assignment(eee = str, "TexAnimState"))) return NULL;
	if (!(str = ConstInteger(eee = str, &m_Frame))) throw CSynErr(eee);
	if (!(str = Character2(eee = str, ','))) throw CSynErr(eee);
	if (!(str = ConstInteger(eee = str, &m_Count))) throw CSynErr(eee);
	if (!(str = Character2(eee = str, ';'))) throw CSynErr(eee);
	return str;
}

void CTexAnimState::Save(FILE *df, char *ind) {
	fprintf(df, "%sTexAnimState = %d, %d;\n", ind, m_Frame, m_Count);
}

// --- CTrainTemplate ---

void CTrainTemplate::Init(CTrainPlugin *tpi, bool turn, vector<int> &sopt) {
	(void)tpi;
	(void)turn;
	(void)sopt;
}

// --- CWave ---

CWave::CWave() : m_pSB(nullptr), m_pFX(nullptr), m_p3D(nullptr), m_BytesPerSec(0) {}

// --- CAxleObject / CBodyObject ---

float CAxleObject::CalcRotation(float dist) {
	(void)dist;
	return 0.0f;
}

void CAxleObject::SetPosture(VEC3 pos, VEC3 dir, VEC3 up, float rot) {
	(void)pos;
	(void)dir;
	(void)up;
	(void)rot;
}

// --- remaining globals / mesh / rail editor seams ---

bool g_ShowRailSelect = false;
bool g_ShowRailWaySelect = false;
bool g_ShowRailBlockSelect = false;
bool g_ShowSpeedLimitSelect = false;
ILPRailWay g_SingleTrackSegment;
int CRailBuilder::ms_CurrentTrack = 0;
CObject g_LinkObject;
CMeshList g_MeshList;

CMeshList::CMeshList() : m_pList(nullptr) {}
CMeshList::~CMeshList() {}
CMesh *CMeshList::Get(BOOL, LPCSTR, D3DCOLOR, int) { return nullptr; }
void CMeshList::Release(CMesh *) {}

void Draw3DLine(VEC3 p1, VEC3 p2, D3DCOLOR c1, D3DCOLOR c2) {
	(void)p1;
	(void)p2;
	(void)c1;
	(void)c2;
}

bool LineLineNearest(VEC3 *a, VEC3 *b, VEC3 *c, VEC3 *d, VEC3 *e, VEC3 *f) {
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	return false;
}

void CLineDumpL::Add(VEC3 p1, D3DCOLOR c1, VEC3 p2, D3DCOLOR c2) {
	(void)p1;
	(void)c1;
	(void)p2;
	(void)c2;
}

void CModelPlugin::SetAnimation(CModelInst *mi) { (void)mi; }

void CStringDrawer::RenderLeft3D(VEC3 p, VEC3 r, VEC3 u, D3DCOLOR c, float s) {
	(void)p;
	(void)r;
	(void)u;
	(void)c;
	(void)s;
}

void CStringDrawer::RenderRight3D(VEC3 p, VEC3 r, VEC3 u, D3DCOLOR c, float s) {
	(void)p;
	(void)r;
	(void)u;
	(void)c;
	(void)s;
}

void CObject::RenderCustom(CNamedObject *no) { (void)no; }

void CObject::SetScale(float s) { (void)s; }

CObject::~CObject() {}

void CHeadlight::InitRenderList() {}

void CHeadlight::RenderAll() {}

void CProfilePluginList::ClearDumpAll() {}
