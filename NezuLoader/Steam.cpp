#include "pch.h"
#include "Hooks.h"

f_SendMessage H::oSendMessage;
f_RetrieveMessage H::oRetrieveMessage;

EGCResults __fastcall H::Hooked_SendMessage(ISteamGameCoordinator* thisPtr, void* edx, uint32_t unMsgType, const void* pubData, uint32_t cubData) {
	EGCResults status = oSendMessage(thisPtr, unMsgType, pubData, cubData);

	if (status != k_EGCResultOK)
		return status;
	
	uint32_t message_type = unMsgType & 0x7FFFFFFF;

#ifdef _DEBUG
	//L::Debug("[<-] Message sent to GC [%d]!\n", message_type);
	printf("[<-] Message sent to GC [%d]!\n", message_type);
	hexDump((void*)pubData, cubData);
#endif // _DEBUG

	return status;
}

EGCResults __fastcall H::Hooked_RetrieveMessage(ISteamGameCoordinator* thisPtr, void* edx, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize) {

	EGCResults status = oRetrieveMessage(thisPtr, punMsgType, pubDest, cubDest, pcubMsgSize);

	if (status != k_EGCResultOK)
		return status;

	int msg_type = *punMsgType & 0x7FFFFFFF;
	if (*pcubMsgSize > 8)
		GC::ParseIncommingMessage(msg_type, (void*)((uintptr_t)pubDest + 8), *pcubMsgSize - 8);

#ifdef _DEBUG
	//L::Debug("[->] Message got from GC [%d]!\n", msg_type);
	printf("[->] Message got from GC [%d]!\n", msg_type);
	hexDump(pubDest, *pcubMsgSize);
#endif // _DEBUG

	return status;
}