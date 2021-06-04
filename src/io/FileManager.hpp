#pragma once

#include <filesystem>

#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "InputIterator.hpp"
#include "OutputIterator.hpp"

namespace supermap::io {

/**
 * @brief File system explorer.
 */
class FileManager {
  public:
    /**
     * @brief Gets input stream from file @p filename.
     * @param filename Name of file to read from.
     * @param offset reading begin offset.
     * @return Unique pointer at created @p InputStream.
     */
    virtual std::unique_ptr<InputStream> getInputStream(const std::filesystem::path &path, std::uint64_t offset) = 0;

    /**
     * @brief Gets output stream to the @p path.
     * @param path Path to the file to write into.
     * @param append @p true if need to append to file instead of overwrite its content.
     * @return Unique pointer at created @p OutputStream.
     */
    virtual std::unique_ptr<OutputStream> getOutputStream(const std::filesystem::path &path, bool append) = 0;

    /**
     * @brief Removes @p path file from file system.
     * Guaranteed that file won't be in the file system after call.
     * @param path Removed file path.
     */
    virtual void remove(const std::filesystem::path &path) = 0;

    /**
     * @brief Renames file form @p prevPath to @p nextPath.
     * @p prevPath must be already in file system to perform this action.
     * @param prevPath Previous file name.
     * @param nextPath Next file name.
     */
    virtual void rename(const std::filesystem::path &prevPath, const std::filesystem::path &nextPath) = 0;

    /**
     * @brief Clears file with name @p path.
     * @param path File to clear.
     */
    void clear(const std::filesystem::path &path) {
        remove(path);
        create(path);
    }

    /**
     * @brief Creates file with name @p path.
     * @param path New file name. @p path must be absent before call.
     */
    void create(const std::filesystem::path &path) {
        getOutputStream(path, false)->flush();
    }

    /**
     * @brief Swaps contents of files.
     * @param first First file path.
     * @param second Second file path.
     */
    virtual void swap(const std::filesystem::path &first, const std::filesystem::path &second) = 0;

    /**
     * @brief Gets output iterator which writes objects of type @p T to file @p filename.
     * @tparam T Iterated object type.
     * @param filename File to write into.
     * @param append @p true if need to append to the end instead of overwriting file.
     * @return T output iterator.
     */
    template <typename T>
    OutputIterator<T> getOutputIterator(const std::filesystem::path &filename, bool append) {
        return OutputIterator<T>(getOutputStream(filename, append));
    }

    /**
     * @brief Gets input iterator which reads objects of type @p T from file @p filename.
     * @tparam T Iterated object type.
     * @param filename File to read from.
     * @param offset Specifies reading begin offset.
     * @return T input iterator.
     */
    template <typename T, typename IndexT>
    InputIterator<T, IndexT> getInputIterator(const std::filesystem::path &filename, std::uint64_t offset) {
        return InputIterator<T, IndexT>(getInputStream(filename, offset));
    }

    virtual ~FileManager() = default;
};

} // supermap
