#pragma once

#include <memory>
#include <type_traits>

#include "CommandLineInterface.hpp"
#include "core/KeyValueStorage.hpp"
#include "primitive/Key.hpp"
#include "primitive/ByteArray.hpp"

namespace supermap::cli {

template <typename Key, typename Value, typename Index>
class KvsCommandHandler : public CommandLineInterface::Handler {
  public:
    explicit KvsCommandHandler(const std::string &info,
                               std::size_t argsLen,
                               std::shared_ptr<KeyValueStorage<Key, Value, Index>> kvs)
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
    std::shared_ptr<KeyValueStorage<Key, Value, Index>> kvs_;
    const std::size_t argsLen_;
};

template <typename Key, typename Value, typename Index>
class AddKeyHandler : public KvsCommandHandler<Key, Value, Index> {
    using KvsCommandHandler<Key, Value, Index>::kvs_;

  public:
    explicit AddKeyHandler(std::shared_ptr<KeyValueStorage<Key, Value, Index>> kvs)
        : KvsCommandHandler<Key, Value, Index>("Add key to keys", 2, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &) override {
        kvs_->add(Key::fromString(args[0]), Value::fromString(std::string(args[1])));
    }
};

template <typename Key, typename Value, typename Index>
class RemoveKeyHandler : public KvsCommandHandler<Key, Value, Index> {
    using KvsCommandHandler<Key, Value, Index>::kvs_;

  public:
    explicit RemoveKeyHandler(std::shared_ptr<KeyValueStorage<Key, Value, Index>> kvs)
        : KvsCommandHandler<Key, Value, Index>("Remove key from keys", 1, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &) override {
        kvs_->remove(Key::fromString(args[0]));
    }
};

template <typename Key, typename Value, typename Index>
class ContainsKeyHandler : public KvsCommandHandler<Key, Value, Index> {
    using KvsCommandHandler<Key, Value, Index>::kvs_;

  public:
    explicit ContainsKeyHandler(std::shared_ptr<KeyValueStorage<Key, Value, Index>> kvs)
        : KvsCommandHandler<Key, Value, Index>("Check if key in keys", 1, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override {
        os << (kvs_->contains(Key::fromString(args[0])) ? "true" : "false") << std::endl;
    }
};

template <typename Key, typename Value, typename Index>
class GetValueHandler : public KvsCommandHandler<Key, Value, Index> {
    using KvsCommandHandler<Key, Value, Index>::kvs_;

  public:
    explicit GetValueHandler(std::shared_ptr<KeyValueStorage<Key, Value, Index>> kvs)
        : KvsCommandHandler<Key, Value, Index>("Get value of key from keys", 1, std::move(kvs)) {}

    void handleKvs(const std::vector<std::string> &args, std::ostream &os) override {
        os << kvs_->getValueNonSafe(Key::fromString(args[0])).toString() << std::endl;
    }
};

} // supermap::cli
