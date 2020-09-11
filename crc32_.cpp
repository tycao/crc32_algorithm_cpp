/*
  * for more detailed information, please refer to below link:
  * https://cpp.ra2diy.com/thread-51-1-1.html
*/ 

#include <array>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <cstddef>


/*
  @description: C++元编程，编译期 计算出 crc32 table
  @author     : 
  @date       : 2020/09/11
*/

namespace impl {
    // 按照 IEEE 802.3 描述的算法，除数为 0xEDB88320。
    template<unsigned round, std::uint32_t reg>
    struct generator {
        static constexpr std::uint32_t value = generator<round + 1, (reg >> 1) ^ ((reg & 1) ? 0xEDB88320 : 0)>::value;
    };
    template<std::uint32_t reg>
    struct generator<8, reg> {
        static constexpr std::uint32_t value = reg;
    };
    
    template<std::size_t ...indices>
    constexpr std::array<std::uint32_t, sizeof...(indices)> generate(std::index_sequence<indices...>) noexcept {
        return { generator<0, indices>::value... };
    }

    constexpr auto table = generate(std::make_index_sequence<256>());
}






/*
  @description:   运行期 计算出 crc32 table
  @date       ： 2020/09/11
*/

std::array<std::uint32_t, 256> create_table(std::uint32_t divisor) noexcept {
    std::array<std::uint32_t, 256> table;
    for (std::uint32_t i = 0; i < 256; ++i) {
        auto reg = i;
        for (unsigned j = 0; j < 8; ++j) {
            const bool lsb = reg & 1;  // ①
            reg >>= 1;         // ②
            if (lsb) {
                reg ^= divisor;
            }
        }
        table[i] = reg;
    }
    return table;
}

// 按照 IEEE 802.3 描述的算法，除数为 0xEDB88320。
static const auto table_ = create_table(0xEDB88320);


// 获取 crc32 checksum
std::uint32_t CRC32_Fast(const void *data, std::size_t size) noexcept {
    auto reg = static_cast<std::uint32_t>(-1);
    for (std::size_t i = 0; i < size; ++i) {
        const unsigned by = static_cast<const std::uint8_t *>(data)[i];
        reg = table[(reg ^ by) & 0xFF] ^ (reg >> 8);
    }
    return ~reg;
}
