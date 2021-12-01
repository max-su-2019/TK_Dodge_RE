#pragma once

namespace TKDodge
{
	using EventResult = RE::BSEventNotifyControl;

	class PlayerAnimationHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
	{
		using ATTACK_STATE = RE::ATTACK_STATE_ENUM;

	public:
		virtual EventResult ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

		static bool RegisterSink(RE::Actor* pc)
		{
			static PlayerAnimationHandler g_eventhandler;

			RE::BSAnimationGraphManagerPtr graphMgr;

			if (pc)
				pc->GetAnimationGraphManager(graphMgr);

			if (!graphMgr || !graphMgr->graphs.cbegin()) {
				ERROR("Player Graph not found!");
				return false;
			}

			graphMgr->graphs.cbegin()->get()->AddEventSink(&g_eventhandler);

			DEBUG("Register Animation Event Handler!");

			return true;
		}

	private:
		PlayerAnimationHandler() = default;

		~PlayerAnimationHandler() = default;
	};

}