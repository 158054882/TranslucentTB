#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <fmt/format.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <string>
#include <string_view>
#include <type_traits>

namespace RapidJSONHelper {
	using value_t = rapidjson::GenericValue<rapidjson::UTF16LE<>>;

	struct DeserializationError {
		const std::wstring what;
	};

	static constexpr std::array<std::wstring_view, 7> TYPE_NAMES = {
		L"null",
		L"bool",
		L"bool",
		L"object",
		L"array",
		L"string",
		L"number"
	};

	constexpr bool IsType(rapidjson::Type a, rapidjson::Type b) noexcept
	{
		return a == b ||
			(a == rapidjson::Type::kFalseType && b == rapidjson::Type::kTrueType) ||
			(a == rapidjson::Type::kTrueType && b == rapidjson::Type::kFalseType);
	}

	inline void EnsureType(rapidjson::Type expected, rapidjson::Type actual, std::wstring_view obj)
	{
		if (!IsType(expected, actual)) [[unlikely]]
		{
			throw DeserializationError {
				fmt::format(FMT_STRING(L"Expected {} but found {} while deserializing {}"), TYPE_NAMES.at(expected), TYPE_NAMES.at(actual), obj)
			};
		}
	}

	inline void AssertLength(std::wstring_view str)
	{
		assert(str.length() <= std::numeric_limits<rapidjson::SizeType>::max());
	}

	inline std::wstring_view ValueToStringView(const value_t &val)
	{
		assert(val.GetType() == rapidjson::Type::kStringType); // caller should have already ensured

		return { val.GetString(), val.GetStringLength() };
	}

	inline value_t StringViewToValue(std::wstring_view str)
	{
		AssertLength(str);
		return { str.data(), static_cast<rapidjson::SizeType>(str.length()) };
	}

	template<class Writer>
	inline void WriteKey(Writer &writer, std::wstring_view key)
	{
		AssertLength(key);
		writer.Key(key.data(), static_cast<rapidjson::SizeType>(key.length()));
	}

	template<class Writer>
	inline void WriteString(Writer &writer, std::wstring_view str)
	{
		AssertLength(str);
		writer.String(str.data(), static_cast<rapidjson::SizeType>(str.length()));
	}

	template<class Writer>
	inline void Serialize(Writer &writer, bool value, std::wstring_view key)
	{
		WriteKey(writer, key);
		writer.Bool(value);
	}

	template<class Writer, class T, std::size_t size>
	requires std::is_enum_v<T>
	inline void Serialize(Writer &writer, const T &member, std::wstring_view key, const std::array<std::wstring_view, size> &arr)
	{
		WriteKey(writer, key);
		WriteString(writer, arr.at(static_cast<std::size_t>(member)));
	}

	template<class Writer, class T>
	requires std::is_class_v<T>
	inline void Serialize(Writer &writer, const T &member, std::wstring_view key)
	{
		WriteKey(writer, key);
		writer.StartObject();
		member.Serialize(writer);
		writer.EndObject();
	}

	// TODO: write unit test
	inline void Deserialize(const rapidjson::GenericValue<rapidjson::UTF16LE<>> &obj, bool &member, std::wstring_view key)
	{
		if (const auto it = obj.FindMember(StringViewToValue(key)); it != obj.MemberEnd())
		{
			const auto &value = it->value;
			EnsureType(rapidjson::Type::kFalseType, value.GetType(), key);

			member = value.GetBool();
		}
	}

	template<class T, std::size_t size>
	inline void Deserialize(const rapidjson::GenericValue<rapidjson::UTF16LE<>> &obj, T &member, std::wstring_view key, const std::array<std::wstring_view, size> &arr)
	{
		if (const auto it = obj.FindMember(StringViewToValue(key)); it != obj.MemberEnd())
		{
			const auto &value = it->value;
			EnsureType(rapidjson::Type::kStringType, value.GetType(), key);

			const auto str = ValueToStringView(value);
			if (const auto it2 = std::find(arr.begin(), arr.end(), str); it2 != arr.end())
			{
				member = static_cast<T>(it2 - arr.begin());
			}
			else
			{
				throw RapidJSONHelper::DeserializationError {
					fmt::format(FMT_STRING(L"Found invalid enum string \"{}\" while deserializing key \"{}\""), str, key)
				};
			}
		}
	}

	template<class T>
	inline void Deserialize(const rapidjson::GenericValue<rapidjson::UTF16LE<>> &obj, T &member, std::wstring_view key)
	{
		if (const auto it = obj.FindMember(StringViewToValue(key)); it != obj.MemberEnd())
		{
			const auto &value = it->value;
			EnsureType(rapidjson::Type::kObjectType, value.GetType(), key);

			member.Deserialize(value);
		}
	}
}
