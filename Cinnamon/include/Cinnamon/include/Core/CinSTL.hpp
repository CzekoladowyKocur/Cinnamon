#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	namespace STL {
		template<typename T>
		using Unique = std::unique_ptr<T>;
		template<typename T, typename ... Args>
		constexpr Unique<T> MakeUnique(Args&& ... args) noexcept
		{
			return std::make_unique<T>(std::forward<Args>(args)...);
		}

		template<typename T>
		using Shared = std::shared_ptr<T>;
		template<typename T, typename ... Args>
		constexpr Shared<T> MakeShared(Args&& ... args) noexcept
		{
			return std::make_shared<T>(std::forward<Args>(args)...);
		}

		template<typename T>
		using Weak = std::weak_ptr<T>;

		/* TODO: Provide custom allocators */
		template<typename T, std::size_t size>
		using Array = std::array<T, size>;

		template<typename T>
		using Queue = std::queue<T>;

		template<typename T>
		using Vector = std::vector<T>;

		template<typename T>
		using InitializerList = std::initializer_list<T>;

		template <typename KeyType, typename ValueType>
		using Map = std::map<KeyType, ValueType>;

		template <typename KeyType, typename ValueType>
		using UMap = std::unordered_map<KeyType, ValueType>;

		template<typename T>
		using Set = std::set<T>;

		using StringView = std::string_view;
		using StringU8View = std::u8string_view;
		using StringU16View = std::u16string_view;
		using StringU32View = std::u32string_view;

		using String = std::string;
		using StringU8 = std::u8string;
		using StringU16 = std::u16string;
		using StringU32 = std::u32string;

		using WString = std::wstring;

		using Filepath = std::filesystem::path;
		using LastFileWriteStamp = std::filesystem::file_time_type;

		using DirectoryIterator = std::filesystem::directory_iterator;
		using RecursiveDirectoryIterator = std::filesystem::recursive_directory_iterator;

		using Mutex = std::mutex;

		template<typename F>
		concept IsFunction = std::is_function_v<F>;

		template<typename Sig>
		struct FunctionSignatureRetriever;

		template<typename R, typename ...Args>
		struct FunctionSignatureRetriever<R(Args...)>
		{
			using type = std::tuple<Args...>;
		};

		template<IsFunction F>
		auto FunctionSignature(const F&) -> typename FunctionSignatureRetriever<F>::type;
	}
}