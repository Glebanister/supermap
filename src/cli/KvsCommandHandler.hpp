#pragma once

#include <memory>
#include <type_traits>

#include "CommandLineInterface.hpp"
#include "Supermap.hpp"

namespace supermap {

class KvsCommandHandler : public CommandLineInterface::Handler {
  public:
    explicit KvsCommandHandler(const std::string &info,
                               std::size_t argsLen,
                               std::shared_ptr<KeyValueStorage> kvs);

    static void assertArgsLen(std::size_t len, const std::vector<std::string> &args);

    void handle(const std::vector<std::string> &args, std::ostream &os) final {
        try {
            if (args.size() < argsLen_) {
                throw SupermapException(
                    "Invalid number of arguments, at least " + std::to_string(argsLen_) + " expected");
            }
            handleKvs(args, os);
        } catch (const SupermapException &supermapException) {
            os << supermapException.what() << std::endl;
        } catch (const std::exception &e) {
            os << "Critical: " << e.what() << std::endl;
        } catch (...) {
            os << "Unknown error has occurred" << std::endl;
        }
    }

    virtual void handleKvs(const std::vector<std::string> &args, std::ostream &os) = 0;

  protected:
    std::shared_ptr<KeyValueStorage> kvs_;
    const std::size_t argsLen_;
};

class AddKeyHandler : public KvsCommandHandler {
  public:
    explicit AddKeyHandler(std::shared_ptr<KeyValueStorage> kvs);

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override;
};

class RemoveKeyHandler : public KvsCommandHandler {
  public:
    explicit RemoveKeyHandler(std::shared_ptr<KeyValueStorage> kvs);

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override;
};

class ContainsKeyHandler : public KvsCommandHandler {
  public:
    explicit ContainsKeyHandler(std::shared_ptr<KeyValueStorage> kvs);

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override;
};

class GetValueHandler : public KvsCommandHandler {
  public:
    explicit GetValueHandler(std::shared_ptr<KeyValueStorage> kvs);

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override;
};

} // supermap
