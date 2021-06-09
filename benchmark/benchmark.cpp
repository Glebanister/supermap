#include <iostream>
#include <cstdlib>
#include <string>
#include <random>

#include "core/BST.hpp"
#include "builder/DefaultSupermap.hpp"
#include "primitive/Key.hpp"
#include "primitive/ByteArray.hpp"
#include "builder/KeyValueStorageBuilder.hpp"
#include "builder/DefaultFilteredKvs.hpp"

class Benchmark {
    static constexpr int KEY_SIZE = 30;
    static constexpr int VALUE_SIZE = 100;

    using K = supermap::Key<KEY_SIZE>;
    using V = supermap::ByteArray<VALUE_SIZE>;
    using KV = supermap::KeyValue<K, V>;
    using I = std::uint64_t;
    using MaybeV = supermap::MaybeRemovedValue<V>;
    using SupermapBuilder = supermap::DefaultSupermap<K, MaybeV, I>;

  public:
    struct BenchmarkResult {
        std::uint64_t averageRequestTimeInMicros;
        std::uint64_t averageGetRequestTimeInMicros;
        std::uint64_t averageAddRequestTimeInMicros;
        std::uint64_t minGetRequestTimeInMicros;
        std::uint64_t maxGetRequestTimeInMicros;
        std::uint64_t minAddRequestTimeInMicros;
        std::uint64_t maxAddRequestTimeInMicros;
    };
    BenchmarkResult benchmarkKVS(std::int64_t initialNumberOfElements,
                                 std::int64_t numberOfRequests,
                                 std::int64_t percentageOfGetRequests) {

        auto supermapParams = SupermapBuilder::BuildParameters{
                static_cast<unsigned long>(initialNumberOfElements / 10),
                0.5,
                "supermap",
                1 / 32.0
        };

        auto backendKvs = SupermapBuilder::build(
                std::make_unique<supermap::BST<K, I, I>>(),
                supermapParams
        );

        kvs_ = supermap::builder::fromKvs<K, MaybeV, I>(std::move(backendKvs))
                        .removable()
                        .build();

        for (std::int64_t k = 0; k < initialNumberOfElements; ++k) {
            std::string keyString = std::to_string(k);
            std::string valueString = keyString;
            keyString.resize(KEY_SIZE);
            valueString.resize(VALUE_SIZE);
            K key = K::fromString(keyString);
            V value = V::fromString(valueString);
            kvs_->add(key, std::move(value));
        }

        std::uint64_t minGetRequestTimeInMicros = UINT64_MAX;
        std::uint64_t maxGetRequestTimeInMicros = 0;

        std::uint64_t minAddRequestTimeInMicros = UINT64_MAX;
        std::uint64_t maxAddRequestTimeInMicros = 0;

        std::uint64_t sumRequestsTimeInMicros = 0;

        std::uint64_t sumAddRequestsTimeInMicros = 0;
        std::uint64_t sumGetRequestsTimeInMicros = 0;

        const auto numberOfGetRequests = static_cast<std::uint64_t>(std::round((double)numberOfRequests / 100.0 * (double)percentageOfGetRequests));
        std::uint64_t restNumberOfGetRequests = numberOfGetRequests;

        const std::uint64_t numberOfAddRequests = numberOfRequests - numberOfGetRequests;

        for (std::int64_t k = 0; k < numberOfRequests; ++k) {
            int x = distribution_(rnd_);
            bool isGetRequest = x <= percentageOfGetRequests && restNumberOfGetRequests > 0;
            std::uint64_t requestTimeInMicros;
            if (isGetRequest) {
                requestTimeInMicros = measureGetRequest();
                maxGetRequestTimeInMicros = std::max(requestTimeInMicros, maxGetRequestTimeInMicros);
                minGetRequestTimeInMicros = std::min(requestTimeInMicros, minGetRequestTimeInMicros);
                sumGetRequestsTimeInMicros += requestTimeInMicros;
                --restNumberOfGetRequests;
            } else {
                requestTimeInMicros = measureAddRequest();
                maxAddRequestTimeInMicros = std::max(requestTimeInMicros, maxAddRequestTimeInMicros);
                minAddRequestTimeInMicros = std::min(requestTimeInMicros, minAddRequestTimeInMicros);
                sumAddRequestsTimeInMicros += requestTimeInMicros;
            }
            sumRequestsTimeInMicros += requestTimeInMicros;
        }
        BenchmarkResult result{};
        result.averageRequestTimeInMicros = sumRequestsTimeInMicros / numberOfRequests;
        result.averageGetRequestTimeInMicros = sumGetRequestsTimeInMicros / numberOfGetRequests;
        result.averageAddRequestTimeInMicros = sumAddRequestsTimeInMicros / numberOfAddRequests;
        result.minGetRequestTimeInMicros = minGetRequestTimeInMicros;
        result.maxGetRequestTimeInMicros = maxGetRequestTimeInMicros;
        result.minAddRequestTimeInMicros = minAddRequestTimeInMicros;
        result.maxAddRequestTimeInMicros = maxAddRequestTimeInMicros;
        return result;
    }

  private:
    std::uint64_t measureAddRequest() {
        KV keyValue = getRandomKeyValue();
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        kvs_->add(keyValue.key, std::move(keyValue.value));
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    }

    std::uint64_t measureGetRequest() {
        std::string keyString = getRandomString();
        keyString.resize(KEY_SIZE);
        K key = K::fromString(keyString);
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        kvs_->getValue(key);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    }

    KV getRandomKeyValue() {
        std::string keyString = getRandomString();
        std::string valueString = keyString;
        keyString.resize(KEY_SIZE);
        valueString.resize(VALUE_SIZE);
        K key = K::fromString(keyString);
        V value = V::fromString(valueString);
        return KV{key, value};
    }

    std::string getRandomString() {
        return std::to_string(rnd_());
    }

  private:
    std::unique_ptr<supermap::KeyValueStorage<K, V, I>> kvs_;

    std::mt19937 rnd_{std::random_device{}()};
    std::uniform_int_distribution<int> distribution_{1, 100};
};

void printParametersErrorMessage() {
    std::cout << "Wrong parameters." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printParametersErrorMessage();
        return 0;
    }
    std::int64_t initialNumberOfElements = std::strtol(argv[1], nullptr, 10);
    std::int64_t numberOfRequests = std::strtol(argv[2], nullptr, 10);
    std::int64_t percentageOfGetRequests = std::strtol(argv[3], nullptr, 10);
    if (initialNumberOfElements < 0 ||
        numberOfRequests < 0 ||
        percentageOfGetRequests < 0 ||
        percentageOfGetRequests > 100) {
        printParametersErrorMessage();
        return 0;
    }

    Benchmark benchmark;
    auto result = benchmark.benchmarkKVS(initialNumberOfElements, numberOfRequests, percentageOfGetRequests);
    std::cout << "averageRequestTimeInMicros " << result.averageRequestTimeInMicros << std::endl;
    std::cout << "averageGetRequestTimeInMicros " << result.averageGetRequestTimeInMicros << std::endl;
    std::cout << "averageAddRequestTimeInMicros " << result.averageAddRequestTimeInMicros << std::endl;
    std::cout << "minGetRequestTimeInMicros " << result.minGetRequestTimeInMicros << std::endl;
    std::cout << "maxGetRequestTimeInMicros " << result.maxGetRequestTimeInMicros << std::endl;
    std::cout << "minAddRequestTimeInMicros " << result.minAddRequestTimeInMicros << std::endl;
    std::cout << "maxAddRequestTimeInMicros " << result.maxAddRequestTimeInMicros << std::endl;

    return 0;
}