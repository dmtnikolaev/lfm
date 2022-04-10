#ifndef LFM_SO_LIST_HPP_IMPL
#define LFM_SO_LIST_HPP_IMPL

#include <cassert>

namespace lfm
{

template< typename K, typename V >
SplitOrderedList< K, V >::SplitOrderedList() : load_factor_(4)
{
    assert( false );
}

template< typename K, typename V >
void SplitOrderedList< K, V >::insert( K key, V value )
{
    assert( false );
}

template< typename K, typename V >
V SplitOrderedList< K, V >::find( K key )
{
    assert( false );
    return 0;
}

template< typename K, typename V >
void SplitOrderedList< K, V >::remove( K key )
{
    assert( false );
}

} // namespace lfm

#endif // !LFM_SO_LIST_HPP_IMPL
