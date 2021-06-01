#pragma once

struct RecvTable;
struct RecvProp;

namespace NetVarManager {

	extern std::vector<std::pair<uint32_t, uint16_t>> offsets;

	void Initialize();
	void WalkTable(const char* name, RecvTable* recvTable, size_t base_offset = 0);
	uint16_t GetOffset(uint32_t hash);

};

#define NETVAR_OFFSET(funcname, class_name, var_name, offset, type) \
std::add_lvalue_reference_t<type> funcname() noexcept \
{ \
    constexpr auto hash = fnv::hash(class_name "->" var_name); \
	static auto base_offset = NetVarManager::GetOffset(hash); \
    return *(std::add_pointer_t<type>)((uintptr_t)this + base_offset + offset); \
}

#define NETVAR(funcname, class_name, var_name, type) \
    NETVAR_OFFSET(funcname, class_name, var_name, 0, type)

