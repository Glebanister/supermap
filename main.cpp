#include <iostream>

#include "cli/CommandLineInterface.hpp"
#include "cli/KvsCommandHandler.hpp"

int main(int, const char **) {
    supermap::CommandLineInterface cli("supermap");
    std::shared_ptr<supermap::KeyValueStorage> kvs =
        std::make_shared<supermap::Supermap>(100, 200);

    cli.addCommand("add", std::make_shared<supermap::AddKeyHandler>(kvs));
    cli.addCommand("remove", std::make_shared<supermap::RemoveKeyHandler>(kvs));
    cli.addCommand("contains", std::make_shared<supermap::ContainsKeyHandler>(kvs));
    cli.addCommand("get", std::make_shared<supermap::GetValueHandler>(kvs));

    cli.run(std::cin, std::cout);
}
