#pragma once

namespace TKDodge
{
	using EventResult = RE::BSEventNotifyControl;

	class InputEventHandler : public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		virtual EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;

		static void Register()
		{
			INFO("Start Registered!");
			auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
			deviceManager->AddEventSink(InputEventHandler::GetSingleton());
			INFO("Registered {}"sv, typeid(RE::InputEvent).name());
		}

		static std::uint32_t GetSprintKey(RE::INPUT_DEVICE a_device);

	private:
		static InputEventHandler* GetSingleton()
		{
			static InputEventHandler singleton;
			return std::addressof(singleton);
		}

		InputEventHandler() = default;
		InputEventHandler(const InputEventHandler&) = delete;
		InputEventHandler(InputEventHandler&&) = delete;
		virtual ~InputEventHandler() = default;

		InputEventHandler& operator=(const InputEventHandler&) = delete;
		InputEventHandler& operator=(InputEventHandler&&) = delete;	

		
		static std::uint32_t GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key);

		enum : uint32_t
		{
			kInvalid = static_cast<uint32_t>(-1),
			kKeyboardOffset = 0,
			kMouseOffset = 256,
			kGamepadOffset = 266
		};
	};


}