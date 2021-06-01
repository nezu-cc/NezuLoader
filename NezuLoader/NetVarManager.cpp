#include "pch.h"
#include "NetVarManager.h"

std::vector<std::pair<uint32_t, uint16_t>> NetVarManager::offsets;

void NetVarManager::Initialize() {
	ClientClass* clientClass = I::Client->GetAllClasses();
	if (!clientClass)
		return;
	while (clientClass) {
		WalkTable(clientClass->m_pNetworkName, clientClass->m_pRecvTable);
		clientClass = clientClass->m_pNext;
	}
}

//pasted from osiris
void NetVarManager::WalkTable(const char* name, RecvTable* recvTable, size_t base_offset) {
	for (int i = 0; i < recvTable->m_nProps; ++i) {
		auto& prop = recvTable->m_pProps[i];

		if (std::isdigit(prop.m_pVarName[0]))
			continue;

		if (fnv::hashRuntime(prop.m_pVarName) == fnv::hash("baseclass"))
			continue;

		if (prop.m_RecvType == 6
			&& prop.m_pDataTable
			&& prop.m_pDataTable->m_pNetTableName[0] == 'D')
			WalkTable(name, prop.m_pDataTable, prop.m_Offset + base_offset);

		uint32_t hash = fnv::hashRuntime((name + std::string{ "->" } + prop.m_pVarName).c_str());

		offsets.emplace_back(hash, (uint16_t)(base_offset + prop.m_Offset));

	}
}

uint16_t NetVarManager::GetOffset(uint32_t hash) {
	const auto it = std::find_if(offsets.begin(), offsets.end(), [hash](const auto& p) { return p.first == hash; });
	if (it != offsets.end())
		return it->second;
	assert(false);
	return 0;
}

