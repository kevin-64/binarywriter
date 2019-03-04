#pragma once

#include <objbase.h>
#include <memory>
#include <vector>

class Guid
{
private:
	GUID m_guid;
public:
	Guid();
	explicit Guid(GUID&& from);
	virtual ~Guid() = default;

	friend void swapGuid(Guid& lhs, Guid& rhs) noexcept;

	Guid(Guid&& other) noexcept;
	Guid& operator=(Guid&& rhs) noexcept;

	Guid(Guid& other) = delete;
	Guid& operator=(Guid& rhs) = delete;

	std::unique_ptr<std::string> toString() const;
	std::vector<char> serialize() const;
};