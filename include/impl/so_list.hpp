#ifndef LFM_SO_LIST_HPP_IMPL
#define LFM_SO_LIST_HPP_IMPL

#include <cassert>
#include <cmath>
#include <so_list.hpp>
#include <stdexcept>

#include <iostream>

namespace lfm
{

template< typename K, typename V >
SplitOrderedList< K, V >::SplitOrderedList( size_t seg_size, size_t load_factor )
    : seg_size_( seg_size ), load_factor_( load_factor )
{
    seg_table_ = new std::atomic< HashTable >[ max_segs_ ];
    seg_table_[ 0 ].store( new std::atomic< Node* >[ seg_size_ ] );
    seg_table_[ 0 ][ 0 ].store( new Node{ nullptr, 0, nullptr } );

    hash_len_ = seg_size_;
}

template< typename K, typename V >
SplitOrderedList< K, V >::~SplitOrderedList()
{
    auto node = seg_table_[ 0 ][ 0 ].load();
    while( node != nullptr )
    {
        auto old = node;
        node = node->next;

        delete old;
    }
    for( size_t i = 0; i < max_segs_; ++i )
    {
        delete[] seg_table_[ i ];
    }

    delete[] seg_table_;
}

template< typename K, typename V >
bool SplitOrderedList< K, V >::insert( K key, V* value )
{
    auto tries = 0;
    uint32_t hash_key = std::hash< K >{}( key );
    auto new_node = new Node{ nullptr, bit::mark( hash_key ), value };

    while( tries < max_tries_ )
    {
        auto hash_len = hash_len_.load();
        if( check_overload( hash_len ) )
        {
            auto new_len = hash_len << 1;
            assert( new_len <= seg_size_ * max_segs_ );

            if( !hash_len_.compare_exchange_strong( hash_len, new_len ) )
            {
                continue;
            }
        }

        auto bucket = new_node->hash_key % hash_len_;
        auto sentinel = find_bucket( bucket );

        bool ret_flag = false;
        if( insert_pos( new_node, sentinel, &ret_flag ) )
        {
            size_++;
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
    uint32_t hash_key = std::hash< K >{}( key );
    hash_key = bit::mark( hash_key );
    auto res = find_pos( find_bucket( hash_key % hash_len_ ), hash_key, nullptr, nullptr );

    if( res == nullptr )
    {
        return false;
    }

    *( ret_value ) = res->value;
    return true;
}

template< typename K, typename V >
bool SplitOrderedList< K, V >::remove( K key )
{
    assert( false ); // Feature is not ready.

    uint32_t hash_key = std::hash< K >{}( key );
    hash_key = bit::mark( hash_key );

    Node* next;
    Node* prev;

    auto found = find_pos( find_bucket( hash_key % hash_len_ ), hash_key, &prev, &next );

    if( found == nullptr )
    {
        return false;
    }

    Node* next_tagged = reinterpret_cast< Node* >( bit::tag( next ) );

    if( found->next.compare_exchange_strong( next, next_tagged ) )
    {
        size_--;
        return true;
    }

    return false;
}

template< typename K, class V >
typename SplitOrderedList< K, V >::Node* SplitOrderedList< K, V >::find_pos(
    Node* sentinel, uint32_t hash_key, Node** ret_prev, Node** ret_next )
{
    if( sentinel == nullptr )
    {
        return nullptr;
    }

    Node* prev = nullptr;
    auto found = sentinel;
    auto next = found->next.load();

    while( next != nullptr && bit::reverse( next->hash_key ) <= bit::reverse( hash_key ) )
    {
        prev = found;
        found = next;
        next = found->next.load();
    }

    if( ret_next != nullptr )
    {
        *ret_next = next;
    }

    if( found->hash_key != hash_key )
    {
        if( ret_prev != nullptr )
        {
            *ret_prev = found;
        }

        return nullptr;
    }

    if( ret_prev != nullptr )
    {
        *ret_prev = prev;
    }

    return found;
}

template< typename K, class V >
bool SplitOrderedList< K, V >::insert_pos( Node* new_node, Node* sentinel, bool* ret_flag )
{
    Node* next;
    Node* prev;

    if( find_pos( sentinel, new_node->hash_key, &prev, &next ) != nullptr )
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
    for( auto i = 0; i < max_tries_; ++i )
    {
        auto sentinel = hash_seg[ bucket % seg_size_ ].load();
        if( sentinel == nullptr )
        {
            if( create_bucket( bucket, hash_seg, &sentinel ) )
            {
                assert( sentinel->hash_key == bucket );
                return sentinel;
            }
        }
        else
        {
            assert( sentinel->hash_key == bucket );
            return sentinel;
        }
    }

    assert( false );
    return nullptr;
}

template< typename K, class V >
bool SplitOrderedList< K, V >::create_bucket( uint32_t bucket, HashTable seg, SplitOrderedList::Node** ret_sentinel )
{
    auto parent = bit::parent_bucket( bucket );
    auto parent_sent = find_bucket( parent );

    while( parent_sent == nullptr )
    {
        parent = bit::parent_bucket( parent );
        parent_sent = find_bucket( parent );
    }

    Node* prev;
    Node* next;
    if( find_pos( parent_sent, bucket, &prev, &next ) != nullptr )
    {
        return false;
    }

    auto new_sent = new Node{ next, bucket, nullptr };
    Node* old_sent = nullptr;
    if( seg[ bucket % seg_size_ ].compare_exchange_strong( old_sent, new_sent ) )
    {
        int try_num = 0;
        while( !( prev->next.compare_exchange_strong( next, new_sent ) ) && try_num < max_tries_ )
        {
            assert( find_pos( parent_sent, bucket, &prev, nullptr ) == nullptr );
            try_num++;
        }
        assert( try_num < max_tries_ );
        *( ret_sentinel ) = new_sent;
        return true;
    }

    delete new_sent;
    return false;
}

template< typename K, class V >
typename SplitOrderedList< K, V >::HashTable SplitOrderedList< K, V >::find_segment( uint32_t bucket )
{
    uint32_t seg_index = bucket >> static_cast< uint32_t >( std::sqrt( seg_size_ ) );

    auto hash_seg = seg_table_[ seg_index ].load();
    if( hash_seg == nullptr )
    {
        auto new_seg = new std::atomic< Node* >[ seg_size_ ];
        assert( new_seg != nullptr );
        if( !seg_table_[ seg_index ].compare_exchange_strong( hash_seg, new_seg ) )
        {
            delete[] new_seg;
            return seg_table_[ seg_index ].load();
        }

        hash_seg = seg_table_[ seg_index ].load();
    }

    assert( hash_seg != nullptr );
    return hash_seg;
}

template< typename K, class V >
bool SplitOrderedList< K, V >::check_overload( size_t hash_len )
{
    return size_ / hash_len >= load_factor_;
}

template< typename K, class V >
size_t SplitOrderedList< K, V >::size()
{
    return size_.load();
}

} // namespace lfm

#endif // !LFM_SO_LIST_HPP_IMPL
