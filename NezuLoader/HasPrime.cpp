#include "pch.h"
#include "Hooks.h"

f_HasPrime H::oHasPrime;

bool H::Hooked_HasPrime() {
	if (Cfg::c.lobby.fake.fake_prime)
		return true;

	return oHasPrime();
}