#include "ConfigEntry.h"
#include "RecordType.h"
#include "shortcuts.h"
#include <iostream>
#include <fstream>

namespace KDB::Primitives
{
	void swapConfigEntry(ConfigEntry& lhs, ConfigEntry& rhs) noexcept
	{
		std::swap(lhs.m_key, rhs.m_key);
		std::swap(lhs.m_size, rhs.m_size);
		std::swap(lhs.m_data, rhs.m_data);
	}

	ConfigEntry::ConfigEntry()
	{
	}

	ConfigEntry::ConfigEntry(int64 key, int data)
		: m_key(key), m_size(sizeof(int))
	{
		Utilities::push_int(m_data, data);
	}

	ConfigEntry::ConfigEntry(int64 key, const std::string& data)
		: m_key(key), m_size(data.length())
	{
		Utilities::push_string(m_data, data);
	}

	ConfigEntry::ConfigEntry(int64 key, std::vector<char>&& data)
		: m_key(key), m_size(data.size()), m_data(std::move(data))
	{
	}

	ConfigEntry::ConfigEntry(ConfigEntry&& other) noexcept
		: ConfigEntry::ConfigEntry()
	{
		swapConfigEntry(*this, other);
	}

	ConfigEntry& ConfigEntry::operator=(ConfigEntry&& rhs) noexcept
	{
		if (&rhs == this)
			return *this;

		ConfigEntry cfgEntry(std::move(*this));
		swapConfigEntry(*this, cfgEntry);
		return *this;
	}

	//Return by-value is efficient as the variable is local thanks to copy elision/RVO
	std::vector<char> ConfigEntry::getData() const
	{
		vector<char> data;
		data.push_back(RecordType::CONFIG_RECORD);

		Utilities::push_int64(data, this->m_key);
		Utilities::push_char(data, static_cast<char>(this->m_size));
		Utilities::push_vector(data, this->m_data);

		return data;
	}

	int ConfigEntry::getSize() const
	{
		return this->m_size;
	}

	std::unique_ptr<ConfigEntry> buildConfigEntry(std::fstream& stream)
	{
		unsigned long long key;
		Utilities::read_int64(stream, &key);

		int length;
		Utilities::read_int(stream, &length);

		vector<char> data(length);
		stream.read(data.data(), length);

		return std::make_unique<ConfigEntry>(ConfigEntry(key, std::move(data)));
	}
}