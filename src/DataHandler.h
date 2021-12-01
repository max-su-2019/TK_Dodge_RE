#pragma once

namespace TKDodge
{
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
		using MovDire = MovDires::MovDire;
		using DirePair = std::pair<MovDire, std::string_view>;

		struct INIData
		{
			INIData() = delete;

			INIData(const INIData&) = delete;

			INIData(INIData&&) = delete;

			INIData& operator=(const INIData&) = delete;

			INIData& operator=(INIData&&) = delete;

			explicit INIData(std::uint32_t a_dodgeKey, bool a_enableTappingSprint, float a_padThld, std::int32_t a_istepDodge, float a_dodgestamina, float a_iFrameDuration) :
				dodgeKey(a_dodgeKey), enableTappingSprint(a_enableTappingSprint), padThld(a_padThld), istepDodge(a_istepDodge), dodgeStamina(a_dodgestamina), iFrameDuration(a_iFrameDuration)
			{
				INFO(FMT_STRING("Set Dodge Hotkey to {}, Tapping Sprint to {}, GamePad Threshold to {}, Step Dodge to {}, Dodge Stamina to {}, iFrame Duration to {}"), dodgeKey, enableTappingSprint,  padThld, istepDodge, dodgeStamina, a_iFrameDuration);
			};
		
			const std::uint32_t		dodgeKey;
			const bool              enableTappingSprint;
			const float				padThld;
			const std::int32_t		istepDodge;
			const float				dodgeStamina;
			const float				iFrameDuration;
		};

	public:
		static DataHandler* GetSingleton()
		{
			static DataHandler singleton;
			return  std::addressof(singleton);
		}

		const std::string GetDodgeEvent() const;

	private:
		DataHandler();

		~DataHandler() = default;

		DataHandler(const DataHandler&) = delete;

		DataHandler(DataHandler&&) = delete;

		DataHandler& operator=(const DataHandler&) = delete;

		DataHandler& operator=(DataHandler&&) = delete;

		//---------------------------------- For Keyboard---------------------------------------------------------------

		const MovDire GetKeyboardDireValue() const;										//	Pick the PowerAttack direction based on key pressed.
		bool GetKeyHeldDuration(const std::uint32_t a_index, float& result) const;	//	Get the held duration of a key

		//--------------------------------------------------------------------------------------------------------------


		//---------------------------------- For GamePad---------------------------------------------------------------

		const MovDire GetGamePadDireValue() const;

		//--------------------------------------------------------------------------------------------------------------

		const std::vector<DirePair> Directions{
			DirePair(MovDire::kForward, "Forward"),
			DirePair(MovDire::kBackward, "Back"),
			DirePair(MovDire::kLeft, "Strafe Left"),
			DirePair(MovDire::kRight, "Strafe Right")
		};

	public:
		std::unique_ptr<INIData> settings;
	};
}