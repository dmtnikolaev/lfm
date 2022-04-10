#include <cassert>

#include "map.hpp"
#include "so_list.hpp"

int main()
{
    lfm::Map<int, int>* map = new lfm::SplitOrderedList<int, int>();

    assert(map != nullptr);

    return 0;
}
