#pragma once

#include <stdlib.h>
#include <tuple>

#include "lz4/lz4.h"
#include "lz4/lz4frame.h"
#include "lz4/lz4hc.h"

std::tuple<char*, size_t> Compress(char* srcData, size_t size)
{
    char* compressionBuffer = (char*)malloc(size);
    size_t compressed_size = LZ4_compress_default(srcData, compressionBuffer, size, size);
    compressionBuffer = (char*)realloc(compressionBuffer, compressed_size);

    return std::make_tuple(compressionBuffer, compressed_size);
}
