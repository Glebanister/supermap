#include <iostream>

#include "CommandLineInterface.hpp"
#include "KvsCommandHandler.hpp"
#include "core/BST.hpp"
#include "io/DiskFileManager.hpp"

int main(int, const char **) {
    supermap::cli::CommandLineInterface cli("supermap");

    using MySupermap = supermap::Supermap<
        supermap::Key<1>,
        supermap::ByteArray<1>,
        std::uint32_t,
        4
    >;
    using RamType = supermap::BST<MySupermap::KeyType, MySupermap::IndexType, MySupermap::IndexType>;
    using SupermapKvs = supermap::KeyValueStorage<MySupermap::KeyType,
                                                  MySupermap::ValueType,
                                                  MySupermap::BoundsType>;

    using AddHandler = supermap::cli::AddKeyHandler<MySupermap::KeyType,
                                                    MySupermap::ValueType,
                                                    MySupermap::BoundsType>;
    using ContainsKeyHandler = supermap::cli::ContainsKeyHandler<MySupermap::KeyType,
                                                                 MySupermap::ValueType,
                                                                 MySupermap::BoundsType>;
    using GetValueHandler = supermap::cli::GetValueHandler<MySupermap::KeyType,
                                                           MySupermap::ValueType,
                                                           MySupermap::BoundsType>;

    std::shared_ptr<supermap::io::FileManager> fileManager = std::make_shared<supermap::io::DiskFileManager>();

    std::shared_ptr<SupermapKvs> kvs = std::make_shared<MySupermap>(
        std::make_unique<RamType>(),
        std::make_unique<MySupermap::DiskStorage>(
            "storage-not-sorted",
            "storage-sorted",
            fileManager
        ),
        [](MySupermap::IndexType notSortedSize, MySupermap::IndexType) {
            return notSortedSize >= 3;
        },
        []() {
            return std::make_unique<MySupermap::IndexList>();
        },
        4
    );

    cli.addCommand("add", std::make_shared<AddHandler>(kvs));
//    cli.addCommand("remove", std::make_shared<supermap::cli::RemoveKeyHandler<4, 100>>(kvs));
    cli.addCommand("contains", std::make_shared<ContainsKeyHandler>(kvs));
    cli.addCommand("get", std::make_shared<GetValueHandler>(kvs));

    cli.run(std::cin, std::cout);

}
