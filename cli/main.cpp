#include <iostream>
#include <memory>

#include "CommandLineInterface.hpp"
#include "KvsCommandHandler.hpp"
#include "core/BST.hpp"
#include "io/DiskFileManager.hpp"
#include "builder/DefaultSupermap.hpp"
#include "primitive/Key.hpp"
#include "primitive/ByteArray.hpp"
#include "builder/KeyValueStorageBuilder.hpp"
#include "builder/DefaultFilteredKvs.hpp"

int main(int, const char **) {
    supermap::cli::CommandLineInterface cli("supermap");

    using K = supermap::Key<1>;
    using V = supermap::ByteArray<1>;
    using I = std::uint64_t;
    using MaybeV = supermap::MaybeRemovedValue<V>;

    using SupermapBuilder = supermap::DefaultSupermap<K, MaybeV, I>;

    auto supermapParams = SupermapBuilder::BuildParameters{
        3,
        0.5,
        "supermap"
    };

    auto backendKvs = SupermapBuilder::build(
        std::make_unique<supermap::BST<K, I, I>>(),
        supermapParams
    );

    std::unique_ptr<supermap::KeyValueStorage<K, V, I>> kvs =
        supermap::builder::fromKvs<K, MaybeV, I>(std::move(backendKvs))
            .removable()
            .filtered(std::make_unique<supermap::BloomFilter<K>>())
            .build();

    std::shared_ptr<supermap::KeyValueStorage<K, V, I>>
        shared = std::shared_ptr<supermap::KeyValueStorage<K, V, I>>(kvs.release());

    cli.addCommand("add", std::make_shared<supermap::cli::AddKeyHandler<K, V, I>>(shared));
    cli.addCommand("remove", std::make_shared<supermap::cli::RemoveKeyHandler<K, V, I>>(shared));
    cli.addCommand("contains", std::make_shared<supermap::cli::ContainsKeyHandler<K, V, I>>(shared));
    cli.addCommand("get", std::make_shared<supermap::cli::GetValueHandler<K, V, I>>(shared));

    cli.run(std::cin, std::cout);
}
