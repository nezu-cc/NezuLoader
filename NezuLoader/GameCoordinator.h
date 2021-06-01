#pragma once
namespace GC {

	extern bool invite_all;
	void InviteAll();
	bool ParseIncommingMessage(int msgtype, void* data, int size);

};

