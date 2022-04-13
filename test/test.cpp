#include <cassert>
#include <iostream>
#include <vector>

#include "bit.hpp"
#include "map.hpp"
#include "so_list.hpp"

int main()
{
    lfm::Map< int, int >* map = new lfm::SplitOrderedList< int, int >( 2, 2 );

    assert( map != nullptr );

    std::vector< int > keys = { 2, 8, 9, 13, 10 };
    std::vector< int > vals = { 2, 8, 9, 13, 10 };

    for( auto i = 0; i < keys.size(); ++i )
    {
        auto res = map->insert( keys[ i ], &vals[ i ] );
        assert( res );
    }

    assert( map->remove( 9 ) );

    for( auto i = 0; i < keys.size(); ++i )
    {
        if( i == 2 ) continue;
        int* found;
        auto res = map->find( keys[ i ], &found );
        assert( res );
        assert( vals[ i ] == *( found ) );
    }

    int* found;
    assert( map->find( 9, &found ) == false );
    assert( map->insert( 9, &vals[ 2 ] ) );
    assert( map->find( 9, &found ) );
    assert( vals[ 2 ] == *( found ) );

    return 0;
}
