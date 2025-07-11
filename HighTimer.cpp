#include <time.h>
#include <windows.h>

LONGLONG g_Freq = 0;

/*
 *	高分解能タイマの初期化
 *
 *	戻り値: CPU が対応していれば true を返す
 */
bool InitHighTimer(){
	if(QueryPerformanceFrequency((LARGE_INTEGER *)(&g_Freq))){
		return true;
	}else{
		g_Freq = 0;
		return false;
	}
}

/*
 *	現在時刻の取得
 *
 *	戻り値: ミリ秒
 */
double HighTimer(){
	if(g_Freq){
		LONGLONG cnt;
		QueryPerformanceCounter((LARGE_INTEGER *)(&cnt));
		return (double)cnt/g_Freq*1000.0;
	}else{
		return (double)clock();	//	非対応 orz
	}
}
