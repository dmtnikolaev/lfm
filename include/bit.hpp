#ifndef LFM_BIT_HPP
#define LFM_BIT_HPP

#include <cstdint>

namespace lfm::bit
{

static uint32_t reverse( uint32_t x );
static uint8_t msb( uint32_t x );
static uint8_t lsb( uint32_t x );
static void mark( void* p );
static void unmark( void* p );
static bool marked( void* p );

} // namespace lfm::bit

#endif // !LFM_BIT_HPP
