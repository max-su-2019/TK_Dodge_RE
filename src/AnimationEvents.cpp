#include "AnimationEvents.h"
#include "DataHandler.h"

namespace TKDodge
{
	EventResult PlayerAnimationHandler::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
	{
		if (!a_event)
			return EventResult::kContinue;

		if (a_event->holder && a_event->holder->IsPlayerRef()) {
			auto datahandler = DataHandler::GetSingleton();
			if (a_event->tag == "TKDR_DodgeStart") {
				logger::debug(FMT_STRING("TK Dodge Start, Time {}!"),clock());
				if (!RE::PlayerCharacter::IsGodMode())
					RE::PlayerCharacter::GetSingleton()->As<RE::ActorValueOwner>()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kStamina, -datahandler->settings->dodgeStamina);
			} else if (a_event->tag == "TKDR_IFrameEnd") {
				logger::debug(FMT_STRING("Invulnerable Frame End!, Time {}"), clock());	
			}
		}

		return EventResult::kContinue;
	}
}