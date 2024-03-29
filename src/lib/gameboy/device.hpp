#ifndef H_AMBER_GAMEBOY_DEVICE
#define H_AMBER_GAMEBOY_DEVICE

#include <gameboy/api.hpp>
#include <gameboy/devicedescription.hpp>

#include <memory>

namespace Amber::Gameboy
{
	class CPU;
	class Joypad;
	class PPU;
	class MMU;

	class GAMEBOY_API Device
	{
		public:
		Device(const DeviceDescription& a_Description);
		~Device() noexcept;

		const DeviceDescription& GetDescription() const noexcept;

		MMU& GetMMU() noexcept;
		CPU& GetCPU() noexcept;
		PPU& GetPPU() noexcept;
		Joypad& GetJoypad() noexcept;

		bool Tick();
		void Reset();

		private:
		const DeviceDescription m_Description;

		std::unique_ptr<MMU> m_MMU;
		std::unique_ptr<CPU> m_CPU;
		std::unique_ptr<PPU> m_PPU;
		std::unique_ptr<Joypad> m_Joypad;
	};
}

#endif