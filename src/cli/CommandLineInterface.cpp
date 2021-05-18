#include "CommandLineInterface.hpp"

namespace supermap {

const std::string &CommandLineInterface::Handler::getInfo() const noexcept {
    return info_;
}

CommandLineInterface::CommandLineInterface(std::string name)
    : name_(std::move(name)) {
    addCommand("help", std::make_shared<HelpHandler>(*this));
    addCommand("stop", std::make_shared<StopHandler>(*this));
}

void CommandLineInterface::addCommand(const std::string &command, std::shared_ptr<Handler> handler) {
    handlers_[command] = std::move(handler);
}
const std::unordered_map<std::string,
                         std::shared_ptr<CommandLineInterface::Handler>> &CommandLineInterface::getHandlers() const {
    return handlers_;
}

void CommandLineInterface::HelpHandler::handle(const std::vector<std::string> &args, std::ostream &os) {
    for (auto &[cmd, handler] : interface_.getHandlers()) {
        os << "'" << cmd << "' - " << handler->getInfo() << '\n';
    }
    os.flush();
}

void CommandLineInterface::StopHandler::handle(const std::vector<std::string> &, std::ostream &os) {
    if (!interface_.running_) {
        os << "Interface has not been started" << std::endl;
        return;
    }
    interface_.stop();
}
} // supermap
