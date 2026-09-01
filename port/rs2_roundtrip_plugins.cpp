// Minimal plugin types for rs2_roundtrip (#54).
// CPluginList::List/FindPlugin for layout Read/Save without CRailPlugin.cpp etc.

#include "stdafx.h"

#include "CRailPlugin.h"
#include "CTiePlugin.h"
#include "CGirderPlugin.h"
#include "CPierPlugin.h"
#include "CLinePlugin.h"
#include "CPolePlugin.h"
#include "CSurfacePlugin.h"
#include "CEnvPlugin.h"
#include "CStructPlugin.h"
#include "CTrainPlugin.h"
#include "CStationPlugin.h"
#include "CModelInst.h"

namespace {

template <typename ListT>
void reset_plugin_list(ListT *&list) {
	delete list;
	list = nullptr;
}

}  // namespace

// --- plugin list globals ---

CRailPluginList *g_RailPluginList = nullptr;
CTiePluginList *g_TiePluginList = nullptr;
CGirderPluginList *g_GirderPluginList = nullptr;
CPierPluginList *g_PierPluginList = nullptr;
CLinePluginList *g_LinePluginList = nullptr;
CPolePluginList *g_PolePluginList = nullptr;
CSurfacePluginList *g_SurfacePluginList = nullptr;
CEnvPluginList *g_EnvPluginList = nullptr;
CStructPluginList *g_StructPluginList = nullptr;
CTrainPluginList *g_TrainPluginList = nullptr;
CStationPluginList *g_StationPluginList = nullptr;

// --- extra globals from removed plugin TUs ---

CTrainPlugin *g_Train = nullptr;
CStructPlugin *g_Struct = nullptr;
CStationPlugin *g_Station = nullptr;
float g_DayAlpha = 0.0f;
float g_NightAlpha = 0.0f;
D3DCOLOR g_NoLightColor = 0;
CDetectInfo g_StationPlatformParentDetectInfo;

// --- profile plugins ---

CRailPlugin::CRailPlugin(char *id) : CProfilePlugin(id) {
	m_WheelSound = nullptr;
}

CRailPlugin::~CRailPlugin() {}

bool CRailPlugin::Load() { return true; }

void CRailPlugin::SetPreview() {}

void CRailPlugin::BeforeDump(VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &) {}

void CRailPlugin::AfterDump(VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &,
	VEC3 &) {}

void CRailPlugin::CalcPierPos(VEC3 *, VEC3 *, VEC3 *, VEC3 *) {}

float CRailPlugin::CantFunc(float) { return 0.0f; }

void CRailPlugin::PlayWheelSound(float, float, VEC3 &) {}

bool CTiePlugin::Load() { return true; }

void CTiePlugin::SetPreview() {}

void CTiePlugin::AfterDump(VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &, VEC3 &,
	VEC3 &) {}

void CTiePlugin::CalcPierPos(VEC3 *, VEC3 *, VEC3 *, VEC3 *) {}

bool CGirderPlugin::Load() { return true; }

void CGirderPlugin::SetPreview() {}

void CGirderPlugin::CalcPierPos(VEC3 *, VEC3 *, VEC3 *, VEC3 *) {}

bool CPierPlugin::Load() { return true; }

void CPierPlugin::SetPreview() {}

void CPierPlugin::AddPierPos(float len) {
	m_PierPos -= len;
	while (m_PierPos <= 0.0f) m_PierPos += m_Interval;
}

bool CLinePlugin::Load() { return true; }

void CLinePlugin::SetPreview() {}

void CLinePlugin::AddPolePos(float len) {
	m_PolePos -= len;
	while (m_PolePos <= 0.0f) m_PolePos += m_MaxInterval;
}

bool CPolePlugin::Load() { return true; }

void CPolePlugin::SetPreview() {}

// --- CEnvPlugin ---

bool CEnvPlugin::Load() { return true; }

void CEnvPlugin::SetPreview() {}

void CEnvPlugin::Render(double) {}

void CEnvPlugin::RenderAfter() {}

// --- CStructPlugin hierarchy ---

CStructPlugin::~CStructPlugin() {}

bool CStructPlugin::Load() { return true; }

char *CStructPlugin::LoadStructBefore(char *str) { return str; }

bool CStructPlugin::LoadOldForm() { return true; }

void CStructPlugin::SetPreview() {}

CNamedObject *CStructPlugin::FindObject(const string &) { return nullptr; }

bool CStructPlugin::IsSoundEnabled() { return false; }

void CStructPlugin::SetPosture() {}

void CStructPlugin::ScanInput(CStruct *) {}

void CStructPlugin::Render(CStruct *) {}

void CStructPlugin::Simulate(CStruct *) {}

CSurfacePlugin::~CSurfacePlugin() {}

char *CSurfacePlugin::LoadStructBefore(char *str) { return str; }

bool CSurfacePlugin::LoadOldForm() { return true; }

void CSurfacePlugin::SetPreview() {}

bool CSurfacePlugin::IsSoundEnabled() { return false; }

bool CSurfacePlugin::PickSurface(VEC3, VEC3, VEC3 *, VEC3 *, int) {
	return false;
}

bool CSurfacePlugin::ClipRect(VEC3 *) { return true; }

CStationPlugin::~CStationPlugin() {}

char *CStationPlugin::LoadStructBefore(char *str) { return str; }

char *CStationPlugin::LoadStructAfter(char *str) { return str; }

bool CStationPlugin::LoadOldForm() { return true; }

void CStationPlugin::SetPreview() {}

void CStationPlugin::PreviewStruct() {}

void CStationPlugin::BuildPlatform(CStation *) {}

// --- CTrainPlugin ---

CTrainPlugin::~CTrainPlugin() {}

bool CTrainPlugin::Load() { return true; }

bool CTrainPlugin::LoadOldForm() { return true; }

void CTrainPlugin::SetPreview() {}

CNamedObject *CTrainPlugin::FindObject(const string &) { return nullptr; }

bool CTrainPlugin::IsSoundEnabled() { return false; }

void CTrainPlugin::SetAxleList(CTrain *) {}

void CTrainPlugin::SetPosture() {}

void CTrainPlugin::AttachPartsObject() {}

void CTrainPlugin::ScanInput(CTrain *) {}

void CTrainPlugin::Render(CTrain *) {}

void CTrainPlugin::Simulate(CTrain *) {}

void CTrainPlugin::Preview(float, bool) {}

void rs2_roundtrip_init_plugins() {
	reset_plugin_list(g_RailPluginList);
	reset_plugin_list(g_TiePluginList);
	reset_plugin_list(g_GirderPluginList);
	reset_plugin_list(g_PierPluginList);
	reset_plugin_list(g_LinePluginList);
	reset_plugin_list(g_PolePluginList);
	reset_plugin_list(g_SurfacePluginList);
	reset_plugin_list(g_EnvPluginList);
	reset_plugin_list(g_StructPluginList);
	reset_plugin_list(g_TrainPluginList);
	reset_plugin_list(g_StationPluginList);

	g_RailPluginList = new CRailPluginList;
	g_TiePluginList = new CTiePluginList;
	g_GirderPluginList = new CGirderPluginList;
	g_PierPluginList = new CPierPluginList;
	g_LinePluginList = new CLinePluginList;
	g_PolePluginList = new CPolePluginList;
	g_SurfacePluginList = new CSurfacePluginList;
	g_EnvPluginList = new CEnvPluginList;
	g_StructPluginList = new CStructPluginList;
	g_TrainPluginList = new CTrainPluginList;
	g_StationPluginList = new CStationPluginList;

	if (!g_BaseDir[0]) return;

	g_RailPluginList->ListIdsOnly();
	g_TiePluginList->ListIdsOnly();
	g_GirderPluginList->ListIdsOnly();
	g_PierPluginList->ListIdsOnly();
	g_LinePluginList->ListIdsOnly();
	g_PolePluginList->ListIdsOnly();
	g_SurfacePluginList->ListIdsOnly();
	g_EnvPluginList->ListIdsOnly();
	g_StructPluginList->ListIdsOnly();
	g_TrainPluginList->ListIdsOnly();
	g_StationPluginList->ListIdsOnly();
}
