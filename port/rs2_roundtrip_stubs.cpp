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
char *YESNO[] = {(char *)"No", (char *)"Yes"};

char g_BaseDir[1024];
CSaveFile *g_SaveFile = nullptr;
CSimulationMode *g_SimulationMode =
	reinterpret_cast<CSimulationMode *>(g_sim_storage);
CConfigMode *g_ConfigMode = reinterpret_cast<CConfigMode *>(g_cfg_storage);
CSkinPlugin *g_Skin = reinterpret_cast<CSkinPlugin *>(g_skin_storage);
CTrain *g_CabinViewTrain = nullptr;
CSurfacePlugin *g_DefaultSurface = nullptr;
CEnvPlugin *g_DefaultEnv = nullptr;
map<std::string, CTrainGroup *> g_RailBlockMap;
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
VEC3 g_WindDir;
VEC3 g_WindDirNorm;
CFrame g_frame;
SYSVALUE_3D sv3;
char *g_DiaDefaultString[2] = {(char *)"", (char *)""};
CModelSwitch g_SystemSwitch[1];
CRailPluginList *g_RailPluginList = nullptr;
CTiePluginList *g_TiePluginList = nullptr;
CGirderPluginList *g_GirderPluginList = nullptr;
CPierPluginList *g_PierPluginList = nullptr;
CLinePluginList *g_LinePluginList = nullptr;

namespace LanguageResource {
string SyntaxError = "SyntaxError";
string CommentEndNotFound = "CommentEndNotFound";
string StringLiteralExceedLineBreak = "StringLiteralExceedLineBreak";
string CannotOpenFile = "CannotOpenFile";
string InvalidVersion = "InvalidVersion";
string UnsupportedVersion = "UnsupportedVersion";
string InvalidDatafileType = "InvalidDatafileType";
string LackedPluginMessage = "LackedPluginMessage";
string LackedPluginListSaved = "LackedPluginListSaved";
string LackedPluginListFailed = "LackedPluginListFailed";
string InitialConsist = "InitialConsist";
string InitialScene = "InitialScene";
string NewConsist = "NewConsist";
string NewScene = "NewScene";
string NotSet = "NotSet";
string Set = "Set";
string Yes = "Yes";
string No = "No";
string Cancel = "Cancel";
}  // namespace LanguageResource

string g_LanguageName = "stub";

void rs2_roundtrip_init_stubs() {
	std::memset(g_sim_storage, 0, sizeof(g_sim_storage));
	std::memset(g_cfg_storage, 0, sizeof(g_cfg_storage));
	std::memset(g_skin_storage, 0, sizeof(g_skin_storage));
	g_SimulationMode = reinterpret_cast<CSimulationMode *>(g_sim_storage);
	g_ConfigMode = reinterpret_cast<CConfigMode *>(g_cfg_storage);
	g_Skin = reinterpret_cast<CSkinPlugin *>(g_skin_storage);
	g_RailBlockMap.clear();
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

char *LoadBinaryText(FILE *file, int maxbyte) {
	if (!file) return nullptr;
	if (std::fseek(file, 0, SEEK_END) != 0) {
		std::fclose(file);
		return nullptr;
	}
	long size = std::ftell(file);
	if (size < 0) {
		std::fclose(file);
		return nullptr;
	}
	if (0 <= maxbyte && maxbyte < size) size = maxbyte;
	if (std::fseek(file, 0, SEEK_SET) != 0) {
		std::fclose(file);
		return nullptr;
	}
	char *buf = new char[static_cast<size_t>(size) + 1];
	if (std::fread(buf, 1, static_cast<size_t>(size), file) !=
		static_cast<size_t>(size)) {
		std::fclose(file);
		delete[] buf;
		return nullptr;
	}
	buf[size] = 0;
	std::fclose(file);
	return buf;
}

char *LoadBinaryText(char *fname, int maxbyte) {
	return LoadBinaryText(fopen(fname, "rb"), maxbyte);
}

void EnqueueCommonDialog(CWindowCtrl *) {}
void ClearRailBlockUser(CTrainGroup *) { g_RailBlockMap.clear(); }
void ClearStaticSwitchTable() {}
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

// --- Object graph stubs: construct, then fail Read immediately ---

CRailWay **CRailWay::ms_Root = nullptr;
float CRailWay::ms_MinDist = -1.0f;
CRailWayLink CRailWay::ms_Detect;

CRailWay::CRailWay() {
	m_OldAdr = nullptr;
	m_Selected = 0;
	m_PierPos = m_PolePos = m_SegLen = 0;
	m_BuildLine = m_WarpDummy = m_MultiTrackDummy = false;
	m_DummyTrackNum = 0;
	m_DummyTrackInterval = 0;
	m_SpeedLimit = -1;
	m_Parent = nullptr;
	m_Platform = nullptr;
	m_Scene = nullptr;
	m_RailPlugin = nullptr;
	m_TiePlugin = nullptr;
	m_GirderPlugin = nullptr;
	m_PierPlugin = nullptr;
	m_LinePlugin = nullptr;
	m_PolePlugin = nullptr;
	m_Next = nullptr;
}
CRailWay::~CRailWay() {}
CRailWay *CRailWay::Delete() {
	delete this;
	return nullptr;
}
char *CRailWay::ReadWarp(char *) {
	delete this;
	return nullptr;
}
void CRailWay::SaveWarp(FILE *) {}
void CRailWay::RestoreAddress() {}
void CRailWay::RenderWarp() {}
void CRailWay::ScanInputWarp(int, VEC3 &, VEC3 &) {}
void CRailWay::CheckWarpEndScene(CScene *) {}

CDiaElement::CDiaElement() {
	m_Action = m_TimeType = m_Hour = m_Minute = m_Second = 0;
	m_Offset = 0;
}
CGroupEndLocator::CGroupEndLocator() {
	m_Side = m_Type = 0;
	m_Offset = 0;
	m_SetRail = nullptr;
	m_Group = nullptr;
}
CGroupEndLocator::CGroupEndLocator(int, float, CRailWay *, CTrainGroup *) {
	m_Side = m_Type = 0;
	m_Offset = 0;
	m_SetRail = nullptr;
	m_Group = nullptr;
}

CTrainGroup::CTrainGroup(char *name) {
	m_OldAdr = nullptr;
	m_ControlState = m_Serial = m_State = m_DoorWait = 0;
	m_Length = m_MaxVelocity = m_MaxAcceleration = m_MaxDeceleration = 0;
	m_DoorClosingTime = m_TargetSpeed = m_EffectTargetSpeed = 0;
	m_CurrentSpeed = m_OldSpeed = m_StopTarget = m_PreviewOffset = 0;
	m_SpeedLimit = 0;
	m_DepartureTime = 0;
	m_OpenDoor[0] = m_OpenDoor[1] = false;
	m_Reverse = m_Enabled = m_NotifyFlag = false;
	m_SplitPos = 0;
	m_ListElement = nullptr;
	m_Platform = nullptr;
	m_TrainList = nullptr;
	m_SelectTrain = nullptr;
	m_Next = nullptr;
	if (name) m_Name = name;
}
CTrainGroup::~CTrainGroup() {}
char *CTrainGroup::Read(char *, CTrainGroup ***) {
	delete this;
	return nullptr;
}
void CTrainGroup::Save(FILE *) {}
void CTrainGroup::RestoreAddress() {}
void CTrainGroup::RestoreSet() {}
int CTrainGroup::GetTrainNum() { return 0; }
void CTrainGroup::Render() {}
void CTrainGroup::Simulate() {}
void CTrainGroup::ScanInput(int, VEC3 &, VEC3 &) {}

int CModelInst::ms_DetectMode = 0;
int CModelInst::ms_DetectTemp = 0;
float CModelInst::ms_MinDist = -1.0f;
VEC3 CModelInst::ms_DetectRect1;
VEC3 CModelInst::ms_DetectRect2;
BOX8 CModelInst::ms_FocusBox;
CModelInst *CModelInst::ms_CurrentInst = nullptr;
CDetectInfo CModelInst::ms_DetectInfo;

CModelInst::CModelInst() {
	m_Selected = 0;
	m_Pos = m_Right = m_Up = m_Dir = V3ZERO;
	m_ModelPlugin = nullptr;
}
CModelInst::CModelInst(CModelPlugin *p) : CModelInst() { (void)p; }
CModelInst::~CModelInst() {}
void CModelInst::ResetDetect(int, VEC3, VEC3) {}

CStruct **CStruct::ms_Root = nullptr;
CStruct::CStruct() : CModelInst() {
	m_Scene = nullptr;
	m_StructPlugin = nullptr;
	m_Next = nullptr;
}
CStruct::CStruct(CStructPlugin *) : CStruct() {}
CStruct::CStruct(CStructPlugin *, VEC3, VEC3, VEC3, bool) : CStruct() {}
CStruct::~CStruct() {}

CScene::CScene() : CStruct() {
	m_Serial = 0;
	m_ListElement = nullptr;
	m_RailConnector = nullptr;
	m_RailWay = nullptr;
	m_Pier = nullptr;
	m_Line = nullptr;
	m_Pole = nullptr;
	m_Station = nullptr;
	m_Struct = nullptr;
	m_SurfacePlugin = nullptr;
	m_EnvPlugin = nullptr;
	m_IsDumpReady = false;
	m_Next = nullptr;
}
CScene::CScene(CSurfacePlugin *, CEnvPlugin *, char *name) : CScene() {
	if (name) m_Name = name;
}
CScene::~CScene() {}
void CScene::Delete() { delete this; }
void CScene::DeleteGroup(CTrainGroup *) {}
void CScene::Enter(bool) {}
void CScene::RestoreAddress() {}
char *CScene::Read(char *, CScene ***) {
	delete this;
	return nullptr;
}
void CScene::Save(FILE *) {}
void CScene::RenderAfter() {}
void CScene::RenderScene() {}
void CScene::SimulateScene() {}
void CScene::ScanInputLine(int, VEC3, VEC3) {}
void CScene::ScanInputPier(int, VEC3, VEC3) {}
void CScene::ScanInputPole(int, VEC3, VEC3) {}
void CScene::ScanInputRailWay(int, VEC3, VEC3, bool) {}
void CScene::ScanInputStation(int, VEC3, VEC3, bool) {}
void CScene::ScanInputStruct(int, VEC3, VEC3, bool) {}

void CParticle::InitRenderList() {}
void CParticle::RenderAll() {}
void CParticle::SimulateAll() {}
void CHeadlight::InitRenderList() {}
void CHeadlight::RenderAll() {}
void CNamedObject::InitAfterRenderList() {}
void CNamedObject::AfterRenderAll() {}
void CProfilePluginList::ClearDumpAll() {}

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

CModelSwitch::CModelSwitch() {}
CDiaElementBase::CDiaElementBase() {}
CSoundState::~CSoundState() {}
CObject::~CObject() {}

bool CYesNoDialog::ScanInputWindow() { return false; }
bool CMultiInputDialog::ScanInputWindow() { return false; }
bool CInputDialog::ScanInputWindow() { return false; }

bool CStruct::IsSelectVisible() { return false; }
void CStruct::SimulateModelInst() {}
CModelInst *CStruct::Control() { return this; }
CPartsInst *CStruct::FindParts(CNamedObject *) { return nullptr; }
void CStruct::PrintInfo() {}

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

// Emit CDiaElement vtable by defining a key virtual if needed.
string CDiaElement::GetListCaption() { return string(); }
char *CDiaElement::Read(char *str) { return str; }
void CDiaElement::Save(FILE *, char *) {}
