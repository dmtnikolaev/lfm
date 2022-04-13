#include <cassert>
#include <iostream>
#include <vector>

#include "map.hpp"
#include "so_list.hpp"

int main()
{
    lfm::Map< int, int >* map = new lfm::SplitOrderedList< int, int >( 2, 2 );

    assert( map != nullptr );

    std::vector< int > elems = { 2, 8, 9, 13, 10 };

    for( int& elem : elems )
    {
        map->insert( elem, &elem );
    }

    for( int elem : elems )
    {
        int* found;
        auto res = map->find( elem, &found );
        assert( res );
        assert( elem == *( found ) );
    }

    return 0;
}
