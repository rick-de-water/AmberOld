#ifndef H_AMBER_GAMEBOY_EVENT
#define H_AMBER_GAMEBOY_EVENT

#include <gameboy/api.hpp>

#include <array>
#include <optional>

namespace Amber::Gameboy
{
	namespace Event
	{
		enum Enum
		{
			VBlankBegin,
			VBlankEnd,
			OAMSearchBegin,
			OAMSearchEnd,
			PixelTransferBegin,
			PixelTransferEnd,
			HBlankBegin,
			HBlankEnd,
		};

		constexpr std::array<Event::Enum, 8> Enums = { Event::VBlankBegin, Event::VBlankEnd, Event::OAMSearchBegin, Event::OAMSearchEnd, Event::PixelTransferBegin, Event::PixelTransferEnd, Event::HBlankBegin, Event::HBlankEnd };

		constexpr std::optional<std::string_view> ToString(Event::Enum a_Value) noexcept
		{
			switch (a_Value)
			{
				case Event::VBlankBegin:        return "V-Blank Begin";
				case Event::VBlankEnd:          return "V-Blank End";
				case Event::OAMSearchBegin:     return "OAM Search Begin";
				case Event::OAMSearchEnd:       return "OAM Search End";
				case Event::PixelTransferBegin: return "Pixel Transfer Begin";
				case Event::PixelTransferEnd:   return "Pixel Transfer End";
				case Event::HBlankBegin:        return "H-Blank Begin";
				case Event::HBlankEnd:          return "H-Blank End";

				default:
				return {};
			}
		}
	};
}

#endif