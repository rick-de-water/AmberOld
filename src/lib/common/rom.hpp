#ifndef H_AMBER_COMMON_ROM
#define H_AMBER_COMMON_ROM

#include <common/memory.hpp>

#include <memory>

namespace Amber::Common
{
	template <typename T, bool BE>
	class ROM : public MemoryHelper<T, BE>
	{
		public:
		explicit ROM(size_t a_Size):
			m_Size(a_Size),
			m_Data(std::make_unique<uint8_t[]>(m_Size))
		{
			std::memset(m_Data.get(), 0, m_Size);
		}

		size_t GetSize() const noexcept
		{
			return m_Size;
		}

		uint8_t* GetData() noexcept
		{
			return const_cast<uint8_t*>(static_cast<const ROM*>(this)->GetData());
		}

		const uint8_t* GetData() const noexcept
		{
			return m_Data.get();
		}

		uint8_t Load8(Address a_Address) const override
		{
			return GetData()[a_Address];
		}

		void Store8(Address a_Address, uint8_t a_Value) override
		{
		}

		private:
		const size_t m_Size;
		std::unique_ptr<uint8_t[]> m_Data;
	};

	template <bool BE> using ROM8  = ROM<uint8_t, BE>;
	template <bool BE> using ROM16 = ROM<uint16_t, BE>;
	template <bool BE> using ROM32 = ROM<uint32_t, BE>;
	template <bool BE> using ROM64 = ROM<uint64_t, BE>;
}

#endif