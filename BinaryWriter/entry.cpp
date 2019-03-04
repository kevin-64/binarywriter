#include <string>
#include <iostream>
#include <chrono>
#include "core.h"
#include "Type.h"
#include "Field.h"
#include "FieldType.h"
#include "shortcuts.h"
#include "FileWriter.h"

using Type = KDB::Primitives::Type;
using Field = KDB::Primitives::Field;
using FieldType = KDB::Primitives::FieldType;
using FileWriter = KDB::Binary::FileWriter;
using Core = KDB::Binary::Core;

using namespace std::string_literals;

void writeDef(Core&);
void readDef(Core&);

int main()
{
	auto start = std::chrono::system_clock::now();

	Core core(R"(C:\Users\kevinik\Desktop\kdb_files.txt)");

	writeDef(core);
	//readDef(core);
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