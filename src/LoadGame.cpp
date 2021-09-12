#include "LoadGame.h"
#include "DataHandler.h"
#include "Hooks.h"
#include "SimpleIni.h"
#include "InputEvents.h"
#include "AnimationEvents.h"

namespace TKDodge
{
#define SETTINGFILE_PATH "Data\\SKSE\\Plugins\\TK Dodge RE.ini"

	DataHandler::DataHandler()
	{
		CSimpleIniA ini;
		SI_Error rc = ini.LoadFile(SETTINGFILE_PATH);

		std::uint32_t dodgeKey = ini.GetLongValue("Main", "DodgeHotkey", 42);
		float padThld = ini.GetDoubleValue("Main", "GamePadThreshold", 0.15f);
		std::int32_t stepDodge = ini.GetLongValue("Main", "StepDodge");
		float dodgeStamina = ini.GetDoubleValue("Main", "DodgeStamina", 10.f);
		float iFrameDuration = ini.GetDoubleValue("Main", "iFrameDuration", 0.3f);

		settings = std::make_unique<INIData>(dodgeKey, padThld, stepDodge, dodgeStamina, iFrameDuration);

		if (ini.GetBoolValue("Main", "EnableDebugLog")) {
			spdlog::set_level(spdlog::level::debug);
			logger::debug("Enable Debug Log!");
		}

		InputEventHandler::Register();
		IsGhostHook::InstallHook();
		InitScriptHandler::RegisterEvent();
	}

	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
			logger::info("Data Load CallBack Trigger!");
			DataHandler::GetSingleton();
		}	
	}
}