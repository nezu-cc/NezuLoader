#include "pch.h"
#include "GameCoordinator.h"
#include "protobuf/messages.pb.h"

bool GC::invite_all = false;

void GC::InviteAll() {

	auto& settings = Cfg::c.lobby.auto_invite.mass_invite;

	uint32 game_type = 0;
	switch (settings.game_type)
	{
	case LobbyGameType::COMP:			game_type = 8;  break;
	case LobbyGameType::WIGMAN:			game_type = 10; break;
	case LobbyGameType::DANGER_ZONE:	game_type = 13; break;
	}
	if (game_type == 0) {
		L::Error("InviteAll Failed (invalid game_type)");
		return;
	}

	CMsgGCCStrike15_v2_Party_Search message = CMsgGCCStrike15_v2_Party_Search_init_zero;
	message.has_ver = true;
	message.has_game_type = true;
	message.has_apr = true;
	message.has_ark = true;
	message.has_launcher = true;

	message.ver = I::Engine->GetEngineBuildNumber();
	message.game_type = game_type;
	message.apr = settings.prime ? 1 : 0;
	message.ark = settings.avg_rank * 10;
	message.launcher = 0;

	uint8_t data[256];//should be plenty XD

	uint32_t msgtype = ECsgoGCMsg_k_EMsgGCCStrike15_v2_Party_Search | (1 << 31); 
	memcpy(data, &msgtype, sizeof(uint32_t));
	((uint32_t*)data)[1] = 0;

	pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t*)((uintptr_t)data + 8), sizeof(data) - 8);
	if (!pb_encode(&stream, CMsgGCCStrike15_v2_Party_Search_fields, &message)) {
		L::Error("Faild to encode CMsgGCCStrike15_v2_Party_Search (%s)", PB_GET_ERROR(&stream));
		return;
	}

	invite_all = I::SteamGameCoordinator->SendMessage(msgtype, data, stream.bytes_written + 8) == k_EGCResultOK;

}

bool GC::ParseIncommingMessage(int msgtype, void* data, int size) {

	if (msgtype == ECsgoGCMsg_k_EMsgGCCStrike15_v2_Party_Search && invite_all) {
		pb_istream_t stream = pb_istream_from_buffer((pb_byte_t*)data, size);

		std::vector<CMsgGCCStrike15_v2_Party_SearchResults_Entry> entries;

		CMsgGCCStrike15_v2_Party_SearchResults message = CMsgGCCStrike15_v2_Party_SearchResults_init_zero;
		message.entries.funcs.decode = [](pb_istream_t* stream, const pb_field_t* field, void** arg) -> bool {
			auto entries = *(std::vector<CMsgGCCStrike15_v2_Party_SearchResults_Entry>**)arg;

			CMsgGCCStrike15_v2_Party_SearchResults_Entry entry = CMsgGCCStrike15_v2_Party_SearchResults_Entry_init_zero;
			if (!pb_decode(stream, CMsgGCCStrike15_v2_Party_SearchResults_Entry_fields, &entry))
				return false;

			entries->push_back(entry);
			return true;
		};
		message.entries.arg = &entries;

		if (!pb_decode(&stream, CMsgGCCStrike15_v2_Party_SearchResults_fields, &message)) {
			L::Error("Faild to decode CMsgGCCStrike15_v2_Party_SearchResults (%s)", PB_GET_ERROR(&stream));
			return false;
		}

		if (entries.size() > 0)
			CreateOnlineLobby();

		for (auto& entry : entries) {
			if (!entry.has_id)
				continue;
			CSteamID player;
			player.InstancedSet(entry.id, 1, k_EUniversePublic, k_EAccountTypeIndividual);
			invitePlayer(player.ConvertToUint64());
		}

		invite_all = false;
		return true;
	}

	if (msgtype == ECsgoGCMsg_k_EMsgGCCStrike15_v2_Party_Invite && Cfg::c.lobby.auto_invite.auto_join) {
		pb_istream_t stream = pb_istream_from_buffer((pb_byte_t*)data, size);

		CMsgGCCStrike15_v2_Party_Invite message = CMsgGCCStrike15_v2_Party_Invite_init_zero;
		if (!pb_decode(&stream, CMsgGCCStrike15_v2_Party_Invite_fields, &message)) {
			L::Error("Faild to decode CMsgGCCStrike15_v2_Party_Invite (%s)", PB_GET_ERROR(&stream));
			return false;
		}

		if (!message.has_lobbyid) {
			L::Warning("CMsgGCCStrike15_v2_Party_Invite dosn't contain lobbyid, ignoring");
			return false;
		}

		CSteamID lobby_id;
		lobby_id.InstancedSet(message.lobbyid, k_EChatInstanceFlagMMSLobby | k_EChatInstanceFlagLobby, k_EUniversePublic, k_EAccountTypeChat);
		uint64_t lobby_id_uint = lobby_id.ConvertToUint64();

		if (std::find_if(G::PendingInvites.begin(), G::PendingInvites.end(), 
			[&lobby_id_uint](const std::pair<uint64_t, uint64_t> x) { 
				return x.first == lobby_id_uint; 
			}) == G::PendingInvites.end()) {
			G::PendingInvites.push_back(std::make_pair(lobby_id_uint, (uint64_t)GetTickCount64()));
			L::Debug("invited to lobby: %llu\n", lobby_id_uint);
		}
		return true;
	}

	if (msgtype == ECsgoGCMsg_k_EMsgGCCStrike15_v2_MatchmakingGC2ClientUpdate && Cfg::c.lobby.auto_queue.enabled) {
		pb_istream_t stream = pb_istream_from_buffer((pb_byte_t*)data, size);

		CMsgGCCStrike15_v2_MatchmakingGC2ClientUpdate message = CMsgGCCStrike15_v2_MatchmakingGC2ClientUpdate_init_zero;

		std::string error = "";

		message.error.funcs.decode = [](pb_istream_t* stream, const pb_field_t* field, void** arg) -> bool {
			auto str = *(std::string**)arg;

			str->resize(stream->bytes_left);
			return pb_read(stream, (pb_byte_t*)str->data(), str->size());
		};
		message.error.arg = &error;

		if (!pb_decode(&stream, CMsgGCCStrike15_v2_MatchmakingGC2ClientUpdate_fields, &message)) {
			L::Error("Faild to decode CMsgGCCStrike15_v2_MatchmakingGC2ClientUpdate (%s)", PB_GET_ERROR(&stream));
			return false;
		}

		if (message.has_matchmaking) { //matchmaking stopped
			if (message.matchmaking == 0) {
				if (error != "Stopped by user") {
					Cfg::c.lobby.auto_queue.enabled = false;
					L::Info("Matchmaking error: \"%s\", disabling Auto queue", error.c_str());
				}
			}
			else if (message.matchmaking == 4) {//Match confirmed
				L::Debug("Match confirmed, disabling auto queue for next 20 seconds", error.c_str());
				G::NextAutoQueueCall = GetTickCount64() + 20000; //prevent auto queue from running for next 20 seconds if we found a match
			}
		}
		return true;
	}

	return false;
}