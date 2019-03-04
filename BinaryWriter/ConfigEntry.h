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
		unsigned int m_key;
		std::vector<char> m_data;
		int m_size;

	public:
		ConfigEntry(unsigned int key, int data);
		ConfigEntry(unsigned int key, std::string data);
		virtual ~ConfigEntry() = default;

		friend void swapType(ConfigEntry& lhs, ConfigEntry& rhs) noexcept;

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