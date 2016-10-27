
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

#include "CompressorZFP.h"

#include <pression/data/Registry.h>
#include <lunchbox/buffer.h>

namespace pression
{
namespace data
{
namespace
{
const bool _initialized =
    Registry::getInstance().registerEngine< CompressorZFP >(
        { "pression::data::CompressorZFP", .55f, .43f });
}

CompressorZFP::CompressorZFP()
    : _stream( zfp_stream_open( NULL ))
{
    zfp_stream_set_accuracy( _stream, 0, zfp_type_float );
}

CompressorZFP::~CompressorZFP()
{
    zfp_stream_close( _stream );
}

size_t CompressorZFP::getCompressBound( const size_t size ) const
{
    zfp_field* field = zfp_field_1d( nullptr, zfp_type_float,
                                     size / sizeof( float ));
    const size_t maxSize = zfp_stream_maximum_size( _stream, field );
    zfp_field_free( field );
    return maxSize;
}

void CompressorZFP::compress( const uint8_t* const data, const size_t size,
                              Result& output )
{
    if( !_initialized )
        return;

    zfp_field* field = zfp_field_1d( (void*)data, zfp_type_float,
                                     size / sizeof( float ));
    bitstream* bstream = stream_open( output.getData(), output.getMaxSize( ));

    zfp_stream_set_bit_stream( _stream, bstream );
    zfp_stream_rewind( _stream );
    output.setSize( zfp_compress( _stream, field ));

    stream_close( bstream );
    zfp_field_free( field );
}

void CompressorZFP::decompress( const uint8_t* const input,
                                const size_t inputSize,
                                uint8_t* const data, const size_t size )
{
    if( !_initialized )
        return;

    zfp_field* field = zfp_field_1d( (void*)data, zfp_type_float,
                                     size / sizeof( float ));
    bitstream* bstream = stream_open( (void*)input, inputSize );

    zfp_stream_set_bit_stream( _stream, bstream );
    zfp_stream_rewind( _stream );
    zfp_decompress( _stream, field );

    stream_close( bstream );
    zfp_field_free( field );
}

}
}
