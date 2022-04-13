#ifndef LFM_SO_LIST_HPP_IMPL
#define LFM_SO_LIST_HPP_IMPL

#include <cassert>
#include <cmath>
#include <so_list.hpp>
#include <stdexcept>

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
        auto hash_len = hash_len_.load();
        if( check_overload( hash_len ) )
        {
            auto new_len = hash_len << 1;

            HashTable old_seg = nullptr;

            uint32_t new_seg_ind = hash_len >> static_cast< uint32_t >( std::sqrt( seg_size_ ) );
            for( auto i = new_seg_ind; i < new_len; ++i )
            {
                auto new_seg = new std::atomic< Node* >[ seg_size_ ];
                if( seg_table_[ i ].compare_exchange_strong( old_seg, new_seg ) == false )
                {
                    delete[] new_seg;
                }
            }

            if( hash_len_.compare_exchange_strong( hash_len, new_len ) == false )
            {
            continue;
        }
        }

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
    assert( false );
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
            if( create_bucket( bucket, &sentinel ) )
            {
    return sentinel;
            }
        }
        else
        {
            return sentinel;
        }
    }

    return nullptr;
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
    Node* old_sent = nullptr;
    seg[ bucket % seg_size_ ].compare_exchange_strong( old_sent, new_sent );

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
    auto hash_seg = seg_table_[ bucket >> static_cast< uint32_t >( std::sqrt( seg_size_ ) ) ].load();
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
