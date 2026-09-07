//	Copyright (c) 2002 Midikyou

#include "headers.h"
#include "debug.h"
#include "window.h"
#include "graphic.h"
#include "input.h"
#include "rs2_input.h"

void InputPollOnce();

int g_InputPollCount = 0;
CRITICAL_SECTION g_InputPollCriticalSection;

/*
 *	DirectInput no longer owns devices. Init wires the port backend
 *	(stub in check; SDL2 when RS2_HAVE_SDL2) and keeps ScanInputDevice() as the
 *	per-frame merge. The Win32 poll thread is omitted here: the
 *	process.h _beginthreadex stub runs the start routine inline, and
 *	ScanInputDevice already calls InputPollOnce when the count is 0.
 */
BOOL InitDirectInput(){
	DebugHL();
	Debug("InitDirectInput\n");

	rs2_input_backend_reset();
	if(!InitKeyboard()) return FALSE;
	if(!InitMouse()) return FALSE;
	if(!InitJoyStick()) return FALSE;
	FlushInputDevice();
	InitializeCriticalSection(&g_InputPollCriticalSection);
	return TRUE;
}

void FreeDirectInput(){
	DebugHL();
	Debug("FreeInput\n");

	DeleteCriticalSection(&g_InputPollCriticalSection);
	FreeJoyStick();
	FreeMouse();
	FreeKeyboard();
}

BOOL InitKeyboard(){
	return TRUE;
}

void FreeKeyboard(){
}

BOOL InitMouse(){
	if(!sv3.fWindowed){
		ShowCursor(FALSE);
		SetCursor(svw.winW/2, svw.winH/2);
	}
	return TRUE;
}

void FreeMouse(){
}

BOOL InitJoyStick(){
	svi.numJoy = 0;
	return TRUE;
}

void FreeJoyStick(){
}

BOOL CALLBACK EnumJoyCallback(const DIDEVICEINSTANCE *pInst, VOID *pContext){
	(void)pInst;
	(void)pContext;
	return 0;
}

BOOL CALLBACK EnumAxisCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef){
	(void)lpddoi;
	(void)pvRef;
	return 0;
}

void ScanInputDevice(){
	EnterCriticalSection(&g_InputPollCriticalSection);

	if(!g_InputPollCount){
		InputPollOnce();
	}
	g_InputPollCount = 0;

	ScanKeyboard();
	ScanMouse();
	ScanJoyStick();

	LeaveCriticalSection(&g_InputPollCriticalSection);
}

void FlushInputDevice(){
	rs2_input_flush(
		svi.key, svi.keyPoll, svi.keyOld,
		svi.btn, svi.btnPoll, svi.btnOld,
		svi.joy, svi.joyOld,
		&svi.wheel, &svi.wheelPoll);
}

void InputPollOnce(){
	rs2_input_poll_once(svi.keyPoll, svi.btnPoll, &svi.wheelPoll);
	++g_InputPollCount;
}

void ScanKeyboard(){
	rs2_input_scan_keyboard(svi.key, svi.keyOld, svi.keyPoll);
}

POINT GetCursorPosClient(){
	POINT cur;
	int x = 0;
	int y = 0;
	rs2_input_backend_get_cursor(&x, &y);
	cur.x = x;
	cur.y = y;
	return cur;
}

void ScanMouse(){
	int x = 0;
	int y = 0;
	int inside = 0;
	rs2_input_scan_mouse(
		svi.btn, svi.btnOld, svi.btnPoll,
		&svi.wheel, &svi.wheelPoll,
		&x, &y, &inside,
		svw.winW, svw.winH, sv3.fWindowed);
	svi.cur.x = x;
	svi.cur.y = y;
	svi.inside = inside ? TRUE : FALSE;
}

void ScanJoyStick(){
	rs2_input_scan_joystick(svi.joy, svi.joyOld, svi.numJoy, svi.fJoy);
}

int GetKey(int id){
	return rs2_input_edge(svi.key[id], svi.keyOld[id]);
}

int CheckKeyDown(){
	int i, s, t = -1;
	for(i = 0; i<256; i++){
		s = GetKey(i);
		if(s==S_PUSH) return i;
		if(t<0 && s>S_PUSH) t = i;
	}
	return t;
}

void FlushKey(){
	memset(svi.key, 0, sizeof(svi.key));
}

int DequeueChar(){
	if(!svi.charKey.size()) return 0;
	int key = *svi.charKey.begin();
	svi.charKey.pop_front();
	return key;
}

void OnChar(WPARAM wParam){
	svi.charKey.push_back((TCHAR)wParam);
}

void SetExitKey(int id){
	svi.exitKey = id;
}

int GetButton(int id){
	return rs2_input_edge(svi.btn[id], svi.btnOld[id]);
}

POINT GetCursorXY(){
	return svi.cur;
}

int GetCursorX(){
	return svi.cur.x;
}

int GetCursorY(){
	return svi.cur.y;
}

BOOL IsCursorInside(){
	return svi.inside;
}

LONG GetWheel(){
	return svi.wheel;
}

void SetCursor(int x, int y){
	rs2_input_backend_set_cursor(x, y);
}

void EnableJoyStick(BOOL f){
	svi.fJoy = f;

	if(!f) memset(svi.joy, FALSE, sizeof(svi.joy));
}

int GetJoy(int n, int id){
	return rs2_input_edge(svi.joy[n][id], svi.joyOld[n][id]);
}

void MsgBox(char *msg){
	MessageBox(svw.hWnd, msg, "", MB_OK);
	FlushInputDevice();
}

int MsgYesNo(char *msg){
	int ret = MessageBox(
		svw.hWnd, msg, "", MB_YESNO|MB_SYSTEMMODAL|MB_ICONQUESTION);
	FlushInputDevice();
	return ret==IDYES;
}
