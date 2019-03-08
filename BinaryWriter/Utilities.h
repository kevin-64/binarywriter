#pragma once
#include <string>

namespace KDB::Utilities
{
	//writing utilities
	inline void push_char(std::vector<char>& vec, char value)
	{
		vec.push_back(value);
	}

	void push_varint(std::vector<char>& vec, unsigned long long value, char size);
	void push_ushort(std::vector<char>& vec, unsigned short value);
	void push_int(std::vector<char>& vec, int value);
	void push_ulong(std::vector<char>& vec, unsigned long value);
	void push_int64(std::vector<char>& vec, unsigned long long value);
	void push_string(std::vector<char>& vec, const std::string& value);
	void push_stringview(std::vector<char>& vec, std::string_view value);
	void push_vector(std::vector<char>& vec, const std::vector<char>& value);

	//reading utilities
	void read_varint(std::fstream& stream, unsigned long long* value, char size);
	void read_int(std::fstream& stream, int* value);
	void read_int64(std::fstream& stream, unsigned long long* value);
	void read_ulong(std::fstream& stream, unsigned long* value);
	void read_ushort(std::fstream& stream, unsigned short* value);
}