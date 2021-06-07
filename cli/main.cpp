#include <iostream>

#include "CommandLineInterface.hpp"
#include "KvsCommandHandler.hpp"
#include "core/BST.hpp"
#include "io/DiskFileManager.hpp"
#include "core/DefaultSupermap.hpp"

int main(int, const char **) {
    supermap::cli::CommandLineInterface cli("supermap");

    using SupermapType = supermap::DefaultSupermap<1, 1, 10, 4, 3>;

    using K = typename SupermapType::K;
    using V = typename SupermapType::V;
    using I = typename SupermapType::B;

    std::shared_ptr<supermap::KeyValueStorage<K, V, I>> kvs = SupermapType::make();

    cli.addCommand("add", std::make_shared<supermap::cli::AddKeyHandler<K, V, I>>(kvs));
//    cli.addCommand("remove", std::make_shared<supermap::cli::RemoveKeyHandler<4, 100>>(kvs));
    cli.addCommand("contains", std::make_shared<supermap::cli::ContainsKeyHandler<K, V, I>>(kvs));
    cli.addCommand("get", std::make_shared<supermap::cli::GetValueHandler<K, V, I>>(kvs));

    cli.run(std::cin, std::cout);
}
