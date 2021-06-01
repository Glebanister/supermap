#include <algorithm>

#include "RamFileManager.hpp"
#include "exception/FileException.hpp"

namespace supermap::io {

std::deque<RamFileManager::File>::const_iterator RamFileManager::getFileIterator(
    const std::filesystem::path &file) const {
    auto it = getFileIteratorNoThrow(file);
    if (it == files.end()) {
        throw FileException(file, "Not found");
    }
    return it;
}

std::unique_ptr<InputStream> RamFileManager::getInputStream(const std::filesystem::path &filename,
                                                            std::uint64_t offset) {
    auto fileIt = getFileIterator(filename);
    return std::make_unique<StringInputStream>(fileIt->content, offset);
}

std::unique_ptr<OutputStream> RamFileManager::getOutputStream(const std::filesystem::path &filename, bool append) {
    auto fileIt = getFileIteratorNoThrow(filename);
    if (fileIt == files.end()) {
        files.push_back({filename, ""});
    }
    fileIt = getFileIterator(filename);
    std::size_t fileIndex = fileIt - files.begin();
    return std::make_unique<StringOutputStream>(files[fileIndex].content, append);
}

void RamFileManager::remove(const std::filesystem::path &path) {
    auto fileIt = getFileIterator(path);
    std::size_t fileIndex = fileIt - files.begin();
    files[fileIndex].content.clear();
    files[fileIndex].name = "";
}

void RamFileManager::rename(const std::filesystem::path &prev, const std::filesystem::path &next) {
    accessFile(getFileIterator(prev)).name = next;
}

void RamFileManager::swap(const std::filesystem::path &first, const std::filesystem::path &second) {
    auto firstIt = getFileIterator(first);
    auto secondIt = getFileIterator(second);
    std::swap(accessFile(firstIt).name, accessFile(secondIt).name);
}

std::deque<RamFileManager::File>::const_iterator RamFileManager::getFileIteratorNoThrow(const std::filesystem::path &path) const noexcept {
    return std::find_if(files.begin(), files.end(), [&path](const File &f) { return f.name == path; });
}

RamFileManager::File &RamFileManager::accessFile(const std::deque<RamFileManager::File>::const_iterator &it) {
    return files[it - files.begin()];
}

} // supermap::io
