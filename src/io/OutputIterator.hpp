#pragma once

#include <memory>

#include "OutputStream.hpp"
#include "SerializeHelper.hpp"
#include "exception/IteratorException.hpp"

namespace supermap::io {

/**
 * @brief Iterator which allows to write continuous sequence of objects of type @p T to the output stream.
 * @tparam T Iterated object type.
 */
template <typename T, typename = std::enable_if_t<SerializeHelper<T>::isSerializable>>
class OutputIterator {
  public:
    /**
     * @brief Creates new @p OutputIterator.
     * @param os @p OutputStream ownership.
     */
    explicit OutputIterator(std::unique_ptr<OutputStream> &&os)
        : os_(std::move(os)) {}

    /**
     * @brief Creates OutputIterator of objects @p T to @p filename.
     * @param filename Name of file to write into.
     * @param append @p true if need to append to the end of file instead of overwriting file.
     * @return Created @p OutputIterator.
     */
    static OutputIterator toFile(const std::string &filename, bool append) {
        return OutputIterator(std::make_unique<FileOutputStream>(filename, append));
    }

    /**
     * @brief Creates OutputIterator of objects @p T to string @p buffer.
     * @param buffer String buffer to write into.
     * @param append @p true if need to append to the end of string instead of overwriting it.
     * @return Created @p OutputIterator.
     */
    static OutputIterator toString(std::string &buffer, bool append) {
        return OutputIterator(std::make_unique<StringOutputStream>(buffer, append));
    }

  public:
    /**
     * @brief Serializes @p obj to the output stream.
     * @param obj Object to write.
     */
    void write(const T &obj) {
        serialize(obj, os_->get());
    }

    /**
     * @brief Writes all objects from iterator @p begin to iterator @p end.
     * @tparam Iterator Type of iterator.
     * @tparam Functor Type of function which is applied to every object before writing.
     * @param begin Collection begin iterator.
     * @param end Collection end iterator.
     * @param f Functor which is applied to every object before writing to output stream.
     */
    template <
        typename Iterator,
        typename Functor,
        typename = std::enable_if_t<
            std::is_same_v<T,
                           std::invoke_result_t<Functor,
                                                typename std::iterator_traits<Iterator>::value_type>>>
    >
    void writeAll(Iterator begin, Iterator end, Functor f) {
        std::stringstream stringStream;
        for (auto it = begin; it < end; ++it) {
            serialize(f(*it), stringStream);
        }
        os_->get() << stringStream.rdbuf();
    }

    /**
     * @brief Flushes underlying output stream.
     */
    void flush() {
        os_->flush();
    }

    std::unique_ptr<OutputStream> os_;
};

} // supermap::io
