#include <iostream>
#include <iomanip>
#include <array>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <cstddef>

#define HEX( x ) std::setw(2) << std::setfill('0') << std::hex << (uint)( x )
typedef unsigned char byte;
typedef unsigned int uint;


/* 
  @description: 使用C++ 元编程，编译期计算出crc32 table
  @author     :
  @date       : 2020/09/11
*/
static const uint polynomial = 0x04C11DB7;
static const uint initReminder = 0xFFFFFFFF;
static const uint finalXor = 0xFFFFFFFF;
template<unsigned round, std::uint32_t reg>
struct generator {
    static constexpr std::uint32_t value = generator<round + 1, (reg << 1) ^ ((reg & 0x80000000) ? polynomial : 0)>::value;
};
template<std::uint32_t reg>
struct generator<8, reg> {
    static constexpr std::uint32_t value = reg;
};

template<std::size_t ...indices>
constexpr std::array<std::uint32_t, sizeof...(indices)> generate(std::index_sequence<indices...>) noexcept {
    return { generator<0, indices << 24>::value... };
}
static constexpr auto crcTable_ = generate(std::make_index_sequence<256>());

// 获取crc32 checksum
uint CRC32_Fast(byte bytes[], uint len)
{
    uint crc = 0;
    for (int i = 0; i < len; ++i)
    {
        byte b = bytes[i];
        /* XOR-in next input byte into MSB of crc and get this MSB, that's our new intermediate divident */
        byte pos = (byte)((crc ^ (b << 24)) >> 24);
        /* Shift out the MSB used for division per lookuptable and XOR with the remainder */
        crc = (uint)((crc << 8) ^ (uint)(crcTable_[pos]));
    }
    return crc;
}






/*
  运行期 计算出 crc32 table
*/
/* 
  @description: 运行期 计算出 crc32 table
  @author     :
  @date       : 2020/09/11
*/
static uint crcTable[256]; 
static void CalculateCrcTable_CRC32()
{
    for (int divident = 0; divident < 256; divident++) /* iterate over all possible input byte values 0 - 255 */
    {
        uint curByte = (uint)(divident << 24); /* move divident byte into MSB of 32Bit CRC */
        for (byte bit = 0; bit < 8; bit++)
        {
            if ((curByte & 0x80000000) != 0)
            {
                curByte <<= 1;
                curByte ^= polynomial;
            }
            else
            {
                curByte <<= 1;
            }
        }
        crcTable[divident] = curByte;
    }
}

uint CRC32_Fast(byte bytes[], uint len)
{
    uint crc = 0;
    for(int i = 0; i < len; ++i)
    {
        byte b = bytes[i];
        /* XOR-in next input byte into MSB of crc and get this MSB, that's our new intermediate divident */
        byte pos = (byte)((crc ^ (b << 24)) >> 24);
        /* Shift out the MSB used for division per lookuptable and XOR with the remainder */
        crc = (uint)((crc << 8) ^ (uint)(crcTable[pos]));
    }

    return crc;
}

uint CRC32_Slow(byte bytes[], uint len)
{
    //const uint polynomial = 0x04C11DB7; /* divisor is 32bit */
    uint crc = 0; /* CRC value is 32bit */

    for(int i = 0; i < len; ++i)
    {
        byte b = bytes[i];
        crc ^= (uint)(b << 24); /* move byte into MSB of 32bit CRC */
        for (int i = 0; i < 8; i++)
        {
            if ((crc & 0x80000000) != 0) /* test for MSB = bit 31 */
            {
                crc = (uint)((crc << 1) ^ polynomial);
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}
