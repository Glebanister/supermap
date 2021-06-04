#pragma once

namespace supermap {

class Formattable {
  public:
    [[nodiscard]] virtual std::string toString() const = 0;

    virtual ~Formattable() = default;
};

} // supermap
