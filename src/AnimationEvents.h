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
				logger::error("Player Graph not found!");
				return false;
			}

			graphMgr->graphs.cbegin()->get()->AddEventSink(&g_eventhandler);

			logger::info("Register Animation Event Handler!");

			return true;
		}

	private:
		PlayerAnimationHandler() = default;

		~PlayerAnimationHandler() = default;
	};


	class InitScriptHandler : public RE::BSTEventSink<RE::TESInitScriptEvent>
	{
	public:
		virtual EventResult ProcessEvent(const RE::TESInitScriptEvent* a_event, RE::BSTEventSource<RE::TESInitScriptEvent>*)
		{
			if (a_event && a_event->objectInitialized && a_event->objectInitialized->IsPlayerRef() && a_event->objectInitialized->As<RE::Actor>()) {
				PlayerAnimationHandler::RegisterSink(a_event->objectInitialized->As<RE::Actor>());
			}

			return EventResult::kContinue;
		}

		static bool RegisterEvent()
		{
			static InitScriptHandler eventhandler;

			auto ScriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();

			if (!ScriptEventSource) {
				logger::error("ScriptEventSource not found!");
				return false;
			}

			ScriptEventSource->AddEventSink(&eventhandler);

			logger::info("Register InitScript Handler!");

			return true;
		}


	private:
		InitScriptHandler() = default;

		~InitScriptHandler() = default;

		InitScriptHandler(const InitScriptHandler&) = delete;

		InitScriptHandler(InitScriptHandler&&) = delete;

		InitScriptHandler& operator=(const InitScriptHandler&) = delete;

		InitScriptHandler& operator=(InitScriptHandler&&) = delete;
	};
}