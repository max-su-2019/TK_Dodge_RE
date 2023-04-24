#include "Hooks.h"
#include "TKRE.h"
#include "InputEvents.h"

namespace Hooks
{
	void install()
	{
		INFO("Hooking...");

		SprintHandlerHook::Hook();

		INFO("...success");
	}
	static bool bStoppingSprint = false;
	//static float heldDownTimeOffset = 0.f;

	void SprintHandlerHook::ProcessButton(RE::SprintHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data)
	{
		using FlagBDD = RE::PlayerCharacter::FlagBDD;

		if (Settings::EnableSprintKeyDodge) {
			auto playerCharacter = RE::PlayerCharacter::GetSingleton();
			auto userEvent = a_event->QUserEvent();
			auto userEvents = RE::UserEvents::GetSingleton();

			if (userEvent == userEvents->sprint) {
				if (a_event->IsDown() && (playerCharacter->unkBDD & FlagBDD::kSprinting) != FlagBDD::kNone) {  // stopping sprint
					bStoppingSprint = true;
				} else if (a_event->HeldDuration() < Settings::SprintingPressDuration) {//TODO:ADD THIS to settings
					if (a_event->IsUp()) {
						TKRE::dodge();
						bStoppingSprint = false;
					}
					return;
				} else if (playerCharacter && (playerCharacter->unkBDD & FlagBDD::kSprinting) == FlagBDD::kNone && !bStoppingSprint) {
					a_event->heldDownSecs = 0.f;
				} else if (a_event->IsUp()) {
					bStoppingSprint = false;
				}
			}
		}

		_ProcessButton(a_this, a_event, a_data);
	}
}
