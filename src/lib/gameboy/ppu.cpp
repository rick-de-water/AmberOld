#include <gameboy/ppu.hpp>

#include <gameboy/cpu.hpp>
#include <gameboy/mmu.hpp>

using namespace Amber;
using namespace Gameboy;

PPU::PPU()
{
	Reset();
}

uint8_t PPU::GetLY() const noexcept
{
	return static_cast<uint8_t>(m_VCounter);
}

void PPU::SetCPU(CPU* a_CPU) noexcept
{
	m_CPU = a_CPU;
}

void PPU::SetMMU(MMU* a_MMU) noexcept
{
	m_MMU = a_MMU;
}

void PPU::Tick()
{
	// Increment counters
	++m_HCounter;
	if (m_HCounter == LineCycles)
	{
		m_HCounter = 0;
		++m_VCounter;
		if (m_VCounter == FrameLines)
		{
			m_VCounter = 0;
		}
	}

	// Update LCD mode
	switch (m_LCDMode)
	{
		case LCDMode::HBlank:
		if (m_HCounter == 0)
		{
			if (m_VCounter == ScreenLines)
			{
				m_LCDMode = LCDMode::VBlank;
				if (m_CPU != nullptr)
				{
					m_CPU->RequestInterrupts(CPU::InterruptVBlank);
				}
			}
			else
			{
				m_LCDMode = LCDMode::OAMSearch;
			}
		}
		break;

		case LCDMode::VBlank:
		if (m_VCounter == 0)
		{
			m_LCDMode = LCDMode::OAMSearch;
		}
		break;

		case LCDMode::OAMSearch:
		if (m_HCounter == OAMCycles)
		{
			m_LCDMode = LCDMode::PixelTransfer;
		}
		break;

		case LCDMode::PixelTransfer:
		// TODO: accurate timing
		if (m_HCounter == 240)
		{
			m_LCDMode = LCDMode::HBlank;
		}
		break;
	}

	// Process LCD Mode
	switch (m_LCDMode)
	{
		case LCDMode::OAMSearch:
		OAMSearch();
		break;

		case LCDMode::PixelTransfer:
		PixelTransfer();
		break;
	}
}

void PPU::Reset()
{
	m_HCounter = LineCycles - 1;
	m_VCounter = FrameLines - 1;
	m_LCDMode = LCDMode::VBlank;
}

void PPU::Blit(void* a_Destination, size_t a_Pitch) const noexcept
{
	static constexpr uint8_t colors[] = { 0xFF, 0x77, 0xCC, 0x00 };

	for (size_t y = 0; y < LCDHeight; ++y)
	{
		for (size_t x = 0; x < LCDWidth; ++x)
		{
			const uint8_t color = colors[GetPixel(x, y)];

			uint8_t* const pixel = reinterpret_cast<uint8_t*>(a_Destination) + (y * a_Pitch + x) * 4;

			pixel[0] = color;
			pixel[1] = color;
			pixel[2] = color;
			pixel[3] = 0xFF;
		}
	}
}

void PPU::OAMSearch() noexcept
{

}

void PPU::PixelTransfer() noexcept
{
	const uint8_t x = static_cast<uint8_t>(m_HCounter - OAMCycles);
	const uint8_t y = static_cast<uint8_t>(m_VCounter);

	const uint8_t background_x = x / 8;
	const uint8_t background_y = y / 8;

	const uint16_t tile_index_offset = background_x + background_y * 32;
	const uint8_t tile_index = m_MMU->Load8(0x9800 + tile_index_offset);

	uint8_t tile_data[16];
	for (uint16_t i = 0; i < 16; ++i)
	{
		tile_data[i] = m_MMU->Load8(0x8000 + (tile_index * 16 + i));
	}

	const uint8_t tile_x = x % 8;
	const uint8_t tile_y = y % 8;

	const uint8_t line = tile_y * 2;

	const uint8_t byte0 = tile_data[line + 0];
	const uint8_t byte1 = tile_data[line + 1];

	const uint8_t bit0 = (byte0 >> (7 - tile_x)) & 0x01;
	const uint8_t bit1 = (byte1 >> (7 - tile_x)) & 0x01;

	const uint8_t color = bit0 | (bit1 << 1);

	SetPixel(x, y, color);
}

uint8_t PPU::GetPixel(uint8_t a_X, uint8_t a_Y) const noexcept
{
	const size_t byte_offset = (static_cast<size_t>(a_X) + static_cast<size_t>(a_Y) * LCDWidth) / 4;
	const uint8_t bit_offset = (3 - (a_X % 4)) * 2;

	const uint8_t pixel_mask = (0b11 << bit_offset);

	return (m_LCDBuffer[byte_offset] & pixel_mask) >> bit_offset;
}

void PPU::SetPixel(uint8_t a_X, uint8_t a_Y, uint8_t a_Color) noexcept
{
	const size_t byte_offset = (static_cast<size_t>(a_X) + static_cast<size_t>(a_Y) * LCDWidth) / 4;
	const uint8_t bit_offset = (3 - (a_X % 4)) * 2;

	const uint8_t pixel_mask = ~(0b11 << bit_offset);

	m_LCDBuffer[byte_offset] = (m_LCDBuffer[byte_offset] & pixel_mask) | ((a_Color & 0b11) << bit_offset);
}