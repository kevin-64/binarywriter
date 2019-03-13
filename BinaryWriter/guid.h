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
	friend bool operator==(const Guid& lhs, const Guid& rhs);

	Guid(Guid&& other) noexcept;
	Guid& operator=(Guid&& rhs) noexcept;

	Guid(const Guid& other);
	Guid& operator=(const Guid& rhs);

	std::string toString() const;
	std::vector<char> serialize() const;
};