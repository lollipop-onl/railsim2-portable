#include <windows.h>

// Not extern "C": under port/stub WINAPI is empty, so lib/main.cpp defines
// WinMain with C++ linkage and only this exact prototype mangles to it.
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT);

int main() {
  char cmdLine[] = "";
  return WinMain(nullptr, nullptr, cmdLine, SW_SHOW);
}
