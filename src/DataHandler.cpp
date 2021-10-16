#include "DataHandler.h"

namespace TKDodge
{
	bool DataHandler::GetKeyHeldDuration(const std::uint32_t a_index, float& result) const
	{
		auto inputMgr = RE::BSInputDeviceManager::GetSingleton();

		if (inputMgr && inputMgr->GetKeyboard()->IsPressed(a_index) && inputMgr->GetKeyboard()->deviceButtons.find(a_index)->second) {
			result = inputMgr->GetKeyboard()->deviceButtons.find(a_index)->second->heldDownSecs;
			return true;
		}

		return false;
	}

	const DataHandler::MovDire DataHandler::GetKeyboardDireValue() const
	{
		auto ctrlMap = RE::ControlMap::GetSingleton();

		std::optional<MovDire> pickedDir;
		float recordDur = 0.f;
		for (auto dir : Directions) {
			auto key = ctrlMap->GetMappedKey(dir.second, RE::INPUT_DEVICE::kKeyboard);

			float heldDur;
			if (GetKeyHeldDuration(key, heldDur) && (!pickedDir.has_value() || heldDur <= recordDur)) {
				pickedDir.emplace(dir.first);
				recordDur = heldDur;
				logger::debug(FMT_STRING("Get a Held Direction Key Index {:x}, Duration {}"), key, recordDur);
			}
		}

		return pickedDir.has_value() ? pickedDir.value() : MovDire::kNone;
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

	const DataHandler::MovDire DataHandler::GetGamePadDireValue() const
	{
		auto inputMgr = RE::BSInputDeviceManager::GetSingleton();

		if (inputMgr && inputMgr->GetGamepad()) {
			auto gamePad = (RE::BSWin32GamepadDevice*)(inputMgr->GetGamepad());
			if (gamePad) {
				logger::debug(FMT_STRING("Current LX is {}, Current LY is {}"), gamePad->curLX, gamePad->curLY);

				float dir_xy[2] = { gamePad->curLX, gamePad->curLY };
				static const float dir_base[2] = { 0, 1.0f };

				float power = sqrt(std::powf(gamePad->curLX, 2) + std::powf(gamePad->curLY, 2));
				logger::debug(FMT_STRING("Current Power is {}"), power);

				float theta = (dir_xy[0] * dir_base[0] + dir_xy[1] * dir_base[1]) / sqrt(dir_xy[0] * dir_xy[0] + dir_xy[1] * dir_xy[1]) / sqrt(dir_base[0] * dir_base[0] + dir_base[1] * dir_base[1]);
				theta = gamePad->curLX >= 0.f ? std::acos(theta) : -std::acos(theta);
				logger::debug(FMT_STRING("theta is {}"), theta);

				auto dir = NormalAbsoluteAngle(theta);
				dir /= 6.283185f;
				dir += 0.125f;
				dir *= 4.0f;
				dir = fmod(dir, 4.0f);
				dir = floor(dir);
				dir += 1.0f;
				logger::debug(FMT_STRING("GamePad Direction is {}"), dir);

				if (power > settings->padThld) {
					switch (std::int32_t(dir)) {
					case 1:
						return MovDire ::kForward;
					case 2:
						return MovDire ::kRight;
					case 3:
						return MovDire ::kBackward;
					case 4:
						return MovDire::kLeft;
					default:
						return MovDire::kNone;
					}
				}		
			}
		}

		return MovDire::kNone;
	}


	const std::string DataHandler::GetDodgeEvent() const
	{
		MovDire dire = MovDire::kNone;

		auto inputMgr = RE::BSInputDeviceManager::GetSingleton();
		if (inputMgr && inputMgr->GetKeyboard() && inputMgr->GetKeyboard()->IsEnabled() && !inputMgr->IsGamepadConnected())
			dire = GetKeyboardDireValue();
		else if (inputMgr->IsGamepadConnected())
			dire = GetGamePadDireValue();

		auto enable_tdm = RE::TESForm::LookupByEditorID<RE::TESGlobal>("TDM_DirectionalMovement");

		if (enable_tdm && enable_tdm->value && dire != MovDire::kNone) {
			logger::debug("TDM Install, Force to Forward Dodge!");
			return "TKDodgeForward";
		} else if (RE::PlayerCharacter::GetSingleton()->IsSprinting() && !settings->enableTappingSprint) {
			logger::debug("Player is Sprinting, Force to Forward Dodge!");
			return "TKDodgeForward";
		}

		switch (dire){
		case MovDire::kForward:
			return "TKDodgeForward";
		case MovDire::kBackward:
			return "TKDodgeBack";
		case MovDire::kLeft:
			return "TKDodgeLeft";
		case MovDire::kRight:
			return "TKDodgeRight";
		default:
			return "";
		}
	}
}