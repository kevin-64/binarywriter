#pragma once

using int64 = unsigned long long;

/*
 * List of keys used to find specific configurations in the config file;
 * these are NOT the values of those configurations, only the record keys.
 */
namespace KDB::Constants::ConfigurationKeys
{
	//size of the address in pointers; determines how many pointers are available
	const int64 ADDRESS_SIZE = 1;			

	//size of the block ID; determines how many blocks are addressable
	const int64 BLOCK_ID_SIZE = 2;

	//size of the offset in pointers; determines how many locations in a block are addressable
	const int64 OFFSET_SIZE = 3;
}