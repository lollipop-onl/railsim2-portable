// Link stubs for rs2_roundtrip -> CSaveFile::Load/Save (#36).
// Real object graphs (CTrainGroup::Read, CScene::Read, ...) stay out of this
// slice; Load is expected to fail into CSynErr and report roundtrip diff.
// Do not use these stubs outside the roundtrip harness.

#include "stdafx.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <new>
#include <string>

#include "CSaveFile.h"
#include "CConfigMode.h"
#include "CSimulationMode.h"
#include "CSkinPlugin.h"
#include "CRailWay.h"
#include "CTrainGroup.h"
#include "CScene.h"
#include "CSimpleDialog.h"
#include "CParticle.h"
#include "CListView.h"
#include "CModelInst.h"
#include "CWindowDivInfo.h"
#include "CDiaInst.h"
#include "CTrainSetBuffer.h"
#include "md5.h"
#include "Language.h"
#include "SystemCover.h"
#include "port/path.h"
#include "CRailPlugin.h"
#include "CTiePlugin.h"
#include "CGirderPlugin.h"
#include "CPierPlugin.h"
#include "CLinePlugin.h"
#include "CModelSwitch.h"
#include "CLensFlare.h"
#include "CNamedObject.h"
#include "CExpression.h"
#include "CNeutralMode.h"
#include "CRailBuildMode.h"
#include "CRailEditMode.h"
#include "CStationEditMode.h"
#include "CStructEditMode.h"
#include "CDiaEditMode.h"
#include "CStation.h"
#include "CLine.h"
#include "CSkinPlugin.h"

namespace {

alignas(CSimulationMode) unsigned char g_sim_storage[sizeof(CSimulationMode)];
alignas(CConfigMode) unsigned char g_cfg_storage[sizeof(CConfigMode)];
alignas(CSkinPlugin) unsigned char g_skin_storage[sizeof(CSkinPlugin)];

char g_flash_buf[8][1024];
int g_flash_sel = 0;

}  // namespace

// --- constants / globals expected by CSaveFile ---

const float RAILSIM_VERSION = 2.15f;
const char *LAYOUT_DIRNAME = "Layout";
char *YESNO[] = {(char *)"no", (char *)"yes"};

CSaveFile *g_SaveFile = nullptr;
CSimulationMode *g_SimulationMode =
	reinterpret_cast<CSimulationMode *>(g_sim_storage);
CConfigMode *g_ConfigMode = reinterpret_cast<CConfigMode *>(g_cfg_storage);
CSkinPlugin *g_Skin = reinterpret_cast<CSkinPlugin *>(g_skin_storage);
CSurfacePlugin *g_DefaultSurface = nullptr;
CEnvPlugin *g_DefaultEnv = nullptr;
int g_NetworkDummyMapAddress = 1;
bool g_NetworkInitialized = false;
int g_NetworkSyncLimitReceived = 0;
int g_NetworkSyncLimitSent = 0;
int g_DispWidth = 640;
int g_DispHeight = 480;
int g_RSPV = 0;
CCursor g_Cursor;
CStringTexture *g_StrTex = nullptr;
CWindowCtrl *g_ModalDialog = nullptr;
CFrame g_frame;
SYSVALUE_3D sv3;
char *g_DiaDefaultString[2] = {(char *)"", (char *)""};

// Render / editor globals referenced from linked Read/Save TUs (#54).
extern const float CLIP_PLANE_NEAR;
extern const float CLIP_PLANE_FAR;
extern const float FOV_DEF;
const float CLIP_PLANE_NEAR = 0.5f;
const float CLIP_PLANE_FAR = 10000.0f;
const float FOV_DEF = 0.25f * D3DX_PI;
MAT8 *g_AltMaterial = nullptr;
SYSVALUE_L svl = {};
CTexList g_TexList;
COffScreen g_HidefCapture;
list<list<list<CRailWay *>>> g_MultiTrackRailList;
ILLPRailWay g_MultiTrackSegment;
CObject g_SegmentObject;
CPlatformInst *g_PlatformInst = nullptr;
CNeutralMode *g_NeutralMode = nullptr;
CRailBuildMode *g_RailBuildMode = nullptr;
CRailEditMode *g_RailEditMode = nullptr;
CStationEditMode *g_StationEditMode = nullptr;
CStructEditMode *g_StructEditMode = nullptr;
CDiaEditMode *g_DiaEditMode = nullptr;
bool g_ShowWarpSelect = false;
bool g_ShowPierSelect = false;
bool g_ShowPoleSelect = false;
bool g_ShowLineSelect = false;
bool g_ShadowNeeded = false;
bool g_MapDrawNeeded = false;
bool g_NamedObjectMipMap = false;
bool g_ManualControl = false;
bool g_IgnoreAcceleration = false;
bool g_HidefCaptureFlag = false;
float g_BlinkAlpha = 0.0f;
int g_HidefQuality = 0;
int g_HidefBufferSize = 0;
float g_HidefLeft = 0.0f;
float g_HidefRight = 0.0f;
float g_HidefBottom = 0.0f;
float g_HidefTop = 0.0f;

CTexList::CTexList() { m_pList = nullptr; }
CTexList::~CTexList() {}
LPTEX8 CTexList::Get(BOOL, LPCSTR, D3DCOLOR, int) { return nullptr; }
void CTexList::Release(LPTEX8) {}

COffScreen::COffScreen() : m_pTex(nullptr), m_pRT(nullptr), m_pZB(nullptr),
	m_pOldRT(nullptr), m_pOldZB(nullptr), m_fRender(FALSE) {}
COffScreen::~COffScreen() {}
BOOL COffScreen::Create(int, int) { return FALSE; }
void COffScreen::Free() {}
BOOL COffScreen::Begin(D3DCOLOR) { return FALSE; }
void COffScreen::End() {}

void rs2_roundtrip_init_stubs() {
	std::memset(g_sim_storage, 0, sizeof(g_sim_storage));
	std::memset(g_cfg_storage, 0, sizeof(g_cfg_storage));
	std::memset(g_skin_storage, 0, sizeof(g_skin_storage));
	g_SimulationMode = reinterpret_cast<CSimulationMode *>(g_sim_storage);
	g_ConfigMode = reinterpret_cast<CConfigMode *>(g_cfg_storage);
	g_Skin = reinterpret_cast<CSkinPlugin *>(g_skin_storage);
	g_BaseDir[0] = 0;
}

// --- SystemCover / script helpers ---

char *FlashIn(char *format, ...) {
	g_flash_sel = (g_flash_sel + 1) % 8;
	va_list vl;
	va_start(vl, format);
	std::vsnprintf(g_flash_buf[g_flash_sel], sizeof(g_flash_buf[0]), format, vl);
	va_end(vl);
	return g_flash_buf[g_flash_sel];
}

void ErrorDialog(char *format, ...) {
	char buf[2048];
	va_list vl;
	va_start(vl, format);
	std::vsnprintf(buf, sizeof(buf), format, vl);
	va_end(vl);
	std::fprintf(stderr, "ErrorDialog: %s\n", buf);
}

bool CheckSlash(const char *chk) {
	for (; *chk; chk = CharNext(chk)) {
		if (*chk == '\\' || *chk == '/') return true;
	}
	return false;
}

string ExpandDoubleQuote(const string &str) {
	string ret = str;
	for (size_t i = 0; i < ret.size();) {
		if (ret[i] == '\"') {
			ret[i] = '\'';
			ret.insert(i, 1, '\'');
			i += 2;
		} else {
			i++;
		}
	}
	return ret;
}

string RestoreDoubleQuote(const string &str) {
	string ret = str;
	for (size_t i = 0; i + 1 < ret.size();) {
		if (ret[i] == '\'' && ret[i + 1] == '\'') {
			ret[i] = '\"';
			ret.erase(i + 1, 1);
		} else {
			i++;
		}
	}
	return ret;
}

void EnqueueCommonDialog(CWindowCtrl *) {}
void CheckLayoutDigest(const unsigned char *) {}
void InitRailMap() {}
void InitShadow() {}
void RenderShadow() {}
void UpdateSyncLimit() {}
void SyncTrainControls(int) {}
void ExceedNetworkSyncLimit(int) {}
void GetAppPath(char *out) {
	if (out) out[0] = 0;
}
int GetKey(int) { return 0; }
int GetButton(int) { return 0; }
void Draw2DRect(int, int, int, int, unsigned long) {}
void TexMap2DRect(int, int, int, int, unsigned long) {}
void TexMap2DRect90(int, int, int, int, unsigned long) {}
void TexMap3DRect(VEC3, VEC3, VEC3, VEC3, unsigned long) {}
void CalcTextRect(int *, int *, const char *, void *) {}
BOOL CheckArguments(LPCSTR) { return TRUE; }

// --- Radio / config / skin ---

void CRadioButton::ClearGroupCheck() { m_Check = 0; }

int CRadioButton::GetNumber() {
	if (!m_Prev || !m_Next) return m_Check ? 0 : -1;
	CRadioButton *ptr = this;
	int i = 0;
	do {
		if (ptr->m_Check) return i;
		i++;
		ptr = ptr->m_Next;
	} while (ptr != this);
	return -1;
}

void CConfigMode::FreeWindowDiv() {}

void CSkinPlugin::Error() {}
void CSkinPlugin::MouseUp() {}
void CSkinPlugin::MouseDown() {}

int CSimulationMode::GetTimeScale() { return 1; }
int CSimulationMode::GetSimSpeed() { return 1; }

CWindowInfo::CWindowInfo() {
	m_Scene = nullptr;
	m_Div = nullptr;
	m_PosX = m_PosY = m_Width = m_Height = 0;
}
CWindowInfo::~CWindowInfo() {}
void CWindowInfo::OnDeleteScene(CScene *) {}
char *CWindowInfo::Read(char *str) { return str; }
void CWindowInfo::Save(FILE *, string) {}

CListElement *CListView::InsertItem(int, char *) { return nullptr; }
void CListView::EnsureVisible(int) {}
void CListView::DeleteAllItems() {}
int CListView::GetSelectionMark() { return -1; }
void CListView::SetSelectionMark(int, int) {}

CSimpleDialog::CSimpleDialog(char *, char *) {}
CYesNoDialog::CYesNoDialog(char *, char *, bool) {}
CYesNoDialog::~CYesNoDialog() {}
CInputDialog::CInputDialog(char *, char *, char *, int) {}
CMultiInputDialog::CMultiInputDialog(char *, int, char **, char **, int *) {}
CMultiInputDialog::~CMultiInputDialog() {}

CInterface *CInterface::ms_Focus = nullptr;
CInterface *CInterface::ms_Drop = nullptr;
CInterface *CInterface::ms_TabHead = nullptr;
CInterface::CInterface() {
	m_PosX = m_PosY = m_Width = m_Height = 0;
	m_Enabled = true;
	m_Visible = true;
	m_Parent = m_Brother = m_Child = m_Owner = nullptr;
	m_TabPrev = m_TabNext = nullptr;
}
CInterface::~CInterface() {}
void CInterface::Init(int, int, int, int, char *, CInterface *) {}
void CInterface::LinkTab(bool) {}
void CInterface::GetAbsPos(int *x, int *y) { if (x) *x = 0; if (y) *y = 0; }
bool CInterface::IsInside(int, int) { return false; }
void CInterface::GiveFocus(bool) {}
bool CInterface::ScanInput() { return false; }
void CInterface::Render() {}
void CInterface::DrawFocusFrame() {}
void CInterface::ProcessKey() {}
void CInterface::SetChild(CInterface *) {}
void CInterface::SetBrother(CInterface *) {}
void CInterface::RemoveChild(CInterface *) {}
CInterface *CInterface::FindTabItem() { return nullptr; }
bool CInterface::IsVisible() { return m_Visible; }

bool CEditBox::ms_Active = false;

CWindowCtrl::CWindowCtrl() {}
CWindowCtrl::~CWindowCtrl() {}
void CWindowCtrl::Init(int, int, int, int, char *, CInterface *, bool) {}
void CWindowCtrl::Render() {}
void CWindowCtrl::SetSize(int, int) {}
bool CWindowCtrl::ScanInput() { return false; }

CEditBox::CEditBox() {}
CEditBox::~CEditBox() {}
void CEditBox::Create(int, int, int, int, string, int) {}
void CEditBox::Render() {}
void CEditBox::Release(int) {}
int CEditBox::ScanInput() { return 0; }

CTexture::CTexture() {}
CTexture::~CTexture() {}
BOOL CTexture::Create(int, int) { return FALSE; }
BOOL CTexture::DrawInText(int, int, LPCSTR, HFONT, unsigned long, unsigned long, int, int) { return FALSE; }

void CGameMode::WakeUp() {}

// Extra vtable / member stubs pulled by dialog and model headers.

CSoundState::~CSoundState() {}

bool CYesNoDialog::ScanInputWindow() { return false; }
bool CMultiInputDialog::ScanInputWindow() { return false; }
bool CInputDialog::ScanInputWindow() { return false; }

// Force vtables for controls used as dialog members.
void CPushButton::Init(int, int, int, int, char *, CInterface *) {}
bool CPushButton::ScanInput() { return false; }
void CPushButton::Render() {}
void CStaticCtrl::Init(int, int, int, int, char *, CInterface *, int, int) {}
void CStaticCtrl::Render() {}

void CEditCtrl::Init(int, int, int, int, char *, CInterface *, int) {}
void CEditCtrl::GiveFocus(bool) {}
void CEditCtrl::FinishInput() {}

CWave::~CWave() {}
void CEditCtrl::Render() {}
bool CEditCtrl::ScanInput() { return false; }
