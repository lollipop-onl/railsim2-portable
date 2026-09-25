// Win32 message queue behind port/stub/windows.h (#205, parent #8).
// See docs/porting/input-seams.md. PeekMessage / GetMessage / DispatchMessage
// / PostMessage / SendMessage / PostQuitMessage / RegisterClassEx /
// CreateWindowEx / GetClientRect / DestroyWindow / WaitMessage / Sleep are
// defined in port/rs2_msg.cpp; this header adds the port-only surface.
//
// One queue for the whole process: the game runs on one thread.

#pragma once

#include <windows.h>

// --- backend hooks (stub in rs2_msg.cpp; SDL2 in rs2_msg_sdl.cpp) ---

// Called at the start of every PeekMessage / GetMessage / WaitMessage.
// Translates backend window events into Win32 messages with PostMessage /
// rs2_msg_set_client_size. Must not consume keyboard, mouse or wheel events:
// rs2_input_sdl.cpp reads those.
void rs2_msg_backend_pump();

// Called by GetMessage / WaitMessage when the queue is empty after a pump.
// Returns false when no message can ever arrive (the check stub), so
// GetMessage fails instead of hanging. May return after a bounded wait
// without anything having arrived. Nothing polls input while the game
// waits, so the SDL backend discards keyboard, mouse and wheel events here.
bool rs2_msg_backend_wait();

// --- port-only queries ---

// Oldest window CreateWindowEx returned that has not been destroyed, or
// null. The backend addresses its window events to it.
HWND rs2_msg_main_window();

// Record a new client size for GetClientRect. Does not post WM_SIZE.
// Returns FALSE for an unknown window.
BOOL rs2_msg_set_client_size(HWND hwnd, int width, int height);

// Drop every class, window, queued message and pending WM_QUIT (ctest).
void rs2_msg_reset();
