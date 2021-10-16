#include "LoadGame.h"
#include "DataHandler.h"
#include "SimpleIni.h"
#include "InputEvents.h"

namespace TKDodge
{
#define SETTINGFILE_PATH "Data\\SKSE\\Plugins\\TK Dodge RE.ini"

	DataHandler::DataHandler()
	{
		CSimpleIniA ini;
		SI_Error rc = ini.LoadFile(SETTINGFILE_PATH);

		std::uint32_t dodgeKey = ini.GetLongValue("Main", "DodgeHotkey", 42);

		bool enableTappingDodge = ini.GetBoolValue("Main", "EnableTappingDodge");

		float padThld = ini.GetDoubleValue("Main", "GamePadThreshold", 0.15f);

		std::int32_t stepDodge = ini.GetBoolValue("Main", "StepDodge");
		stepDodge ? stepDodge = 2 : stepDodge = 0;

		float dodgeStamina = ini.GetDoubleValue("Main", "DodgeStamina", 10.f);
		if (dodgeStamina < 0.f) { dodgeStamina = 0.f;}

		float iFrameDuration = ini.GetDoubleValue("Main", "iFrameDuration", 0.3f);
		if (iFrameDuration < 0.f) { iFrameDuration = 0.f;}

		settings = std::make_unique<INIData>(dodgeKey, enableTappingDodge, padThld, stepDodge, dodgeStamina, iFrameDuration);

		if (ini.GetBoolValue("Main", "EnableDebugLog")) {
			spdlog::set_level(spdlog::level::debug);
			logger::debug("Enable Debug Log!");
		}

		InputEventHandler::Register();
	}

	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
			logger::info("Data Load CallBack Trigger!");
			DataHandler::GetSingleton();
		}	
	}
}