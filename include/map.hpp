#ifndef LFM_MAP_HPP
#define LFM_MAP_HPP

namespace lfm
{

template< typename K, class V >
struct Map
{
    virtual ~Map() = default;

    virtual bool insert( K key, V* value ) = 0;
    virtual bool find( K key, V** ret_value ) = 0;
    virtual bool remove( K key ) = 0;
};

} // namespace lfm

#endif // !LFM_MAP_HPP
