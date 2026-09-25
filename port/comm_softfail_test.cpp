// lib/comm.cpp when no DirectPlay8 peer can be created (#206).
// port/stub/windows.h answers CoCreateInstance with E_NOTIMPL, which is what
// every host without DirectPlay8 does. The game must still start, and every
// comm call must report failure instead of calling through a null svc.pDP.

#include "headers.h"
#include "debug.h"
#include "comm.h"

#include <cstdio>
#include <cstring>

SYSVALUE_C svc;

void Debug(LPCTSTR, ...) {}
void DebugHL() {}

namespace {

const GUID kApp = {};
int failures = 0;

void expect(bool ok, const char *spec) {
	if (ok) return;
	std::fprintf(stderr, "comm_softfail: FAIL %s\n", spec);
	++failures;
}

void no_peer_leaves_every_session_call_failing() {
	char payload[4] = {1, 2, 3, 4};

	expect(!CreateSession(&kApp, "RailSimNetPlay", 2501), "CreateSession fails without a peer");
	expect(!IsHost(), "a failed CreateSession does not make this player the host");
	expect(!JoinSession(&kApp, "192.168.1.1", 2501, 2502), "JoinSession fails without a peer");
	expect(!EnumHosts(&kApp, "192.168.1.1"), "EnumHosts fails without a peer");
	expect(!SendTo(1, payload, sizeof(payload)), "SendTo fails without a peer");
	expect(!SendToAll(payload, sizeof(payload)), "SendToAll fails without a peer");
	expect(!CloseSession(), "CloseSession reports that no session was closed");
	expect(svc.pDeviceAddress == NULL && svc.pHostAddress == NULL,
	       "no address object is created without a peer");
}

int self_test() {
	static char not_a_peer;
	svc.pDP = reinterpret_cast<IDirectPlay8Peer *>(&not_a_peer);

	expect(InitDirectPlay() == TRUE, "InitDirectPlay succeeds without a peer, so CApp::Init goes on");
	expect(svc.pDP == NULL, "InitDirectPlay leaves svc.pDP NULL when CoCreateInstance fails");

	no_peer_leaves_every_session_call_failing();

	FreeDirectPlay();
	FreeDirectPlay();
	expect(svc.pDP == NULL, "FreeDirectPlay without a peer can run any number of times");

	if (failures) {
		std::fprintf(stderr, "comm_softfail: %d check(s) failed\n", failures);
		return 1;
	}
	std::printf("comm_softfail: self-test ok\n");
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	std::fprintf(stderr, "usage: rs2_comm_softfail_test --self-test\n");
	return 2;
}
