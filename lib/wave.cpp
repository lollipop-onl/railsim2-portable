//	Copyright (c) 2002 Midikyou

#include "headers.h"
#include "debug.h"
#include "window.h"
#include "sound.h"
#include "wave.h"
#include "wav_pcm.h"

#include <cstring>

#ifndef DSBUFFERDESC
typedef struct _DSBUFFERDESC {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwBufferBytes;
	DWORD dwReserved;
	LPWAVEFORMATEX lpwfxFormat;
	GUID guid3DAlgorithm;
} DSBUFFERDESC;
#endif
#ifndef DSBCAPS_CTRLVOLUME
#define DSBCAPS_CTRLVOLUME 0x00000080
#endif
#ifndef DSBCAPS_CTRL3D
#define DSBCAPS_CTRL3D 0x00000010
#endif
#ifndef DSERR_BUFFERTOOSMALL
#define DSERR_BUFFERTOOSMALL ((HRESULT)0x8878004AL)
#endif
#ifndef DSERR_OUTOFMEMORY
#define DSERR_OUTOFMEMORY ((HRESULT)0x00000007L)
#endif
#ifndef DSERR_BUFFERLOST
#define DSERR_BUFFERLOST ((HRESULT)0x88780096L)
#endif
#ifndef DS_OK
#define DS_OK S_OK
#endif
#ifndef DSBPLAY_LOOPING
#define DSBPLAY_LOOPING 0x00000001
#endif
#ifndef DSBSTATUS_PLAYING
#define DSBSTATUS_PLAYING 0x00000001
#endif
#ifndef IID_IDirectSoundBuffer8
static const GUID IID_IDirectSoundBuffer8 = {0,0,0,{0,0,0,0,0,0,0,0}};
#endif
#ifndef IID_IDirectSound3DBuffer
static const GUID IID_IDirectSound3DBuffer = {0,0,0,{0,0,0,0,0,0,0,0}};
#endif

/*
 *	コンストラクタ
 *
 *	※プログラム終了までにFree()またはdelete(newで確保した場合)して下さい。
 */
CWave::CWave(){
	m_pSB = NULL;
	m_p3D = NULL;
	m_pFX = NULL;
	m_BytesPerSec = 0;
	m_nChannels = 0;
	m_wBitsPerSample = 0;
}

/*
 *	デストラクタ
 */
CWave::~CWave(){
	Free();
}

/*
 *	音声データの読込み
 *
 *	strFile	: ファイル名
 */
BOOL CWave::Load(char *strFile){
	//	existing buffer
	if(m_pSB) Free();

	Debug("load(%s) ... ", strFile);
	char full[_MAX_PATH];
	_fullpath(full, strFile, _MAX_PATH);
	m_strName = full;

	Rs2WavPcm wav;
	std::string err;
	if(!rs2_wav_pcm_parse_file(strFile, &wav, &err)){
		if(err.find("cannot open") != std::string::npos ||
		   err.find("null path") != std::string::npos){
			Debug("open error.\n");
		}else if(err.find("not a RIFF") != std::string::npos){
			Debug("RIFF is not found.\n");
		}else if(err.find("fmt is not found") != std::string::npos){
			Debug("fmt is not found.\n");
		}else if(err.find("PCM") != std::string::npos){
			Debug("is not PCM format.\n");
		}else if(err.find("data is not found") != std::string::npos){
			Debug("chunk is not found.\n");
		}else{
			Debug("open error.\n");
		}
		return FALSE;
	}

	m_BytesPerSec = wav.nAvgBytesPerSec;
	m_nChannels = wav.nChannels;
	m_wBitsPerSample = wav.wBitsPerSample;
	m_pcm = wav.pcm;

	WAVEFORMATEX wfmtx;
	memset(&wfmtx, 0, sizeof(wfmtx));
	wfmtx.wFormatTag = WAVE_FORMAT_PCM;
	wfmtx.nChannels = wav.nChannels;
	wfmtx.nSamplesPerSec = wav.nSamplesPerSec;
	wfmtx.nAvgBytesPerSec = wav.nAvgBytesPerSec;
	wfmtx.nBlockAlign = wav.nBlockAlign;
	wfmtx.wBitsPerSample = wav.wBitsPerSample;
	wfmtx.cbSize = 0;

	if(!CreateBuffer(&wfmtx, (DWORD)m_pcm.size())) return FALSE;

	Debug("ok.\n");
	return TRUE;
}

/*
 *	別のCWaveオブジェクトからデータを複製する
 *
 *	pWav	: コピー元のオブジェクト
 */
BOOL CWave::Duplicate(CWave *pWav){
	if(!svs.pDS || m_pSB) Free();

	m_strName = pWav->m_strName;
	m_BytesPerSec = pWav->m_BytesPerSec;
	m_nChannels = pWav->m_nChannels;
	m_wBitsPerSample = pWav->m_wBitsPerSample;
	m_pcm = pWav->m_pcm;

	WAVEFORMATEX wfmtx;
	memset(&wfmtx, 0, sizeof(wfmtx));
	wfmtx.wFormatTag = WAVE_FORMAT_PCM;
	wfmtx.nChannels = m_nChannels;
	wfmtx.nSamplesPerSec = (m_nChannels && m_wBitsPerSample)
		? m_BytesPerSec / ((m_wBitsPerSample/8)*m_nChannels) : 0;
	wfmtx.nAvgBytesPerSec = m_BytesPerSec;
	wfmtx.nBlockAlign = (WORD)((m_wBitsPerSample/8)*m_nChannels);
	wfmtx.wBitsPerSample = m_wBitsPerSample;
	wfmtx.cbSize = 0;

	return CreateBuffer(&wfmtx, (DWORD)m_pcm.size());
}

/*
 *	サウンドバッファの作成
 *
 *	hMMMI	: ファイルハンドラ
 *	pFmt	: ウエーブフォーマット
 *	len	: ウエーブサイズ
 */
BOOL CWave::CreateBuffer(LPWAVEFORMATEX pFmt, DWORD len){
	if(!svs.pDS) return FALSE;
	//	バッファの作成
	DSBUFFERDESC desc;

	memset(&desc, 0, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_CTRLVOLUME;
	desc.dwBufferBytes = len;
	desc.lpwfxFormat = pFmt;

	if(svs.f3D) desc.dwFlags |= DSBCAPS_CTRL3D;
//	if(svs.fFX) desc.dwFlags |= DSBCAPS_CTRLFX;

	HRESULT hr = svs.pDS->CreateSoundBuffer(&desc, &m_pSB, NULL);

	if(FAILED(hr)){
		if(hr==DSERR_BUFFERTOOSMALL){
			//	データが短くてエフェクトを使用できない
			Debug("data is too small.\n");
			return TRUE;
		}else if(hr = DSERR_OUTOFMEMORY){
			//	メモリ不足（サウンドカード内の？）
			Debug("out of memory.\n");
			return FALSE;
		}else{
			Debug("can't create buffer.\n");
			return FALSE;
		}
	}
	//	バッファのロック
	LPVOID write1, write2;
	DWORD length1, length2;

	if(m_pSB->Lock(0, len, &write1, &length1, &write2, &length2, 0)==DSERR_BUFFERLOST){
#if !defined(RS2_PORTABLE_COMPILE_FIREWALL)
		m_pSB->Restore();
#endif
		return FALSE;
	}
	//	PCM payload into the locked buffer
	if(write1 && length1){
		DWORD n = length1;
		if(n > (DWORD)m_pcm.size()) n = (DWORD)m_pcm.size();
		if(n) memcpy(write1, m_pcm.data(), n);
	}
	if(write2 && length2){
		DWORD off = length1;
		DWORD n = length2;
		if(off < (DWORD)m_pcm.size()){
			if(off + n > (DWORD)m_pcm.size()) n = (DWORD)m_pcm.size() - off;
			memcpy(write2, m_pcm.data() + off, n);
		}
	}
	//	バッファのアンロック
	if(m_pSB->Unlock(write1, length1, write2, length2)!=DS_OK)
		return FALSE;

	return Query();
}

/*
 *	インターフェイスの取得
 */
BOOL CWave::Query(){
	//	FXバッファの取得
	if(svs.fFX){
		if(FAILED(m_pSB->QueryInterface(IID_IDirectSoundBuffer8, (void **)&m_pFX))){
			Debug("DirectSoundBuffer8\n");
			return FALSE;
		}
	}
	//	3Dバッファの取得
	if(svs.f3D){
		if(FAILED(m_pSB->QueryInterface(IID_IDirectSound3DBuffer, (void **)&m_p3D))){
			Debug("DirectSound3DBuffer\n");
			return FALSE;
		}
	}
	return TRUE;
}

/*
 *	バッファの解放
 */
void CWave::Free(){
	if(m_strName!=""){
		Debug("release(%s)\n", m_strName.c_str());
		m_strName = "";
	}
	Stop();
	RELEASE(m_p3D);
	RELEASE(m_pFX);
	RELEASE(m_pSB);
	m_pcm.clear();
	m_nChannels = 0;
	m_wBitsPerSample = 0;
	m_BytesPerSec = 0;
}

/*
 *	再生
 *
 *	ms	: 再生開始位置 [ms] (<0 でループ)
 */
void CWave::Play(int ms){
	if(!m_pSB) return;

	m_pSB->Stop();
#if !defined(RS2_PORTABLE_COMPILE_FIREWALL)
	m_pSB->SetCurrentPosition(ms<0 ? 0 : m_BytesPerSec*ms/1000);
#endif
	HRESULT hr = m_pSB->Play(0, 0, ms<0 ? DSBPLAY_LOOPING : 0);
	//	リロード処理
	if(hr==DSERR_BUFFERLOST){
		Debug("DSERR_BUFFERLOST:%s", m_strName.c_str());

		PrimaryBufferVerify();	//	先にプライマリバッファを検証する
#if !defined(RS2_PORTABLE_COMPILE_FIREWALL)
		m_pSB->Restore();	//	どの道Free()するが念のため
#endif
		Load((char *)m_strName.c_str());
	}
}

/*
 *	再生状態の取得
 */
BOOL CWave::GetStatus(){
	if(!m_pSB) return FALSE;

	DWORD dw;
	m_pSB->GetStatus(&dw);

	if(dw&DSBSTATUS_PLAYING) return TRUE;
	else return FALSE;
}

/*
 *	エフェクトの設定
 *
 *	fx	: エフェクトタイプ（FX_～）
 *
 *	※16bit以外のWAVEファイルはエフェクトが正常にかからない場合がある。
 */
void CWave::SetFX(int fx){
	if(!svs.fFX || !m_pFX) return;
#if defined(RS2_PORTABLE_COMPILE_FIREWALL)
	(void)fx;
#else

	//	エフェクトのリセット
	if(fx==FX_DRY){
		m_pFX->SetFX(0, NULL, NULL);
		return;
	}

	DSEFFECTDESC desc;
	DWORD rc = 0;

	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = 0;

	switch(fx){
	case FX_REVERB		: desc.guidDSFXClass = GUID_DSFX_WAVES_REVERB;			break;
	case FX_REVERB3D	: desc.guidDSFXClass = GUID_DSFX_STANDARD_I3DL2REVERB;	break;
	case FX_ECHO		: desc.guidDSFXClass = GUID_DSFX_STANDARD_ECHO;			break;
	case FX_CHORUS		: desc.guidDSFXClass = GUID_DSFX_STANDARD_CHORUS;		break;
	case FX_FLANGER		: desc.guidDSFXClass = GUID_DSFX_STANDARD_FLANGER;		break;
	case FX_GARGLE		: desc.guidDSFXClass = GUID_DSFX_STANDARD_GARGLE;		break;
	case FX_COMP		: desc.guidDSFXClass = GUID_DSFX_STANDARD_COMPRESSOR;	break;
	case FX_PARAMEQ		: desc.guidDSFXClass = GUID_DSFX_STANDARD_PARAMEQ;		break;
	case FX_DISTORTION	: desc.guidDSFXClass = GUID_DSFX_STANDARD_DISTORTION;	break;
	default				: SetFX(FX_DRY); return;
	}
	m_pFX->SetFX(1, &desc, &rc);
#endif
}
