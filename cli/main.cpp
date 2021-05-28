#include <iostream>

#include "CommandLineInterface.hpp"
#include "KvsCommandHandler.hpp"
#include "core/BST.hpp"

int main(int, const char **) {
    supermap::cli::CommandLineInterface cli("supermap");
    std::shared_ptr<supermap::KeyValueStorage<4, 100>> kvs = std::make_shared<supermap::BST<4, 100>>();

    cli.addCommand("add", std::make_shared<supermap::cli::AddKeyHandler<4, 100>>(kvs));
    cli.addCommand("remove", std::make_shared<supermap::cli::RemoveKeyHandler<4, 100>>(kvs));
    cli.addCommand("contains", std::make_shared<supermap::cli::ContainsKeyHandler<4, 100>>(kvs));
    cli.addCommand("get", std::make_shared<supermap::cli::GetValueHandler<4, 100>>(kvs));

    cli.run(std::cin, std::cout);

}
