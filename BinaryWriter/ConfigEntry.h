#pragma once
#include "IDBRecord.h"
#include "Utilities.h"
#include <memory>

namespace KDB::Primitives
{
	/*
	* Describes an entry in the configuration file for the database.
	*/
	class ConfigEntry : public Contracts::IDBRecord
	{
	private:
		ConfigEntry();
		unsigned long long m_key;
		std::vector<char> m_data;
		int m_dataSize;
		int m_size;

	public:
		ConfigEntry(unsigned long long key, int data);
		ConfigEntry(unsigned long long key, const std::string& data);
		ConfigEntry(unsigned long long key, std::vector<char>&& data);
		virtual ~ConfigEntry() = default;

		friend void swapConfigEntry(ConfigEntry& lhs, ConfigEntry& rhs) noexcept;

		//move semantics are supported
		ConfigEntry(ConfigEntry&& other) noexcept;
		ConfigEntry& operator=(ConfigEntry&& rhs) noexcept;

		//no copy is supported
		ConfigEntry(const ConfigEntry&) = delete;
		ConfigEntry& operator=(const ConfigEntry&) = delete;

		virtual std::vector<char> getData() const override;
		virtual int getSize() const override;

		friend std::unique_ptr<ConfigEntry> buildConfigEntry(std::fstream& stream);
	};

	std::unique_ptr<ConfigEntry> buildConfigEntry(std::fstream& stream);
}