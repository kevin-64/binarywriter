#pragma once

namespace KDB
{
	namespace Primitives
	{
		enum class FieldType
		{
			String,
			Integer,
			Boolean,
			UUID,
			Object = 0xF0
		};
	}
}
