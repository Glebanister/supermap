#pragma once

#include <string>

#include "KeyValueStorage.hpp"

namespace supermap {

class BST : public KeyValueStorage {
  public:
    void add(const std::string &key, std::string&& value) override;

    void remove(const std::string &key) override;

    bool containsKey(const std::string &key) override;

    std::string getValue(const std::string &key) override;

};

} // supermap
