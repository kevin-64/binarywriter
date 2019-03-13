#include <string>
#include <iostream>
#include <chrono>
#include "core.h"
#include "ConfigEntry.h"
#include "Type.h"
#include "Field.h"
#include "FieldType.h"
#include "BlockDefinition.h"
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
std::unique_ptr<KDB::Contracts::IDBRecord> readDef(Core&);

void writeConf(Core&);

void writePtr(Core&);
void readPtr(Core&);

void writeBlock(Core&, const Guid&);
void readBlock(Core&);

void seekBlock(Core&, Guid);

int main()
{
	auto start = std::chrono::system_clock::now();

	Core core(R"(C:\Users\kevinik\Desktop\kdb_files.txt)");

	//writeConf(core);
	//writeDef(core);
	auto tydef = dynamic_cast<KDB::Primitives::Type*>(readDef(core).release());
	seekBlock(core, tydef->getTypeId());
	//writeBlock(core, tydef->getTypeId());
	//writePtr(core);
	//readPtr(core);
	//readBlock(core);
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

auto readDef(Core& core) -> decltype(core.getType(0))
{
	return core.getType(0);
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

void writeBlock(Core& core, const Guid& typeId)
{
	GUID guid;
	CoCreateGuid(&guid);
	Guid blockID(std::move(guid));

	Guid typeID(typeId);

	std::vector<KDB::Primitives::PartitionDefinition*> parts;
	auto part1 = new KDB::Primitives::PartitionDefinition(0, 0, 1, 0);
	parts.emplace_back(part1);

	KDB::Primitives::BlockDefinition bd(blockID, typeID, std::move(parts));
	core.addBlock(bd);
}

void readBlock(Core& core)
{
	auto record = core.getBlock(0);
}

void seekBlock(Core& core, Guid guid) 
{
	auto block = core.seekBlock(guid);
}