#include "pch.h"
#include "Hooks.h"

f_InitializeNewParticles H::oCParticleCollection_InitializeNewParticles;

void __fastcall H::Hooked_CParticleCollection_InitializeNewParticles(CParticleCollection* thisPtr, void* edx, int nFirstParticle, int nParticleCount, uint32_t nInittedMask, bool bApplyingParentKillList) {
	oCParticleCollection_InitializeNewParticles(thisPtr, nFirstParticle, nParticleCount, nInittedMask, bApplyingParentKillList);
	//unused
}

f_Simulate H::oCParticleCollection_Simulate;

// the dt argument is passed in xmm1 for some reason (fuck you valve)
// we just rely on the fact that it won't change if we dont do any floating point math in between function start and the call to the original
void __fastcall H::Hooked_CParticleCollection_Simulate(CParticleCollection* thisPtr, void* edx/*, float dt*/) {
	// Dont't do FPU operations here!
	// if you do you have to backup and restore the xmm1 register before calling the original
	oCParticleCollection_Simulate(thisPtr/*, dt*/);
	
	if (Cfg::c.visuals.flame_rainbow) {
		CParticleCollection* root_colection = thisPtr;
		while (root_colection->m_pParent)
			root_colection = root_colection->m_pParent;

		const char* root_name = root_colection->m_pDef.m_pObject->m_Name.buffer;

		switch (fnv::hashRuntime(root_name))
		{
		case fnv::hash("molotov_groundfire"):
		case fnv::hash("molotov_groundfire_00MEDIUM"):
		case fnv::hash("molotov_groundfire_00HIGH"):
		case fnv::hash("molotov_groundfire_fallback"):
		case fnv::hash("molotov_groundfire_fallback2"):
		case fnv::hash("molotov_explosion"):
		case fnv::hash("explosion_molotov_air"):
		case fnv::hash("extinguish_fire"):
		case fnv::hash("weapon_molotov_held"):
		case fnv::hash("weapon_molotov_fp"):
		case fnv::hash("weapon_molotov_thrown"):
		case fnv::hash("incgrenade_thrown_trail"):
			switch (fnv::hashRuntime(thisPtr->m_pDef.m_pObject->m_Name.buffer))
			{
			case fnv::hash("explosion_molotov_air_smoke"):
			case fnv::hash("molotov_smoking_ground_child01"):
			case fnv::hash("molotov_smoking_ground_child02"):
			case fnv::hash("molotov_smoking_ground_child02_cheapo"):
			case fnv::hash("molotov_smoking_ground_child03"):
			case fnv::hash("molotov_smoking_ground_child03_cheapo"):
			case fnv::hash("molotov_smoke_screen"):
				if (Cfg::c.visuals.flame_no_smoke) {
					for (int i = 0; i < thisPtr->m_nActiveParticles; i++) {
						float* pColor = thisPtr->m_ParticleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
						*pColor = 0.f;
					}
				}
				break;
			default:
				for (int i = 0; i < thisPtr->m_nActiveParticles; i++) {
					float* pColor = thisPtr->m_ParticleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
					ImGui::ColorConvertHSVtoRGB(fmodf((float)ImGui::GetTime() / 2, 1), 1, 1, pColor[0], pColor[4], pColor[8]);
				}
				break;
			}
			break;
		//default:
			//printf("%s\n", root_name);
		}
	}

}