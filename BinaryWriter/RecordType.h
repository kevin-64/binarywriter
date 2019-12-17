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
		OWNING_POINTER_RECORD = 0xA0,
		SHARED_POINTER_RECORD = 0xA1,
		REFERENCE_POINTER_RECORD = 0xA2,
		HOLD_POINTER_RECORD = 0xA3,

		//block definition records
		BLOCK_DEFINITION = 0xB0,
		BLOCK_PARTITION = 0xB1,

		//configuration records
		CONFIG_RECORD = 0xF0,
		
		//special records
		PADDING = 0xF1,
		END_OF_FILE = 0xFE,
		DELETED_ENTRY = 0xFF
	};
}