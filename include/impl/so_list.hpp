#ifndef LFM_SO_LIST_HPP_IMPL
#define LFM_SO_LIST_HPP_IMPL

#include <cassert>
#include <cmath>
#include <so_list.hpp>
#include <stdexcept>

namespace lfm
{

template< typename K, typename V >
SplitOrderedList< K, V >::SplitOrderedList()
{
    seg_table_ = new HashTable[ max_segs_ ];
    seg_table_[ 0 ] = new Node*[ seg_size_ ];
    seg_table_[ 0 ][ 0 ] = new Node{ nullptr, 0, nullptr };

    hash_len_ = seg_size_;
}

template< typename K, typename V >
SplitOrderedList< K, V >::~SplitOrderedList()
{
    delete[] seg_table_;
    assert( false ); // TODO: full delete
}

template< typename K, typename V >
bool SplitOrderedList< K, V >::insert( K key, V* value )
{
    auto tries = 0;
    uint32_t hash_key = std::hash< K >{}( key );
    auto new_node = new Node{ nullptr, bit::mark( hash_key ), value };

    while( tries < max_tries_ )
    {
        auto bucket = new_node->hash_key % hash_len_;
        auto sentinel = find_bucket( bucket );

        if( sentinel == nullptr )
        {
            if( create_bucket( bucket, &sentinel ) == false )
            {
                tries++;
                continue;
            }
        }

        bool ret_flag = false;
        if( insert_pos( new_node, sentinel, &ret_flag ) )
        {
            return true;
        }

        if( ret_flag )
        {
            delete new_node;
            return false;
        }

        tries++;
    }

    assert( false );
    return false;
}

template< typename K, typename V >
bool SplitOrderedList< K, V >::find( K key, V** ret_value )
{
    assert( false );
    return 0;
}

template< typename K, typename V >
bool SplitOrderedList< K, V >::remove( K key )
{
    assert( false );
}
template< typename K, class V >
typename SplitOrderedList< K, V >::Node* SplitOrderedList< K, V >::find_pos(
    Node* sentinel, uint32_t hash_key, Node** ret_next )
{
    auto prev = sentinel;
    auto next = prev->next.load();
    while( next != nullptr && bit::reverse( next->hash_key ) <= bit::reverse( hash_key ) )
    {
        prev = next;
        next = prev->next.load();
    }

    if( ret_next != nullptr )
    {
        *ret_next = next;
    }

    return prev;
}
template< typename K, class V >
bool SplitOrderedList< K, V >::insert_pos( Node* new_node, Node* sentinel, bool* ret_flag )
{
    Node* next;
    auto prev = find_pos( sentinel, new_node->hash_key, &next );
    if( prev->hash_key == new_node->hash_key )
    {
        if( ret_flag != nullptr ) *( ret_flag ) = true;
        return false;
    }

    new_node->next.store( next );
    auto exchanged = prev->next.compare_exchange_strong( next, new_node );

    return exchanged;
}
template< typename K, class V >
typename SplitOrderedList< K, V >::Node* SplitOrderedList< K, V >::find_bucket( uint32_t bucket )
{
    auto hash_seg = find_segment( bucket );

    auto sentinel = hash_seg[ bucket % seg_size_ ];
    return sentinel;
}

template< typename K, class V >
bool SplitOrderedList< K, V >::create_bucket( uint32_t bucket, SplitOrderedList::Node** ret_sentinel )
{
    auto parent = bit::parent_bucket( bucket );
    auto parent_sent = find_bucket( parent );

    while( parent_sent == nullptr )
    {
        parent = bit::parent_bucket( parent );
        parent_sent = find_bucket( parent );
    }

    auto new_sent = new Node{ nullptr, bucket, nullptr };
    auto seg = find_segment( bucket );
    seg[ bucket % seg_size_ ] = new_sent;

    if( insert_pos( new_sent, parent_sent, nullptr ) )
    {
        *( ret_sentinel ) = new_sent;
        return true;
    }

    delete new_sent;
    return false;
}
template< typename K, class V >
typename SplitOrderedList< K, V >::HashTable SplitOrderedList< K, V >::find_segment( uint32_t bucket )
{
    auto hash_seg = seg_table_[ bucket >> static_cast< uint32_t >( std::sqrt( seg_size_ ) ) ];
    assert( hash_seg != nullptr );

    return hash_seg;
}

} // namespace lfm

#endif // !LFM_SO_LIST_HPP_IMPL
