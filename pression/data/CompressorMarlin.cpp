
/* Copyright (c) 2016, Stefan Eilemann <eile@eyescale.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "CompressorMarlin.h"

#include <codecs/marlin.hpp>
#include <codecs/nibble.hpp>
#include <codecs/rice.hpp>
#include <codecs/rle.hpp>

#include <pression/data/Registry.h>

#include <lunchbox/buffer.h>

namespace pression
{
namespace data
{
namespace
{
const bool _initialized =
    Registry::getInstance().registerEngine<CompressorMarlin<Marlin, 9>>(
        {.552f, .180f});

struct Uncompressed : public UncompressedData8
{
    Uncompressed() {}
    Uncompressed(const uint8_t* const data, const size_t size) noexcept
    {
        for (size_t i = 0; i < size; i += BlockSizeBytes)
            emplace_back(&data[i],
                         std::min(size_t(size - i), size_t(BlockSizeBytes)));
    }

    void copy(uint8_t* data)
    {
        size_t index = 0;
        for (auto& i : *this)
        {
            memcpy(data + index, i.begin(), i.size());
            index += i.size();
        }
    }
};

struct Compressed : public CompressedData8
{
    Compressed() {}
    Compressed(const uint8_t* data, const size_t size)
    {
        // OPT me by accessing array directly to avoid string copy
        fromString(std::string((const char*)data, size));
    }

    void copy(Compressor::Result& result)
    {
        // OPT me by accessing array directly to avoid string copy
        const auto& data = toString();
        result.replace(data.data(), data.size());
    }
};

using CodecPtr = std::shared_ptr<CODEC8>;

template <class T, int level>
CodecPtr makeCodec()
{
    return std::make_shared<T>(Distribution::Laplace, Marlin::MARLIN, level);
}
}

template <class T, int level>
std::string CompressorMarlin<T, level>::getName()
{
    return std::string("pression::data::CompressorMarlin" +
                       std::to_string(level));
}

template <class T, int level>
void CompressorMarlin<T, level>::compressChunk(const uint8_t* const data,
                                               const size_t size,
                                               Result& output)
{
    if (!_initialized)
        return;

    auto compressor = makeCodec<T, level>();
    Uncompressed in(data, size);
    Compressed out;

    compressor->compress(in, out);
    out.copy(output);
}

template <class T, int level>
void CompressorMarlin<T, level>::decompressChunk(const uint8_t* const input,
                                                 const size_t inputSize,
                                                 uint8_t* const data,
                                                 const size_t)
{
    if (!_initialized)
        return;

    auto compressor = makeCodec<T, level>();
    Compressed in(input, inputSize);
    Uncompressed out;

    compressor->uncompress(in, out);
    out.copy(data);
}
}
}

template class pression::data::CompressorMarlin<Marlin, 9>;
