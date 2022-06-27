#include "Hooks.h"
#include "InputEvents.h"
#include "TKRE.h"
#include "AnimationEvents.h"
void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
		// Skyrim lifecycle events.
	case SKSE::MessagingInterface::kPostLoad:      // Called after all plugins have finished running SKSEPlugin_Load.
												   // It is now safe to do multithreaded operations, or operations against other plugins.
	case SKSE::MessagingInterface::kPostPostLoad:  // Called after all kPostLoad message handlers have run.
	case SKSE::MessagingInterface::kInputLoaded:   // Called when all game data has been found.
		break;
	case SKSE::MessagingInterface::kDataLoaded:  // All ESM/ESL/ESP plugins have loaded, main menu is now active.
		// It is now safe to access form data.
		Settings::readSettings();
		InputEventHandler::Register();
		animEventHandler::RegisterForPlayer();
		Hooks::install();
		break;

		// Skyrim game events.
	case SKSE::MessagingInterface::kNewGame:       // Player starts a new game from main menu.
	case SKSE::MessagingInterface::kPreLoadGame:   // Player selected a game to load, but it hasn't loaded yet.
												   // Data will be the name of the loaded save.
	case SKSE::MessagingInterface::kPostLoadGame:  // Player's selected save game has finished loading.
												   // Data will be a boolean indicating whether the load was successful.
	case SKSE::MessagingInterface::kSaveGame:      // The player has saved a game.
												   // Data will be the save name.
	case SKSE::MessagingInterface::kDeleteGame:    // The player deleted a saved game from within the load menu.
		break;
	}
}

void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

#ifdef SKYRIM_SUPPORT_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);

	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	return v;
}();
#endif



extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) { Sleep(100); }
#endif

	InitializeLog();
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	SKSE::Init(a_skse);

	auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

	return true;
}

