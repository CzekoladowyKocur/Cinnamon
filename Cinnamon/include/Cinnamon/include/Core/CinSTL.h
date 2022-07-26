#pragma once
#include "Cinnamon/include/Core/Core.h"

namespace Cinnamon {
	namespace STL {
		/* TODO: Provide custom allocators */
		template<typename T, std::size_t size>
		using Array = std::array<T, size>;

		template<typename T>
		using Vector = std::vector<T>;

		template <typename KeyType, typename ValueType>
		using UMap = std::unordered_map<KeyType, ValueType>;

		using StringView = std::string_view;
		using StringU8View = std::u8string_view;
		using StringU16View = std::u16string_view;
		using StringU32View = std::u32string_view;

		using String = std::string;
		using StringU8 = std::u8string;
		using StringU16 = std::u16string;
		using StringU32 = std::u32string;
	}
}