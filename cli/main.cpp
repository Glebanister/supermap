#include <iostream>

#include "CommandLineInterface.hpp"
#include "KvsCommandHandler.hpp"
#include "core/BST.hpp"
#include "io/DiskFileManager.hpp"
#include "core/DefaultBuilder.hpp"
#include "primitive/Key.hpp"
#include "primitive/ByteArray.hpp"
#include "builder/KeyValueStorageBuilder.hpp"

int main(int, const char **) {
    supermap::cli::CommandLineInterface cli("supermap");

    using SupermapBuilder = supermap::DefaultBuilder<supermap::Key<1>, supermap::ByteArray<1>, std::uint64_t>;

    using K = typename SupermapBuilder::K;
    using V = typename SupermapBuilder::V;
    using B = typename SupermapBuilder::B;

    std::shared_ptr<supermap::KeyValueStorage<K, V, B>> kvs = SupermapBuilder::build(
        SupermapBuilder::BuildParameters{
            3,
            0.5,
            "supermap"
        }
    );

    cli.addCommand("add", std::make_shared<supermap::cli::AddKeyHandler<K, V, B>>(kvs));
    cli.addCommand("remove", std::make_shared<supermap::cli::RemoveKeyHandler<K, V, B>>(kvs));
    cli.addCommand("contains", std::make_shared<supermap::cli::ContainsKeyHandler<K, V, B>>(kvs));
    cli.addCommand("get", std::make_shared<supermap::cli::GetValueHandler<K, V, B>>(kvs));

    cli.run(std::cin, std::cout);
}
