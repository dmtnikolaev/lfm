#include <cassert>

#include "map.hpp"
#include "so_list.hpp"

int main()
{
    lfm::Map< int, int >* map = new lfm::SplitOrderedList< int, int >();

    assert( map != nullptr );

    int k1 = 2;
    int k2 = 8;
    int d1 = 2;
    int d2 = 8;

    map->insert( k1, &d1 );
    map->insert( k2, &d2 );

    return 0;
}
