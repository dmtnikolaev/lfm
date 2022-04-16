#include "bit.hpp"

#include <cassert>

namespace lfm::bit
{

uint32_t reverse( uint32_t x )
{
    // swap odd and even bits
    x = ( ( x >> 1 ) & 0x55555555 ) | ( ( x & 0x55555555 ) << 1 );
    // swap consecutive pairs
    x = ( ( x >> 2 ) & 0x33333333 ) | ( ( x & 0x33333333 ) << 2 );
    // swap nibbles ...
    x = ( ( x >> 4 ) & 0x0F0F0F0F ) | ( ( x & 0x0F0F0F0F ) << 4 );
    // swap bytes
    x = ( ( x >> 8 ) & 0x00FF00FF ) | ( ( x & 0x00FF00FF ) << 8 );
    // swap 2-byte long pairs
    return ( x >> 16 ) | ( x << 16 );
}

uint8_t msb( uint32_t x )
{
    assert( x > 0 );

    uint8_t i = 31;
    while( ( x & ( 1 << i ) ) == 0 )
    {
        --i;
    }

    return i;
}

void* tag( void* p )
{
    return reinterpret_cast< void* >( reinterpret_cast< uintptr_t >( p ) | 1 );
}

void* untag( void* p )
{
    return reinterpret_cast< void* >( reinterpret_cast< uintptr_t >( p ) & ~static_cast< uintptr_t >( 1 ) );
}

bool tagged( void* p )
{
    return ( reinterpret_cast< uintptr_t >( p ) & 1 ) == 1;
}

uint32_t mark( uint32_t hash_key )
{
    return hash_key | 1 << 31;
}

uint32_t unmark( uint32_t hash_key )
{
    return hash_key & ~( 1 << 31 );
}

uint32_t parent_bucket( uint32_t bucket_num )
{
    assert( bucket_num > 0 );
    return bucket_num & ~( 1 << msb( bucket_num ) );
}

} // namespace lfm::bit
