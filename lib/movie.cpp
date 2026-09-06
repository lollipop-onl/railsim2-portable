//	Copyright (c) 2002 Midikyou

#include "headers.h"
#include <dshow.h>
#include "debug.h"
#include "window.h"
#include "graphic.h"
#include "movie.h"

/*
 *	DirectShowの初期化
 */
BOOL InitDirectShow(){
	DebugHL();
	Debug("InitDirectShow\n");

	svv.pGraph = NULL;
	svv.pMCtrl = NULL;
	svv.pEvent = NULL;
	svv.pVWin = NULL;
	svv.fPlay = FALSE;
	return TRUE;
}

void FreeDirectShow(){
	DebugHL();
	Debug("FreeDirectShow\n");

	StopMovie();
	RELEASE(svv.pEvent);
	RELEASE(svv.pMCtrl);
	RELEASE(svv.pGraph);
}

/*
 *	ムービーイベントに対するハンドラ
 */
void OnGrapNotify(){
	if(!svv.pEvent) return;	//	ムービー機能未使用時も呼ばれるためチェック

	long evCode, param1, param2;

	//	全イベントを取り出す
	while(SUCCEEDED(svv.pEvent->GetEvent(&evCode, &param1, &param2, 0))){ 
		//	イベントに関連するリソースを解放する
		svv.pEvent->FreeEventParams(evCode, param1, param2);

		if((EC_COMPLETE==evCode) || (EC_USERABORT==evCode)){
			StopMovie();
			break;
		} 
	} 
}

/*
 *	ムービー再生
 *
 *	strFile	: ファイル名
 *
 *	※再生中はシーンのレンダリングを止めること。
 */
BOOL PlayMovie(char *strFile){
	Debug("play(%s) ... ", strFile);
	Debug("can't render.\n");
	return FALSE;
}

void StopMovie(){
	svv.fPlay = FALSE;
	RELEASE(svv.pVWin);
}

BOOL GetMovieState(){
	return svv.fPlay;
}

/*
 *	ムービーレイヤーのサイズをクライアント領域に合わせる
 */
void AdjustMovieLayer(){
	if(!svv.pVWin) return;	//	ムービー機能未使用時も呼ばれるためチェック

	svv.pVWin->put_Owner((OAHWND)svw.hWnd);
	svv.pVWin->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS);
	svv.pVWin->SetWindowPosition(0, 0, svw.winW, svw.winH);
}
