#pragma once
#include <xstddef>

class DataHandler
{
	struct MovDires
	{
		enum MovDire : std::uint32_t
		{
			kNone = 0,
			kForward,
			kRight,
			kBackward,
			kLeft,
		};
	};
	using MovementDirection = MovDires::MovDire;
	using DirePair = std::pair<MovementDirection, std::string_view>;


public:
	static DataHandler* GetSingleton()
	{
		static DataHandler singleton;
		return  std::addressof(singleton);
	}

	const std::string GetDodgeEvent() const;

private:
	DataHandler() = default;

	~DataHandler() = default;

	DataHandler(const DataHandler&) = delete;

	DataHandler(DataHandler&&) = delete;

	DataHandler& operator=(const DataHandler&) = delete;

	DataHandler& operator=(DataHandler&&) = delete;

	//---------------------------------- For Keyboard---------------------------------------------------------------

	const MovementDirection GetKeyboardDireValue() const;										//	Pick the PowerAttack direction based on key pressed.
	bool GetKeyHeldDuration(const std::uint32_t a_index, float& result) const;	//	Get the held duration of a key

	//--------------------------------------------------------------------------------------------------------------


	//---------------------------------- For GamePad---------------------------------------------------------------

	const MovementDirection GetGamePadDireValue() const;

	//--------------------------------------------------------------------------------------------------------------

	const std::vector<DirePair> Directions{
		DirePair(MovementDirection::kForward, "Forward"),
		DirePair(MovementDirection::kBackward, "Back"),
		DirePair(MovementDirection::kLeft, "Strafe Left"),
		DirePair(MovementDirection::kRight, "Strafe Right")
	};
};


class Settings
{
public:
	static inline std::uint32_t dodgeKey = 42;
	static inline bool enableTappingSprint = false;
	static inline float padThld = 0.15f;
	static inline bool stepDodge = false;
	static inline float dodgeStamina = 10.f;
	static inline bool enableSneakDodge = false;
	static inline float iFrameDuration = 0.3f;

	static void readSettings();

private:
	static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
	static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);
	static void ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting);

};
