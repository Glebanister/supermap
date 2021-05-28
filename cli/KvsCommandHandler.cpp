#include "KvsCommandHandler.hpp"
#include "exception/SupermapException.hpp"

namespace supermap {

KvsCommandHandler::KvsCommandHandler(const std::string &info, std::size_t argsLen, std::shared_ptr<KeyValueStorage> kvs)
    : CommandLineInterface::Handler(info),
      kvs_(std::move(kvs)),
      argsLen_(argsLen) {}

void KvsCommandHandler::assertArgsLen(std::size_t len, const std::vector<std::string> &args) {
    if (args.size() == len) return;
    throw SupermapException(std::string("Invalid number of arguments.") + std::to_string(len) + " expected.");
}

void AddKeyHandler::handleKvs(const std::vector<std::string> &args, std::ostream &os) {
    kvs_->add(args[0], std::string(args[1]));
}

AddKeyHandler::AddKeyHandler(std::shared_ptr<KeyValueStorage> kvs)
    : KvsCommandHandler("Add key to storage", 2, std::move(kvs)) {}

RemoveKeyHandler::RemoveKeyHandler(std::shared_ptr<KeyValueStorage> kvs)
    : KvsCommandHandler("Remove key from storage", 1, std::move(kvs)) {}

void RemoveKeyHandler::handleKvs(const std::vector<std::string> &args, std::ostream &os) {
    kvs_->remove(args[0]);
}

ContainsKeyHandler::ContainsKeyHandler(std::shared_ptr<KeyValueStorage> kvs)
    : KvsCommandHandler("Check if key in storage", 1, std::move(kvs)) {}

void ContainsKeyHandler::handleKvs(const std::vector<std::string> &args, std::ostream &os) {
    os << (kvs_->containsKey(args[0]) ? "true" : "false") << std::endl;
}

GetValueHandler::GetValueHandler(std::shared_ptr<KeyValueStorage> kvs)
    : KvsCommandHandler("Get value of key from storage", 1, std::move(kvs)) {}

void GetValueHandler::handleKvs(const std::vector<std::string> &args, std::ostream &os) {
    os << kvs_->getValue(args[0]) << std::endl;
}
}
