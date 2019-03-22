#include "shortcuts.h"
#include "Utilities.h"
#include <iostream>
#include <fstream>

namespace KDB::Utilities
{
	void push_varint(std::vector<char>& vec, unsigned long long value, char size)
	{
		switch (size) {
		case 4:
			push_int(vec, static_cast<int>(value));
			break;
		case 8:
			push_int64(vec, value);
			break;
		case 2:
			push_ushort(vec, static_cast<unsigned short>(value));
			break;
		}
	}

	void push_ushort(vector<char>& vec, unsigned short value)
	{
		vec.push_back(static_cast<char>(value & 0x00FF));
		vec.push_back(static_cast<char>((value & 0xFF00) >> 8));
	}

	void push_int(vector<char>& vec, int value)
	{
		vec.push_back(static_cast<char>(value & 0x000000FF));
		vec.push_back(static_cast<char>((value & 0x0000FF00) >> 8));
		vec.push_back(static_cast<char>((value & 0x00FF0000) >> 16));
		vec.push_back(static_cast<char>((value & 0xFF000000) >> 24));
	}

	void push_ulong(vector<char>& vec, unsigned long value)
	{
		vec.push_back(static_cast<char>(value & 0x000000FF));
		vec.push_back(static_cast<char>((value & 0x0000FF00) >> 8));
		vec.push_back(static_cast<char>((value & 0x00FF0000) >> 16));
		vec.push_back(static_cast<char>((value & 0xFF000000) >> 24));
	}

	void push_int64(vector<char>& vec, int64 value)
	{
		vec.push_back(static_cast<char>(value  & 0x00000000000000FF));
		vec.push_back(static_cast<char>((value & 0x000000000000FF00) >> 8));
		vec.push_back(static_cast<char>((value & 0x0000000000FF0000) >> 16));
		vec.push_back(static_cast<char>((value & 0x00000000FF000000) >> 24));
		vec.push_back(static_cast<char>((value & 0x000000FF00000000) >> 32));
		vec.push_back(static_cast<char>((value & 0x0000FF0000000000) >> 40));
		vec.push_back(static_cast<char>((value & 0x00FF000000000000) >> 48));
		vec.push_back(static_cast<char>((value & 0xFF00000000000000) >> 56));
	}

	void push_string(vector<char>& vec, const string& value)
	{
		std::copy(begin(value), end(value), std::back_inserter(vec));
	}

	void push_stringview(vector<char>& vec, std::string_view value)
	{
		std::copy(begin(value), end(value), std::back_inserter(vec));
	}

	void push_vector(vector<char>& vec, const std::vector<char>& value)
	{
		std::copy(begin(value), end(value), std::back_inserter(vec));
	}

	void push_memory(vector<char>& vec, int size, void* memory)
	{
		//first we expand to vector to accommodate the extra data, then we obtain a pointer to the end
		//of the current data and copy the new memory there
		int currEnd = vec.size();
		vec.reserve(vec.size() + size);
		char* dest = vec.data() + vec.size();
		std::memcpy(dest, memory, size);
	}

	void writeover_int(std::vector<char>& vec, int position, int value)
	{
		char* dest = vec.data() + position;
		std::memcpy(dest, &value, sizeof(int));
	}

	void read_GUID(std::fstream& stream, GUID* value)
	{
		read_ulong(stream, &(value->Data1));

		read_ushort(stream, &(value->Data2));
		read_ushort(stream, &(value->Data3));

		stream.read(reinterpret_cast<char*>(&(value->Data4)), 8);
	}

	void read_char(std::fstream& stream, char* value)
	{
		stream.read(value, 1);
	}

	void read_varint(std::fstream& stream, unsigned long long* value, char size)
	{
		*value = 0; //blank all bytes first

		stream.read(&reinterpret_cast<char*>(value)[0], 1);
		stream.read(&reinterpret_cast<char*>(value)[1], 1);

		if (size == 2)
			return;

		stream.read(&reinterpret_cast<char*>(value)[2], 1);
		stream.read(&reinterpret_cast<char*>(value)[3], 1);

		if (size == 4)
			return;

		stream.read(&reinterpret_cast<char*>(value)[4], 1);
		stream.read(&reinterpret_cast<char*>(value)[5], 1);
		stream.read(&reinterpret_cast<char*>(value)[6], 1);
		stream.read(&reinterpret_cast<char*>(value)[7], 1);
	}

	void read_int64(std::fstream& stream, int64* value)
	{
		stream.read(&reinterpret_cast<char*>(value)[0], 1);
		stream.read(&reinterpret_cast<char*>(value)[1], 1);
		stream.read(&reinterpret_cast<char*>(value)[2], 1);
		stream.read(&reinterpret_cast<char*>(value)[3], 1);
		stream.read(&reinterpret_cast<char*>(value)[4], 1);
		stream.read(&reinterpret_cast<char*>(value)[5], 1);
		stream.read(&reinterpret_cast<char*>(value)[6], 1);
		stream.read(&reinterpret_cast<char*>(value)[7], 1);
	}

	void read_ulong(std::fstream& stream, unsigned long* value)
	{
		stream.read(&reinterpret_cast<char*>(value)[0], 1);
		stream.read(&reinterpret_cast<char*>(value)[1], 1);
		stream.read(&reinterpret_cast<char*>(value)[2], 1);
		stream.read(&reinterpret_cast<char*>(value)[3], 1);
	}

	void read_ushort(std::fstream& stream, unsigned short* value)
	{
		stream.read(&reinterpret_cast<char*>(value)[0], 1);
		stream.read(&reinterpret_cast<char*>(value)[1], 1);
	}

	void read_int(std::fstream& stream, int* value)
	{
		stream.read(&reinterpret_cast<char*>(value)[0], 1);
		stream.read(&reinterpret_cast<char*>(value)[1], 1);
		stream.read(&reinterpret_cast<char*>(value)[2], 1);
		stream.read(&reinterpret_cast<char*>(value)[3], 1);
	}
}
