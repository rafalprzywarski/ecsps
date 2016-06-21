#include <iostream>
#include <chrono>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <unordered_map>
#include <array>

using ElementType = unsigned;

void test_op(ElementType& x) { x *= (x + 1); }
void test_op_map(std::pair<const unsigned, ElementType>& x) { test_op(x.second); }
template <unsigned size>
void test_op_array(std::array<ElementType, size>& xa) { for (auto& x : xa) test_op(x); }
template <unsigned size>
void test_op_array2(std::vector<std::array<ElementType, size>>& xv) { for (auto& xa : xv) test_op_array<size>(xa); }


template <typename Container>
void test_sequence(unsigned size, unsigned loops, const std::string& name)
{
    Container container;

    for (unsigned i = 0; i < size; ++i)
        container.push_back(std::rand());

    auto t0 = std::chrono::high_resolution_clock::now();

    decltype(container) copy(container);

    auto t1 = std::chrono::high_resolution_clock::now();

    for (unsigned n = 0; n < loops; ++n)
        std::for_each(begin(container), end(container), test_op);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << name << " copy time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms" << std::endl;
    std::cout << name << " access time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms" << std::endl;
}

template <unsigned chunk_size>
void test_chunked_sequence(unsigned size, unsigned loops)
{
    std::list<std::array<ElementType, chunk_size>> container;

    for (unsigned i = 0; i < (size / chunk_size); ++i)
    {
        std::array<ElementType, chunk_size> chunk;
        for (auto& e : chunk)
            e = std::rand();
        container.push_back(chunk);
    }

    auto t0 = std::chrono::high_resolution_clock::now();

    decltype(container) copy(container);

    auto t1 = std::chrono::high_resolution_clock::now();

    for (unsigned n = 0; n < loops; ++n)
        std::for_each(begin(container), end(container), test_op_array<chunk_size>);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << "list of arrays " << chunk_size << " copy time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms" << std::endl;
    std::cout << "list of arrays " << chunk_size << " access time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms" << std::endl;
}

template <unsigned chunk_size>
void test_chunked_sequence2(unsigned size, unsigned loops)
{
    std::list<std::vector<std::array<ElementType, chunk_size>>> container;

    for (unsigned i = 0; i < (size / chunk_size / chunk_size); ++i)
    {
        std::vector<std::array<ElementType, chunk_size>> chunks;
        for (unsigned j = 0; j < chunk_size; ++j)
        {
            std::array<ElementType, chunk_size> chunk;
            for (auto& e : chunk)
                e = std::rand();
            chunks.push_back(chunk);
        }
        container.push_back(chunks);
    }

    auto t0 = std::chrono::high_resolution_clock::now();

    decltype(container) copy(container);

    auto t1 = std::chrono::high_resolution_clock::now();

    for (unsigned n = 0; n < loops; ++n)
        std::for_each(begin(container), end(container), test_op_array2<chunk_size>);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << "list of vectors of arrays " << chunk_size << " copy time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms" << std::endl;
    std::cout << "list of vectors of arrays " << chunk_size << " access time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms" << std::endl;
}

template <typename Container>
void test_map(unsigned size, unsigned loops, const std::string& name)
{
    Container container;

    for (unsigned i = 0; i < size; ++i)
        container.insert({i, std::rand()});

    auto t0 = std::chrono::high_resolution_clock::now();

    decltype(container) copy(container);

    auto t1 = std::chrono::high_resolution_clock::now();

    for (unsigned n = 0; n < loops; ++n)
        std::for_each(begin(container), end(container), test_op_map);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << name << " copy time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms" << std::endl;

    std::cout << name << " sequential time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms" << std::endl;

    t1 = std::chrono::high_resolution_clock::now();

    for (unsigned n = 0; n < loops; ++n)
        for (unsigned i = 0; i < size; ++i)
            test_op(container.at(i));

    t2 = std::chrono::high_resolution_clock::now();

    std::cout << name << " random time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms" << std::endl;
}

int main()
{
    unsigned size = 65536;
    unsigned loops = 16384;
    test_sequence<std::vector<ElementType>>(size, loops, "vector");
    test_chunked_sequence<64>(size, loops);
    test_chunked_sequence<32>(size, loops);
    test_chunked_sequence<16>(size, loops);
    test_chunked_sequence<8>(size, loops);
    test_chunked_sequence2<64>(size, loops);
    test_chunked_sequence2<32>(size, loops);
    test_chunked_sequence2<16>(size, loops);
    test_chunked_sequence2<8>(size, loops);
    test_sequence<std::list<ElementType>>(size, loops, "list");
    test_sequence<std::deque<ElementType>>(size, loops, "deque");
    test_map<std::map<unsigned, ElementType>>(size, loops, "map");
    test_map<std::unordered_map<unsigned, ElementType>>(size, loops, "unordered_map");
}
