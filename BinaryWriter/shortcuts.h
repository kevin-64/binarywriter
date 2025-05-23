#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

template <typename T>
using vector = std::vector<T>;

using string = std::string;

template <typename T>
using shared_ptr = std::shared_ptr<T>;

template <typename T>
using unique_ptr = std::unique_ptr<T>;

template <typename TKey, typename TValue>
using map = std::map<TKey, TValue>;

using ios = std::ios;

using int64 = unsigned long long;