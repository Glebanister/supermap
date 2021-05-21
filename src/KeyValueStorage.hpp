#pragma once

#include <string>

namespace supermap {

class KeyValueStorage {
  public:
    virtual void add(const std::string &key, std::string &&value) = 0;

    virtual void remove(const std::string &key) = 0;

    virtual bool containsKey(const std::string &key) = 0;

    virtual std::string getValue(const std::string &key) = 0;

    virtual ~KeyValueStorage() = default;
};

} // supermap
