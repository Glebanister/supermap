#pragma once

#include <memory>
#include <type_traits>

#include "CommandLineInterface.hpp"
#include "core/Supermap.hpp"

namespace supermap::cli {

template <std::size_t KeyLen, std::size_t ValueLen>
class KvsCommandHandler : public CommandLineInterface::Handler {
  public:
    explicit KvsCommandHandler(const std::string &info,
                               std::size_t argsLen,
                               std::shared_ptr<KeyValueStorage<KeyLen, ValueLen>> kvs)
        : CommandLineInterface::Handler(info),
          kvs_(std::move(kvs)),
          argsLen_(argsLen) {}

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
    std::shared_ptr<KeyValueStorage<KeyLen, ValueLen>> kvs_;
    const std::size_t argsLen_;
};

template <std::size_t KeyLen, std::size_t ValueLen>
class AddKeyHandler : public KvsCommandHandler<KeyLen, ValueLen> {
    using KvsCommandHandler<KeyLen, ValueLen>::kvs_;
  public:
    explicit AddKeyHandler(std::shared_ptr<KeyValueStorage<KeyLen, ValueLen>> kvs)
        : KvsCommandHandler<KeyLen, ValueLen>("Add key to keys", 2, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &) override {
        kvs_->add(Key<KeyLen>::fromString(args[0]), ByteArray<ValueLen>::fromString(std::string(args[1])));
    }
};

template <std::size_t KeyLen, std::size_t ValueLen>
class RemoveKeyHandler : public KvsCommandHandler<KeyLen, ValueLen> {
    using KvsCommandHandler<KeyLen, ValueLen>::kvs_;
  public:
    explicit RemoveKeyHandler(std::shared_ptr<KeyValueStorage<KeyLen, ValueLen>> kvs)
        : KvsCommandHandler<KeyLen, ValueLen>("Remove key from keys", 1, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &) override {
        kvs_->remove(Key<KeyLen>::fromString(args[0]));
    }
};

template <std::size_t KeyLen, std::size_t ValueLen>
class ContainsKeyHandler : public KvsCommandHandler<KeyLen, ValueLen> {
    using KvsCommandHandler<KeyLen, ValueLen>::kvs_;
  public:
    explicit ContainsKeyHandler(std::shared_ptr<KeyValueStorage<KeyLen, ValueLen>> kvs)
        : KvsCommandHandler<KeyLen, ValueLen>("Check if key in keys", 1, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override {
        os << (kvs_->containsKey(Key<KeyLen>::fromString(args[0])) ? "true" : "false") << std::endl;
    }
};

template <std::size_t KeyLen, std::size_t ValueLen>
class GetValueHandler : public KvsCommandHandler<KeyLen, ValueLen> {
    using KvsCommandHandler<KeyLen, ValueLen>::kvs_;
  public:
    explicit GetValueHandler(std::shared_ptr<KeyValueStorage<KeyLen, ValueLen>> kvs)
        : KvsCommandHandler<KeyLen, ValueLen>("Get value of key from keys", 1, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override {
        os << kvs_->getValue(Key<KeyLen>::fromString(args[0])).toString() << std::endl;
    }
};

} // supermap::cli
