#ifndef LFM_MAP_HPP
#define LFM_MAP_HPP

namespace lfm
{

template< typename K, typename V >
struct Map
{
    virtual ~Map() = default;

    virtual void insert( K key, V value ) = 0;
    virtual V find( K key ) = 0;
    virtual void remove( K key ) = 0;
};

} // namespace lfm

#endif // !LFM_MAP_HPP
