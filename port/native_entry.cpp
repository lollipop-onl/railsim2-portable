// POSIX process entry. Upstream lib/main.cpp exposes WinMain only.
// This file is always linked on the native target so the check preset
// produces an executable (game startup is still WinMain / later M3).

int main() { return 0; }
