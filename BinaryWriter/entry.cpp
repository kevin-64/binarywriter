#include <string>
#include <iostream>
#include <chrono>
#include "core.h"
#include "ConfigEntry.h"
#include "Type.h"
#include "Field.h"
#include "FieldType.h"
#include "shortcuts.h"
#include "FileWriter.h"

using ConfigEntry = KDB::Primitives::ConfigEntry;
using Type = KDB::Primitives::Type;
using Field = KDB::Primitives::Field;
using FieldType = KDB::Primitives::FieldType;
using FileWriter = KDB::Binary::FileWriter;
using Core = KDB::Binary::Core;

using namespace std::string_literals;

void writeDef(Core&);
void readDef(Core&);

void writeConf(Core&);

void writePtr(Core&);
void readPtr(Core&);

int main()
{
	auto start = std::chrono::system_clock::now();

	Core core(R"(C:\Users\kevinik\Desktop\kdb_files.txt)");

	//writeConf(core);
	//writeDef(core);
	//readDef(core);
	//writePtr(core);
	//readPtr(core);
	auto end = std::chrono::system_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << diff.count();
	//while (true)
	//{
	//	string input;
	//	std::cin >> input;
	//	
	//	if (input == "DEF")
	//		def();
	//	else
	//		break;
	//}
	return 0;
}

void writeConf(Core& core)
{
	auto conf1 = ConfigEntry(1, 4);
	auto conf2 = ConfigEntry(2, 2);
	auto conf3 = ConfigEntry(3, 4);
	
	core.addConfigEntry(conf1);
	core.addConfigEntry(conf2);
	core.addConfigEntry(conf3);
}

void writeDef(Core& core)
{
	auto name("MyType"s);
	auto fName("MyField"s);
	auto fields = vector<Field>();
	fields.emplace_back(fName, FieldType::Integer);
	auto typeId = Guid();
	auto type = Type(name, std::move(fields), std::move(typeId));

	core.addType(type);
}

void readDef(Core& core)
{
	auto record = core.getType(0);
}

void readConf(Core& core)
{
	auto record = core.getConfigEntry(0);
}

void writePtr(Core& core)
{
	auto fmt = KDB::Primitives::PointerFormat{ 4, 2, 4 };
	auto ptr = KDB::Primitives::Pointer(fmt, 0xFFFFEEEE, 0xAABB, 0x11223344);
	core.addPointer(ptr);
}

void readPtr(Core& core)
{
	auto record = core.getPointer(0);
}