#include "pch.h"
#include "Hooks.h"

f_GetEconGameAccountClient H::oGetEconGameAccountClient;

CEconGameAccountClient* __fastcall H::Hooked_GetEconGameAccountClient(void* thisPtr, void* edx) {
	CEconGameAccountClient* client = oGetEconGameAccountClient(thisPtr);
	if (Cfg::c.lobby.fake.fake_prime)
		client->elevated_state = 5;
	return client;
}

f_IsPrime H::oIsPrime;

bool H::Hooked_IsPrime() {
	return Cfg::c.lobby.fake.fake_prime || oIsPrime();
}