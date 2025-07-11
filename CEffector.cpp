#include "stdafx.h"
#include "CModelPlugin.h"

//	外部グローバル
extern CScene *g_Scene;

//	内部グローバル
CHeadlight g_TempHeadlight;
CParticle g_TempParticle;
CSoundEffector g_TempSoundEffector;

/*
 *	コンストラクタ
 */
CEffectorContainer::CEffectorContainer(){
	m_Effector = NULL;
}

/*
 *	コピーコンストラクタ
 */
CEffectorContainer::CEffectorContainer(
	const CEffectorContainer &src	//	コピー元
){
	m_Effector = src.m_Effector->Duplicate();
}

/*
 *	デストラクタ
 */
CEffectorContainer::~CEffectorContainer(){
	DELETE_V(m_Effector);
}

/*
 *	読込
 */
char *CEffectorContainer::Read(
	char *str,			//	対象文字列
	CModelPlugin *mpi	//	モデルプラグイン
){
	char *tmp;
	DELETE_V(m_Effector);
	CHeadlightApplier headlightapplier;
	CParticleApplier particleapplier;
	CSoundApplier soundapplier;
	CEffectorSwitchApplier effectorswitchapplier;
	if(tmp = headlightapplier.Read(str, mpi)){
		str = tmp;
		m_Effector = new CHeadlightApplier(headlightapplier);
	}else if(tmp = particleapplier.Read(str, mpi)){
		str = tmp;
		m_Effector = new CParticleApplier(particleapplier);
	}else if(tmp = soundapplier.Read(str, mpi)){
		str = tmp;
		m_Effector = new CSoundApplier(soundapplier);
	}else if(tmp = effectorswitchapplier.Read(str, mpi)){
		str = tmp;
		m_Effector = new CEffectorSwitchApplier(effectorswitchapplier);
	}else{
		str = NULL;
	}
	return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 *	読込
 */
char *CHeadlightApplier::Read(
	char *str,			//	対象文字列
	CModelPlugin *mpi	//	モデルプラグイン
){
	if(!(str = g_TempHeadlight.Read(str, mpi))) return NULL;
	m_Headlight = mpi->AddHeadlight(g_TempHeadlight);
	return str;
}

/*
 *	データ読込
 */
void CHeadlightApplier::LoadDataEffector(
	CModelPlugin *mpi	//	呼び出し元
){
	m_Headlight->LoadData();
}

/*
 *	変更子適用
 */
void CHeadlightApplier::ApplyEffector(
	CScene *scene	//	シーン
){
	if(!g_PreSimulationFlag && (!scene || scene==g_Scene)) m_Headlight->Register();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 *	読込
 */
char *CParticleApplier::Read(
	char *str,			//	対象文字列
	CModelPlugin *mpi	//	モデルプラグイン
){
	if(!(str = g_TempParticle.Read(str, mpi))) return NULL;
	m_Particle = mpi->AddParticle(g_TempParticle);
	return str;
}

/*
 *	データ読込
 */
void CParticleApplier::LoadDataEffector(
	CModelPlugin *mpi	//	呼び出し元
){
	m_Particle->LoadData();
}

/*
 *	変更子適用
 */
void CParticleApplier::ApplyEffector(
	CScene *scene	//	シーン
){
	if(g_PreSimulationFlag) m_Particle->Register(scene);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 *	読込
 */
char *CSoundApplier::Read(
	char *str,			//	対象文字列
	CModelPlugin *mpi	//	モデルプラグイン
){
	if(!(str = g_TempSoundEffector.Read(str, mpi))) return NULL;
	m_SoundEffector = mpi->AddSoundEffector(g_TempSoundEffector);
	return str;
}

/*
 *	変更子適用
 */
void CSoundApplier::ApplyEffector(
	CScene *scene	//	シーン
){
	if(!g_PreSimulationFlag) m_SoundEffector->Register(scene);
}
