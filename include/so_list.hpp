#ifndef LFM_SO_LIST_HPP
#define LFM_SO_LIST_HPP

#include <cstdint>
#include <vector>

#include "map.hpp"

namespace lfm
{

template< typename K, typename V >
class SplitOrderedList : public Map< K, V >
{
    struct Node
    {
        Node* next;
        uint32_t hash_key;
        V value;
    };

public:
    SplitOrderedList();
    ~SplitOrderedList() override = default;

    void insert( K key, V value ) override;
    V find( K key ) override;
    void remove( K key ) override;

private:
    std::vector< Node* > hash_table_;
    const uint32_t load_factor_;
};

} // namespace lfm

#include "impl/so_list.hpp"

#endif // !LFM_SO_LIST_HPP
