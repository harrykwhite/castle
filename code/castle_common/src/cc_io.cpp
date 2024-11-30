#include <castle_common/cc_io.h>

namespace cc
{

const char *extract_filename_from_path(const char *const filePath)
{
    const char *const backslash = strrchr(filePath, '\\');
    const char *const slash = strrchr(filePath, '/');

    const char *const filePathEnd = (backslash > slash) ? backslash : slash;

    return filePathEnd ? filePathEnd + 1 : filePath;
}

int extract_filename_from_path_no_ext(const char *const filePath, char *const dest, const int destSize)
{
    const char *const filename = extract_filename_from_path(filePath);
    const char *const ext = strrchr(filename, '.');

    const int idealLen = ext ? ext - filename : strlen(filename);
    const int restrLen = std::min(idealLen, destSize - 1);

    memcpy(dest, filename, restrLen);
    dest[restrLen] = '\0';

    return restrLen;
}

}
