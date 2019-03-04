#pragma once

namespace KDB::Primitives
{
	enum RecordType
	{
		BLOCK_START = 0x00,
	
		//standard records
		TYPE_DEFINITION = 0x01,
		MAIN_RECORD = 0x02,
		EMBEDDED_RECORD = 0x03,
		REMOTE_RECORD = 0x04,
		ADDRESS_ENTRY = 0x05,
		BULK_ENTRY = 0x06,

		//config records
		CONFIG_MASTER = 0xF0,
		CONFIG_ENTRY = 0xF1,
		
		//special records
		END_OF_FILE = 0xFE,
		DELETED_ENTRY = 0xFF
	};
}