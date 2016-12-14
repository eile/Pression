
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
 *
 * This file is part of Pression <https://github.com/Eyescale/Pression>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
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

#include "Compressor.h"

#include <algorithm>
#include <stdexcept>
#ifdef PRESSION_USE_OPENMP
#  include <omp.h>
#endif

namespace pression
{
namespace data
{
namespace
{
const size_t PAGESIZE = LB_4KB;
const size_t PAGESIZE_2 = (PAGESIZE >> 1);

size_t _getClosestPageSize( size_t position, const size_t maxSize )
{
    const size_t pageNumber = (position + PAGESIZE_2) / PAGESIZE;
    position = pageNumber * PAGESIZE;
    return std::min( position, maxSize );
}
}

Compressor::~Compressor()
{
    if( _in > 0 )
        LBDEBUG << _in << " -> " << _out << " ("
                << int( float(_out) / float(_in) * 100.f ) << "%) " << std::endl;
}

const Compressor::Results& Compressor::compress( const uint8_t* data,
                                                 size_t size )
{
#ifdef PRESSION_USE_OPENMP
    const size_t nPages = (size + PAGESIZE - 1) / PAGESIZE;
    const size_t nChunks = std::min< size_t >( nPages,
                                               4 * ::omp_get_num_procs( ));
    const size_t chunkSize = (size + PAGESIZE_2 ) / nChunks;

    compressed.resize( nChunks );
#pragma omp parallel for
    for( ssize_t i = 0; i < ssize_t(size); i += chunkSize )
    {
        const size_t start = _getClosestPageSize( i, size );
        const size_t end = _getClosestPageSize( i + chunkSize, size );
        const size_t nBytes = end - start;
        const size_t index = i / chunkSize;

        compressed[index].reserve( getCompressBound( nBytes ));
        compressChunk( data + start, nBytes, compressed[index] );
    }
#else
    compressed.resize( 1 );
    compressed[0].reserve( getCompressBound( size ));
    compressChunk( data, size, compressed[0] );
#endif
    _in += size;
    _out += getDataSize( compressed );
    return compressed;
}

void Compressor::decompress( const Results& result, uint8_t* data, size_t size )
{
    if( result.empty( ))
        return;

    std::vector< std::pair< const uint8_t*, size_t >> inputs( result.size( ));
    for( size_t i = 0; i < result.size(); ++i )
        inputs[i] = { result[i].getData(), result[i].getSize() };
    decompress( inputs, data, size );
}


void Compressor::decompress(
    const std::vector< std::pair< const uint8_t*, size_t >>& inputs,
    uint8_t* data, size_t size )
{
    if( inputs.empty( ))
        return;
    _in += size;
    for( const auto& input : inputs )
        _out += input.second;

    if( inputs.size() == 1 ) // compressor did not have OpenMP
    {
        decompressChunk( inputs[0].first, inputs[0].second, data, size );
        return;
    }

    const size_t nPages = (size + PAGESIZE - 1) / PAGESIZE;
    const size_t nChunks = std::min< size_t >( nPages,
                                               4 * ::omp_get_num_procs( ));
    const size_t chunkSize = (size + PAGESIZE_2 ) / nChunks;

#pragma omp parallel for
    for( ssize_t i = 0; i < ssize_t(size); i += chunkSize )
    {
        size_t start = _getClosestPageSize( i, size );
        size_t end = _getClosestPageSize( i + chunkSize, size );
        const size_t nBytes = end - start;
        const size_t index = i / chunkSize;

        decompressChunk( inputs[index].first, inputs[index].second,
                         data + start, nBytes );
    }
}
}
}
