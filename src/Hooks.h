#pragma once

namespace TKDodge
{
	class IsGhostHook
	{
	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> ProcessHitFunc{ REL::ID(37673) };
			_IsGhost = trampoline.write_call<5>(ProcessHitFunc.address() + 0x45, IsGhost);

			REL::Relocation<std::uintptr_t> NPCInvulnerableBase{ REL::ID(36715) };
			_IsGhost = trampoline.write_call<5>(NPCInvulnerableBase.address() + 0x2A, IsGhost);

			REL::Relocation<std::uintptr_t> PCInvulnerableBase{ REL::ID(39428) };
			_IsGhost = trampoline.write_call<5>(PCInvulnerableBase.address() + 0x14, IsGhost);
		}

	private:
		static bool IsGhost(const RE::Actor* a_actor)
		{
			logger::debug("IsGhost Trigger!");

			bool iframe = false, isDodge = false;

			if (a_actor->GetGraphVariableBool("bDodgeIframe", iframe) && iframe && a_actor->GetGraphVariableBool("bIsDodging", isDodge) && isDodge) {
				logger::debug("Actor is invulnerable!");
				return true;
			}

			return _IsGhost(a_actor);
		}

		static inline REL::Relocation<decltype(IsGhost)> _IsGhost;
	};
}