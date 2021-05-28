#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <istream>
#include <sstream>
#include <iterator>

namespace supermap::cli {
class CommandLineInterface {
  public:
    class Handler {
      public:
        explicit Handler(std::string info)
            : info_(std::move(info)) {}

        virtual void handle(const std::vector<std::string> &args, std::ostream &os) = 0;

        [[nodiscard]] const std::string &getInfo() const noexcept;

        virtual ~Handler() = default;

      private:
        const std::string info_;
    };

  private:
    class HelpHandler : public Handler {
      public:
        explicit HelpHandler(const CommandLineInterface &interface)
            : Handler("Get help"),
              interface_(interface) {}

        void handle(const std::vector<std::string> &args, std::ostream &os) override;

      private:
        const CommandLineInterface &interface_;
    };

    class StopHandler : public Handler {
      public:
        explicit StopHandler(CommandLineInterface &interface)
            : Handler("Stop interface"),
              interface_(interface) {}

        void handle(const std::vector<std::string> &args, std::ostream &os) override;

      private:
        CommandLineInterface &interface_;
    };

  public:
    explicit CommandLineInterface(std::string name);

    void addCommand(const std::string &command, std::shared_ptr<Handler> handler);

    const std::unordered_map<std::string, std::shared_ptr<Handler>> &getHandlers() const;

    void run(std::istream &is, std::ostream &os) {
        running_ = true;
        while (running_) {
            std::string input;
            os << name_ << "> ";
            os.flush();
            std::getline(is, input);
            std::istringstream iss(input);
            std::vector<std::string> args{std::istream_iterator<std::string>{iss},
                                          std::istream_iterator<std::string>{}};
            if (args.empty()) {
                continue;
            }
            std::string command = args[0];
            if (handlers_.find(command) == handlers_.end()) {
                os << "Unknown command" << std::endl;
                continue;
            }
            args.erase(args.begin(), args.begin() + 1);
            handlers_[command]->handle(args, os);
        }
    }

    void stop() {
        running_ = false;
    }

  private:
    const std::string name_;
    std::unordered_map<std::string, std::shared_ptr<Handler>> handlers_;
    bool running_ = false;
};
} // supermap::cli
