#ifndef LFM_BIT_HPP
#define LFM_BIT_HPP

#include <cstdint>

namespace lfm::bit
{

uint32_t reverse( uint32_t x );
uint8_t msb( uint32_t x );
void* tag( void* p );
uint32_t mark( uint32_t hash_key );
bool tagged( void* p );
uint32_t marked( uint32_t hash_key );
void* untag( void* p );
uint32_t unmark( uint32_t hash_key );

uint32_t parent_bucket( uint32_t bucket_num );

} // namespace lfm::bit

#endif // !LFM_BIT_HPP
