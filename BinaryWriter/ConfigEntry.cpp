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
		std::swap(lhs.m_dataSize, rhs.m_dataSize);
		std::swap(lhs.m_data, rhs.m_data);
		std::swap(lhs.m_size, rhs.m_size);
	}

	ConfigEntry::ConfigEntry()
	{
	}

	ConfigEntry::ConfigEntry(int64 key, int data)
		: m_size(0), m_key(key), m_dataSize(sizeof(int))
	{
		Utilities::push_int(m_data, data);
	}

	ConfigEntry::ConfigEntry(int64 key, const std::string& data)
		: m_size(0), m_key(key), m_dataSize(data.length())
	{
		Utilities::push_string(m_data, data);
	}

	ConfigEntry::ConfigEntry(int64 key, std::vector<char>&& data)
		: m_size(0), m_key(key), m_data(std::move(data))
	{
		m_dataSize = m_data.size();
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

	/* Format of a "ConfigEntry" record:
	 * 0:	     ConfigEntry record identifier (0xF0)
	 * 1-8:      Configuration key: this is used to distinguish settings
	 * 9-12:	 Size of the actual configuration data (S)
	 * 13-S+13   Configuration value; its format is decided for each individual setting
	 */
	//Return by-value is efficient as the variable is local thanks to copy elision/RVO
	std::vector<char> ConfigEntry::getData() const
	{
		vector<char> data;
//disabilitiamo gli warning per il troncamento
#pragma warning( disable : 4305 4309)
		data.push_back(RecordType::CONFIG_RECORD);

		Utilities::push_int64(data, this->m_key);
		Utilities::push_int(data, this->m_dataSize);
		Utilities::push_vector(data, this->m_data);

		return data;
	}

	int ConfigEntry::getSize() const
	{
		//the size is cached for performance
		if (this->m_size != 0)
			return this->m_size;

		int size = 13;									//record ID, key size and value size
		size += this->m_dataSize;						//size of the data for this entry

		return size;
	}

	int ConfigEntry::getIntValue()
	{
		return *reinterpret_cast<int*>(this->m_data.data());
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

	void skipConfigEntry(std::fstream& stream)
	{
		stream.ignore(9); //we skip the key

		int length;
		Utilities::read_int(stream, &length);

		stream.ignore(length); //the actual configuration is not read to avoid needless allocations
	}
}