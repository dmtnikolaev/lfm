#include <cassert>
#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <thread>
#include <vector>

#include <immintrin.h>

#include "map.hpp"
#include "map_wrapper.hpp"
#include "so_list.hpp"

// #define STD_MAP_MUTEX

struct MapElem
{
    explicit MapElem( int val ) : MapElem( val, false ) {}
    MapElem( int val, bool d ) : v( std::make_unique< int >( val ) ), dup( d ) {}

    std::unique_ptr< int > v;
    bool dup;
};

using SrcMap = std::map< std::string, MapElem >;

#ifdef STD_MAP_MUTEX
using Map = std::map< std::string, int* >;
#else
using Map = lfm::SplitOrderedList< std::string, int >;
#endif // STD_MAP_MUTEX

static MapWrapper< Map > g_shared;
static std::atomic< int > g_thread_count;

std::pair< std::vector< SrcMap >, int > BuildTestMaps( int thread_count, int elems_count )
{
    using namespace std::literals;

    std::vector< SrcMap > res;

    // Add unique (70%)
    int unique_count = elems_count * 0.7;
    for( auto i = 0; i < thread_count; ++i )
    {
        auto key = "thread"s + std::to_string( i ) + "_";
        SrcMap m;
        for( auto j = 0; j < unique_count / thread_count; ++j )
        {
            m.emplace( std::make_pair( key + std::to_string( j ), MapElem( j ) ) );
        }
        res.push_back( std::move( m ) );
    }

    size_t d = 0;
    for( auto& t : res )
    {
        d += t.size();
    }

    // Add duplicates (30%)
    auto dups_count = elems_count - unique_count;
    SrcMap m_d;
    for( auto i = 0; i < dups_count; ++i )
    {
        auto key = "dup_"s + std::to_string( i );
        m_d.emplace( std::make_pair( key, MapElem( i ) ) );
    }
    for( auto i = 0; i < thread_count; ++i )
    {
        for( auto& [ k, v ] : m_d )
        {
            res[ i ].emplace( std::make_pair( k, MapElem( *v.v, true ) ) );
        }
    }

    assert( unique_count + dups_count == elems_count );
    d += dups_count;
    return { std::move( res ), d };
}

struct Stopwatch
{
public:
    static Stopwatch Start()
    {
        Stopwatch sw;
        clock_gettime( CLOCK_REALTIME, &sw.start_ );
        return sw;
    }

    void Stop() { clock_gettime( CLOCK_REALTIME, &end_ ); }

    uint64_t GetNs() const
    {
        const uint64_t billion = 1000000000;
        return ( end_.tv_sec - start_.tv_sec ) * billion + ( end_.tv_nsec - start_.tv_nsec );
    }

private:
    Stopwatch() {}

    timespec start_;
    timespec end_;
};

void process( SrcMap* m )
{
    g_thread_count.fetch_sub( 1 );
    while( g_thread_count.load() > 0 )
    {
        _mm_pause();
    }

    for( const auto& [ k, v ] : *m )
    {
        g_shared.insert( k, v.v.get() );
    }
}

int main( int argc, char* argv[] )
{
    if( argc != 2 )
    {
        std::cout << "invalid args\n";
        return 1;
    }

    constexpr auto kElemsCount = 1'000'000;

    auto thread_count = std::stoi( argv[ 1 ] );
    auto [ src, total_count ] = BuildTestMaps( thread_count, kElemsCount );

    g_thread_count = thread_count;
    std::vector< std::thread > ths;

    auto sw = Stopwatch::Start();
    for( auto i = 0; i < thread_count; ++i )
    {
        auto th = std::thread( process, &src[ i ] );
        ths.push_back( std::move( th ) );
    }

    for( auto& t : ths )
    {
        if( t.joinable() )
        {
            t.join();
        }
    }
    sw.Stop();

    assert( g_shared.size() == total_count );
    for( auto i = 0; i < thread_count; ++i )
    {
        for( auto& [ key, v ] : src[ i ] )
        {
            int* act_v;
            auto found = g_shared.find( key, &act_v );
            assert( found );
            if( !v.dup )
            {
                assert( act_v == v.v.get() );
            }
        }
    }

    std::cout << "Took: " << sw.GetNs() << "\n";
    return 0;
}
