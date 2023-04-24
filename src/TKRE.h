#pragma once
#include <SimpleIni.h>
#include <xstddef>

class TKRE
{
public:
	static bool GetDodgeEvent(std::string& a_event);

	static void dodge();

	static void applyDodgeCost();

private:
	TKRE() = default;

	~TKRE() = default;

	TKRE(const TKRE&) = delete;

	TKRE(TKRE&&) = delete;

	TKRE& operator=(const TKRE&) = delete;

	TKRE& operator=(TKRE&&) = delete;
};

class Settings
{
public:
	static inline std::uint32_t dodgeKey = 42;
	static inline bool EnableSprintKeyDodge = false;
	static inline bool EnableSneakKeyDodge = false;
	static inline bool stepDodge = false;
	static inline float dodgeStamina = 10.f;
	static inline bool enableSneakDodge = false;
	static inline bool enableDodgeAttackCancel = true;
	static inline float iFrameDuration = 0.3f;
	static inline std::string defaultDodgeEvent = "TKDodgeBack";
	static inline float SprintingPressDuration = 0.3f;
	static inline float SneakingPressDuration = 0.3f;
	static void readSettings();

private:
	static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
	static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);
	static void ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting);
};
