#pragma once

#include <string>

#include "KeyValueStorage.hpp"

namespace supermap {
class BST : public KeyValueStorage {
  public:
    void addKey(const std::string &key, std::string value) override;

    void removeKey(const std::string &key) override;

    bool containsKey(const std::string &key) override;

    std::string getValue(const std::string &key) override;

};
} // supermap
