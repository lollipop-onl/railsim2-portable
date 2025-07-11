#ifndef CEFFECTOR_H_INCLUDED
#define CEFFECTOR_H_INCLUDED

class CHeadlight;
class CParticle;
class CSoundEffector;
class CModelPlugin;
class CScene;

//	タイプフラグ
const int EFCT_MISCELLANEOUS = 0x0001;
const int EFCT_PARTICLEAPPLIER = 0x0002;

/*
 *	エフェクタ基本クラス
 */
class CEffectorBase{
protected:
public:
	virtual ~CEffectorBase(){}
	virtual CEffectorBase *Duplicate() = 0;
	virtual void LoadDataEffector(CModelPlugin *){}
	virtual int GetTypeFlagEffector(){ return EFCT_MISCELLANEOUS; }
	virtual void ApplyEffector(CScene *){}
};

/*
 *	エフェクタコンテナ
 */
class CEffectorContainer{
private:
	CEffectorBase *m_Effector;	//	エフェクタ
public:
	CEffectorContainer();
	CEffectorContainer(const CEffectorContainer &);
	~CEffectorContainer();
	char *Read(char *, CModelPlugin *);
	void LoadData(CModelPlugin *mpi){ m_Effector->LoadDataEffector(mpi); }
	int GetTypeFlag(){ return m_Effector->GetTypeFlagEffector(); }
	void Apply(CScene *scene){ m_Effector->ApplyEffector(scene); }
};

//	反復子
typedef list<CEffectorContainer>::iterator IEffectorContainer;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 *	ヘッドライト適用子
 */
class CHeadlightApplier: public CEffectorBase{
private:
	CHeadlight *m_Headlight;	//	ヘッドライト
public:
	CHeadlightApplier(){ m_Headlight = NULL; }
	CEffectorBase *Duplicate(){ return new CHeadlightApplier(*this); }
	char *Read(char *, CModelPlugin *);
	void LoadDataEffector(CModelPlugin *);
	void ApplyEffector(CScene *);
};

/*
 *	パーティクル適用子
 */
class CParticleApplier: public CEffectorBase{
private:
	CParticle *m_Particle;	//	パーティクル
public:
	CParticleApplier(){ m_Particle = NULL; }
	CEffectorBase *Duplicate(){ return new CParticleApplier(*this); }
	char *Read(char *, CModelPlugin *);
	void LoadDataEffector(CModelPlugin *);
	int GetTypeFlagEffector(){ return EFCT_PARTICLEAPPLIER; }
	void ApplyEffector(CScene *);
};

/*
 *	サウンド適用子
 */
class CSoundApplier: public CEffectorBase{
private:
	CSoundEffector *m_SoundEffector;	//	サウンドエフェクタ
public:
	CSoundApplier(){ m_SoundEffector = NULL; }
	CEffectorBase *Duplicate(){ return new CSoundApplier(*this); }
	char *Read(char *, CModelPlugin *);
	void ApplyEffector(CScene *);
};

#endif
