#pragma once

#include "IDBRecord.h"
#include "ConfigSettings.h"
#include "RecordType.h"
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>

namespace KDB::Binary
{
	class FileWriter
	{
	private:
		KDB::Primitives::ConfigSettings* m_settings;
		std::string m_fileName;
		std::fstream m_stream;
		FileWriter() = default;
	public:
		FileWriter(KDB::Primitives::ConfigSettings* settings, std::string_view filename);
		virtual ~FileWriter();

		FileWriter(const FileWriter&) = delete;
		FileWriter& operator=(const FileWriter&) = delete;

		friend void swapFileWriters(FileWriter& lhs, FileWriter& rhs) noexcept;

		FileWriter(FileWriter&&) noexcept;
		FileWriter& operator=(FileWriter&&) noexcept;

		bool writeRecord(const Contracts::IDBRecord& record);
		std::unique_ptr<Contracts::IDBRecord> readRecord(long long offset);
	};
}