#include "DataHandler.h"



#define SETTINGFILE_PATH "Data\\SKSE\\Plugins\\TK Dodge RE.ini"


bool DataHandler::GetKeyHeldDuration(const std::uint32_t a_index, float& result) const
{
	auto inputMgr = RE::BSInputDeviceManager::GetSingleton();

	if (inputMgr && inputMgr->GetKeyboard()->IsPressed(a_index) && inputMgr->GetKeyboard()->deviceButtons.find(a_index)->second) {
		result = inputMgr->GetKeyboard()->deviceButtons.find(a_index)->second->heldDownSecs;
		return true;
	}

	return false;
}

const DataHandler::MovementDirection DataHandler::GetKeyboardDireValue() const
{
	auto ctrlMap = RE::ControlMap::GetSingleton();

	std::optional<MovementDirection> pickedDir;
	float recordDur = 0.f;
	for (auto dir : Directions) {
		auto key = ctrlMap->GetMappedKey(dir.second, RE::INPUT_DEVICE::kKeyboard);

		float heldDur;
		if (GetKeyHeldDuration(key, heldDur) && (!pickedDir.has_value() || heldDur <= recordDur)) {
			pickedDir.emplace(dir.first);
			recordDur = heldDur;
			//DEBUG(FMT_STRING("Get a Held Direction Key Index {:x}, Duration {}"), key, recordDur);
		}
	}

	return pickedDir.has_value() ? pickedDir.value() : MovementDirection::kNone;
}



static float NormalAbsoluteAngle(float angle)
{
	static const float PI = 3.1415927f;

	while (angle < 0)
		angle += 2 * PI;
	while (angle > 2 * PI)
		angle -= 2 * PI;
	return angle;
}

const DataHandler::MovementDirection DataHandler::GetGamePadDireValue() const
{
	auto inputMgr = RE::BSInputDeviceManager::GetSingleton();

	if (inputMgr && inputMgr->GetGamepad()) {
		auto gamePad = (RE::BSWin32GamepadDevice*)(inputMgr->GetGamepad());
		if (gamePad) {
			//DEBUG(FMT_STRING("Current LX is {}, Current LY is {}"), gamePad->curLX, gamePad->curLY);

			float dir_xy[2] = { gamePad->curLX, gamePad->curLY };
			static const float dir_base[2] = { 0, 1.0f };

			float power = sqrt(std::powf(gamePad->curLX, 2) + std::powf(gamePad->curLY, 2));
			//DEBUG(FMT_STRING("Current Power is {}"), power);

			float theta = (dir_xy[0] * dir_base[0] + dir_xy[1] * dir_base[1]) / sqrt(dir_xy[0] * dir_xy[0] + dir_xy[1] * dir_xy[1]) / sqrt(dir_base[0] * dir_base[0] + dir_base[1] * dir_base[1]);
			theta = gamePad->curLX >= 0.f ? std::acos(theta) : -std::acos(theta);
			//DEBUG(FMT_STRING("theta is {}"), theta);

			auto dir = NormalAbsoluteAngle(theta);
			dir /= 6.283185f;
			dir += 0.125f;
			dir *= 4.0f;
			dir = fmod(dir, 4.0f);
			dir = floor(dir);
			dir += 1.0f;
			//DEBUG(FMT_STRING("GamePad Direction is {}"), dir);

			if (power > Settings::padThld) {
				switch (std::int32_t(dir)) {
				case 1:
					return MovementDirection ::kForward;
				case 2:
					return MovementDirection ::kRight;
				case 3:
					return MovementDirection ::kBackward;
				case 4:
					return MovementDirection::kLeft;
				default:
					return MovementDirection::kNone;
				}
			}		
		}
	}

	return MovementDirection::kNone;
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

const std::string DataHandler::GetDodgeEvent() const
{
	auto playerControls = RE::PlayerControls::GetSingleton();
	if (!playerControls || !playerControls || !playerControls->attackBlockHandler ||
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
