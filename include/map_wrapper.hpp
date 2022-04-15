#ifndef MAP_WRAPPER_HPP
#define MAP_WRAPPER_HPP

#include <map>
#include <mutex>

#include "so_list.hpp"

template< typename TMap >
class MapWrapper;

template< typename T, typename TKey >
class MapWrapper< lfm::SplitOrderedList< TKey, T > >
{
public:
    void insert( TKey key, T* v );
    bool find( TKey key, T** v );
    int size();

private:
    lfm::SplitOrderedList< TKey, T > m_{ 16, 16 };
};

template< typename T, typename TKey >
class MapWrapper< std::map< TKey, T > >
{
public:
    void insert( TKey key, T v );
    bool find( TKey key, T* v );
    int size() const;

private:
    std::map< TKey, T > m_;
    std::mutex mutex_;
};

template< typename T, typename TKey >
void MapWrapper< lfm::SplitOrderedList< TKey, T > >::insert( TKey key, T* v )
{
    m_.insert( key, v );
}

template< typename T, typename TKey >
bool MapWrapper< lfm::SplitOrderedList< TKey, T > >::find( TKey key, T** v )
{
    return m_.find( key, v );
}

template< typename T, typename TKey >
int MapWrapper< lfm::SplitOrderedList< TKey, T > >::size()
{
    return m_.size();
}

template< typename T, typename TKey >
void MapWrapper< std::map< TKey, T > >::insert( TKey key, T v )
{
    std::lock_guard< std::mutex > guard( mutex_ );
    m_.insert( std::make_pair( key, v ) );
}

template< typename T, typename TKey >
bool MapWrapper< std::map< TKey, T > >::find( TKey key, T* v )
{
    auto it = m_.find( key );
    if( it == m_.end() )
    {
        *v = T{};
        return false;
    }

    *v = it->second;
    return true;
}

template< typename T, typename TKey >
int MapWrapper< std::map< TKey, T > >::size() const
{
    return static_cast< int >( m_.size() );
}

#endif // !MAP_WRAPPER_HPP
