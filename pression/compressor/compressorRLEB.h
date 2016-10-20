
/* Copyright (c) 2010-2016, Cedric Stalder <cedric.stalder@gmail.com>
 *                          Stefan Eilemann <eile@eyescale.ch>
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

#ifndef PRESSION_PLUGIN_COMPRESSORRLEB
#define PRESSION_PLUGIN_COMPRESSORRLEB

#include <pression/dataCompressor.h>

namespace pression
{
namespace plugin
{

class CompressorRLEB : public DataCompressor
{
public:
    CompressorRLEB() : DataCompressor() {}
    virtual ~CompressorRLEB() {}

    size_t getCompressBound( const size_t size ) const override
        { return size << 1; }
    void compress( const uint8_t* data, size_t size, Result& output ) override;
    void decompress( const Result& input, uint8_t* const data,
                     size_t size ) override;
};
}
}
#endif
