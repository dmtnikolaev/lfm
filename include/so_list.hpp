#ifndef LFM_SO_LIST_HPP
#define LFM_SO_LIST_HPP

#include <atomic>
#include <cstdint>
#include <vector>

#include "bit.hpp"
#include "map.hpp"

namespace lfm
{

template< typename K, class V >
class SplitOrderedList : public Map< K, V >
{
    struct Node
    {
        std::atomic< Node* > next;
        uint32_t hash_key;
        V* value;
    };

    using HashTable = Node**;

public:
    SplitOrderedList();
    ~SplitOrderedList() override;

    bool insert( K key, V* value ) override;
    bool find( K key, V** ret_value ) override;
    bool remove( K key ) override;

private:
    Node* find_pos( Node* sentinel, uint32_t hash_key, Node** ret_pos );
    Node* find_bucket( uint32_t bucket );
    HashTable find_segment( uint32_t bucket );
    bool insert_pos( Node* new_node, Node* sentinel, bool* ret_flag );
    bool create_bucket( uint32_t bucket, Node** ret_sentinel );

private:
    const size_t max_segs_ = 256;
    const size_t seg_size_ = 4;
    const uint32_t load_factor_ = 4;
    const uint8_t max_tries_ = 10;

    std::atomic< size_t > hash_len_ = 0;
    std::atomic< size_t > size_ = 0;
    HashTable* seg_table_;
};

} // namespace lfm

#include "impl/so_list.hpp"

#endif // !LFM_SO_LIST_HPP
