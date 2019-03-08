#pragma once

namespace KDB::Primitives
{
	enum RecordType
	{	
		//standard records
		TYPE_DEFINITION = 0x01,
		MAIN_RECORD = 0x02,
		EMBEDDED_RECORD = 0x03,
		REMOTE_RECORD = 0x04,
		BULK_ENTRY = 0x05,

		//addressing records
		POINTER_RECORD = 0xA0,

		//block definition records
		BLOCK_DEFINITION = 0xB0,
		BLOCK_PARTITION = 0xB1,

		//configuration records
		CONFIG_RECORD = 0xF0,
		
		//special records
		END_OF_FILE = 0xFE,
		DELETED_ENTRY = 0xFF
	};
}