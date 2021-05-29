#include <algorithm>

#include "RamFileManager.hpp"
#include "exception/FileException.hpp"

namespace supermap::io {

std::deque<RamFileManager::File>::const_iterator RamFileManager::getFileIterator(
    const std::filesystem::path &file) const noexcept {

    return std::find_if(files.begin(), files.end(), [&file](const File &f) { return f.name == file; });
}

std::unique_ptr<InputStream> RamFileManager::getInputStream(const std::filesystem::path &filename,
                                                            std::uint64_t offset) {
    auto fileIt = getFileIterator(filename);
    if (fileIt == files.end()) {
        throw FileException(filename,
                            std::string() + "can not open virtual file: it was not created yet '" + filename.string()
                                + "'");
    }
    return std::make_unique<StringInputStream>(fileIt->content, offset);
}

std::unique_ptr<OutputStream> RamFileManager::getOutputStream(const std::filesystem::path &filename, bool append) {
    auto fileIt = getFileIterator(filename);
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

} // supermap::io
