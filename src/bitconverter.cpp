#include "CrossLang.hpp"

namespace Tesses::CrossLang
{
    double BitConverter::ToDoubleBits(uint64_t v)
    {
        return *(double*)&v;
    }
    uint64_t BitConverter::ToUintBits(double v)
    {
        return *(uint64_t*)&v;
    }
    double BitConverter::ToDoubleBE(uint8_t& b)
    {
        return ToDoubleBits(ToUint64BE(b));
    }
    uint64_t BitConverter::ToUint64BE(uint8_t& b)
    {
        uint8_t* b2 = &b;
        uint64_t v = 0;
        v |= ((uint64_t)b2[0] << 56);
        v |= ((uint64_t)b2[1] << 48);
        v |= ((uint64_t)b2[2] << 40);
        v |= ((uint64_t)b2[3] << 32);
        v |= ((uint64_t)b2[4] << 24);
        v |= ((uint64_t)b2[5] << 16);
        v |= ((uint64_t)b2[6] << 8);
        v |= (uint64_t)b2[7];
        return v;
    }
    uint32_t BitConverter::ToUint32BE(uint8_t& b)
    {
        uint8_t* b2 = &b;
        uint32_t v = 0;
       
        v |= ((uint32_t)b2[0] << 24);
        v |= ((uint32_t)b2[1] << 16);
        v |= ((uint32_t)b2[2] << 8);
        v |= (uint32_t)b2[3];
        return v;
    }
    uint16_t BitConverter::ToUint16BE(uint8_t& b)
    {
        uint8_t* b2 = &b;
        uint16_t v = 0;
       
        
        v |= ((uint16_t)b2[0] << 8);
        v |= (uint16_t)b2[1];
        return v;
    }
    void BitConverter::FromDoubleBE(uint8_t& b, double v)
    {
        FromUint64BE(b,ToUintBits(v));
    }
    void BitConverter::FromUint64BE(uint8_t& b, uint64_t v)
    {
        uint8_t* b2 = &b;
        b2[0] = (uint8_t)(v >> 56);
        b2[1] = (uint8_t)(v >> 48);
        b2[2] = (uint8_t)(v >> 40);
        b2[3] = (uint8_t)(v >> 32);
        b2[4] = (uint8_t)(v >> 24);
        b2[5] = (uint8_t)(v >> 16);
        b2[6] = (uint8_t)(v >> 8);
        b2[7] = (uint8_t)v;
    }
    void BitConverter::FromUint32BE(uint8_t& b, uint32_t v)
    {
        uint8_t* b2 = &b;
        
        b2[0] = (uint8_t)(v >> 24);
        b2[1] = (uint8_t)(v >> 16);
        b2[2] = (uint8_t)(v >> 8);
        b2[3] = (uint8_t)v;
    }
    void BitConverter::FromUint16BE(uint8_t& b, uint16_t v)
    {
        uint8_t* b2 = &b;
        
        b2[0] = (uint8_t)(v >> 8);
        b2[1] = (uint8_t)v;
    }

};