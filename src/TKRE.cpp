#include "TKRE.h"



#define SETTINGFILE_PATH "Data\\SKSE\\Plugins\\TK Dodge RE.ini"


static float NormalAbsoluteAngle(float angle)
{
	static const float PI = 3.1415927f;

	while (angle < 0)
		angle += 2 * PI;
	while (angle > 2 * PI)
		angle -= 2 * PI;
	return angle;
}
#define PI 3.14159265f
#define PI8 0.39269908f

inline RE::NiPoint2 Vec2Rotate(const RE::NiPoint2& vec, float angle)
{
	RE::NiPoint2 ret;
	ret.x = vec.x * cos(angle) - vec.y * sin(angle);
	ret.y = vec.x * sin(angle) + vec.y * cos(angle);
	return ret;
}

inline float Vec2Length(const RE::NiPoint2& vec)
{
	return std::sqrtf(vec.x * vec.x + vec.y * vec.y);
}

inline RE::NiPoint2 Vec2Normalize(RE::NiPoint2& vec)
{
	RE::NiPoint2 ret(0.f, 0.f);
	float vecLength = Vec2Length(vec);
	if (vecLength == 0) {
		return ret;
	}
	float invlen = 1.0f / vecLength;
	ret.x = vec.x * invlen;
	ret.y = vec.y * invlen;
	return ret;
}

inline float DotProduct(RE::NiPoint2& a, RE::NiPoint2& b)
{
	return a.x * b.x + a.y * b.y;
}

inline float CrossProduct(RE::NiPoint2& a, RE::NiPoint2& b)
{
	return a.x * b.y - a.y * b.x;
}

inline float GetAngle(RE::NiPoint2& a, RE::NiPoint2& b)
{
	return atan2(CrossProduct(a, b), DotProduct(a, b));
}

const std::string TKRE::GetDodgeEvent() const
{
	auto playerControls = RE::PlayerControls::GetSingleton();
	if (!playerControls || !playerControls->attackBlockHandler ||
		!playerControls->attackBlockHandler->inputEventHandlingEnabled || !playerControls->movementHandler || !playerControls->movementHandler->inputEventHandlingEnabled) {
		return "";
	}

	auto normalizedInputDirection = Vec2Normalize(playerControls->data.prevMoveVec);
	if (normalizedInputDirection.x == 0.f && normalizedInputDirection.y == 0.f) {
		return "TKDodgeBack";
	}
	RE::NiPoint2 forwardVector(0.f, 1.f);
	float dodgeAngle = GetAngle(normalizedInputDirection, forwardVector);
	if (dodgeAngle >= -2 * PI8 && dodgeAngle < 2 * PI8) {
		return "TKDodgeForward";
	} else if (dodgeAngle >= -6 * PI8 && dodgeAngle < -2 * PI8) {
		return "TKDodgeLeft";
	} else if (dodgeAngle >= 6 * PI8 || dodgeAngle < -6 * PI8) {
		return "TKDodgeBack";
	} else if (dodgeAngle >= 2 * PI8 && dodgeAngle < 6 * PI8) {
		return "TKDodgeRight";
	}
	return "TKDodgeBack";
}

inline bool isJumping(RE::Actor* a_actor)
{
	bool result = false;
	return a_actor->GetGraphVariableBool("bInJumpState", result) && result;
}

void TKRE::dodge() {
	//logger::info("dodging");
	auto pc = RE::PlayerCharacter::GetSingleton();
	if (pc->IsSprinting() && Settings::enableTappingSprint) {
		return;
	}
	const std::string dodge_event = TKRE::GetSingleton()->GetDodgeEvent();
	if (!dodge_event.empty() && pc->GetSitSleepState() == RE::SIT_SLEEP_STATE::kNormal && pc->GetKnockState() == RE::KNOCK_STATE_ENUM::kNormal &&
		pc->GetFlyState() == RE::FLY_STATE::kNone && (!pc->IsSneaking() || Settings::enableSneakDodge) && !pc->IsSwimming() &&
		!isJumping(pc) && !pc->IsInKillMove() && (pc->GetActorValue(RE::ActorValue::kStamina) >= Settings::dodgeStamina)) {
		//DEBUG(FMT_STRING("{} Trigger!"), dodge_event);
		bool IsDodging = false;
		if (pc->GetGraphVariableBool("bIsDodging", IsDodging) && IsDodging) {
			//DEBUG("Player is already dodging!");
			return;
		}
		if (Settings::stepDodge) {
			pc->SetGraphVariableInt("iStep", 2);
		}
		pc->SetGraphVariableFloat("TKDR_IframeDuration", Settings::iFrameDuration);  //Set invulnerable frame duration
		pc->NotifyAnimationGraph(dodge_event);                                       //Send TK Dodge Event
	}
}


void Settings::ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		//logger::info("found {} with value {}", a_settingName, bFound);
		a_setting = static_cast<int>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
	}
}
void Settings::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		//logger::info("found {} with value {}", a_settingName, bFound);
		a_setting = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
	}
}

void Settings::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		//logger::info("found {} with value {}", a_settingName, bFound);
		a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
	}
}


void Settings::readSettings() {
	CSimpleIniA ini;
	ini.LoadFile(SETTINGFILE_PATH);

	ReadIntSetting(ini, "Main", "DodgeHotkey", dodgeKey);
	ReadBoolSetting(ini, "Main", "EnableTappingDodge", enableTappingSprint);
	ReadFloatSetting(ini, "Main", "GamePadThreshold", padThld);
	ReadBoolSetting(ini, "Main", "StepDodge", stepDodge);
	ReadFloatSetting(ini, "Main", "DodgeStamina", dodgeStamina);
	ReadBoolSetting(ini, "Main", "EnableSneakDodge", enableSneakDodge);
	ReadFloatSetting(ini, "Main", "iFrameDuration", iFrameDuration);
	if (iFrameDuration < 0.f) {
		iFrameDuration = 0.f;
	}

	if (ini.GetBoolValue("Main", "EnableDebugLog")) {
		spdlog::set_level(spdlog::level::debug);
		logger::debug("Debug log enabled");
	}
}
