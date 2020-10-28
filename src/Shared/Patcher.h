#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

#if !defined(_M_IX86) && !defined(__i386__)
#error "Patcher supports only x86 systems"
#endif

namespace Patcher {

const size_t CallJmpSize = 5;

enum ERegisterName : uint8_t
{
    RN_EAX = 0x00,
    RN_ECX = 0x01,
    RN_EDX = 0x02,
    RN_EBX = 0x03,
    RN_ESP = 0x04,
    RN_EBP = 0x05,
    RN_ESI = 0x06,
    RN_EDI = 0x07,
};

static inline size_t WritePatch(void *addr, const void *data, size_t size)
{
    const volatile char *volatile src = (const char *)data;
    volatile char *volatile dst = (char *)addr;
    for (size_t i = 0; i < size; i++)
        dst[i] = src[i];
    return size;
}

static inline size_t WriteFill(void *addr, uint8_t value, size_t size)
{
    volatile char *volatile dst = (char *)addr;
    volatile uint8_t val = value;
    for (size_t i = 0; i < size; i++)
        dst[i] = val;
    return size;
}

template <size_t Size>
static inline size_t WritePatch(void *addr, const uint8_t (&data)[Size])
{
    return WritePatch(addr, data, Size);
}

// clang-format off
static inline char     *volatile &GetVarStr    (void *addr) { return *((char    *volatile *)addr); }
static inline void     *volatile &GetVarPtr    (void *addr) { return *((void    *volatile *)addr); }
static inline float     volatile &GetVarFloat  (void *addr) { return *((float    volatile *)addr); }
static inline double    volatile &GetVarDouble (void *addr) { return *((double   volatile *)addr); }
static inline int32_t   volatile &GetVarI32    (void *addr) { return *((int32_t  volatile *)addr); }
static inline uint32_t  volatile &GetVarU32    (void *addr) { return *((uint32_t volatile *)addr); }
static inline int16_t   volatile &GetVarI16    (void *addr) { return *((int16_t  volatile *)addr); }
static inline uint16_t  volatile &GetVarU16    (void *addr) { return *((uint16_t volatile *)addr); }
static inline int8_t    volatile &GetVarI8     (void *addr) { return *((int8_t   volatile *)addr); }
static inline uint8_t   volatile &GetVarU8     (void *addr) { return *((uint8_t  volatile *)addr); }
static inline int8_t    volatile &GetVarSByte  (void *addr) { return *((int8_t   volatile *)addr); }
static inline uint8_t   volatile &GetVarByte   (void *addr) { return *((uint8_t  volatile *)addr); }

static inline char      *ReadStr      (const void *addr) { return GetVarStr    ((void*)addr); }
static inline void      *ReadPtr      (const void *addr) { return GetVarPtr    ((void*)addr); }
static inline float      ReadFloat    (const void *addr) { return GetVarFloat  ((void*)addr); }
static inline double     ReadDouble   (const void *addr) { return GetVarDouble ((void*)addr); }
static inline int32_t    ReadI32      (const void *addr) { return GetVarI32    ((void*)addr); }
static inline uint32_t   ReadU32      (const void *addr) { return GetVarU32    ((void*)addr); }
static inline int16_t    ReadI16      (const void *addr) { return GetVarI16    ((void*)addr); }
static inline uint16_t   ReadU16      (const void *addr) { return GetVarU16    ((void*)addr); }
static inline int8_t     ReadI8       (const void *addr) { return GetVarI8     ((void*)addr); }
static inline uint8_t    ReadU8       (const void *addr) { return GetVarU8     ((void*)addr); }
static inline int8_t     ReadSByte    (const void *addr) { return GetVarI8     ((void*)addr); }
static inline uint8_t    ReadByte     (const void *addr) { return GetVarU8     ((void*)addr); }

static inline size_t     WriteCharPtr (void *addr, const char  *value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WritePtr     (void *addr, const void  *value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteFloat   (void *addr, float        value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteDouble  (void *addr, double       value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteI32     (void *addr, int32_t      value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteU32     (void *addr, uint32_t     value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteI16     (void *addr, int16_t      value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteU16     (void *addr, uint16_t     value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteI8      (void *addr, int8_t       value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteU8      (void *addr, uint8_t      value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteSByte   (void *addr, int8_t       value) { return WritePatch(addr, &value, sizeof(value)); }
static inline size_t     WriteByte    (void *addr, uint8_t      value) { return WritePatch(addr, &value, sizeof(value)); }

static inline size_t WriteBytes(
    void *addr,
    int b0 = -1, int b1 = -1, int b2 = -1, int b3 = -1, int b4 = -1,
    int b5 = -1, int b6 = -1, int b7 = -1, int b8 = -1, int b9 = -1)
{
    int data[] = { b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, -1 };
    size_t count;
    for (count = 0; data[count] >= 0 && data[count] <= 255; count++)
        WriteByte((uint8_t *)addr + count, (uint8_t)(data[count]));
    return count;
}
// clang-format on

// For call/jmp instructions
static inline int32_t AddrDelta(const void *from, const void *to, int instrSize)
{
    auto offset = (intptr_t)((const char *)to - (const char *)from - instrSize);
    assert(offset >= INT32_MIN && offset <= INT32_MAX);
    return (int32_t)offset;
}

static inline size_t WriteNops(void *addr, size_t count)
{
    return WriteFill(addr, 0x90, count);
}

static inline size_t WriteCall(void *addr, const void *from, const void *to)
{
    return WriteU8(addr, 0xE8) + WriteI32((char *)addr + 1, AddrDelta(from, to, CallJmpSize));
}

static inline size_t WriteJump(void *addr, const void *from, const void *to)
{
    return WriteU8(addr, 0xE9) + WriteI32((char *)addr + 1, AddrDelta(from, to, CallJmpSize));
}

static inline size_t WriteCall(void *addr, const void *to)
{
    return WriteCall(addr, addr, to);
}

static inline size_t WriteJump(void *addr, const void *to)
{
    return WriteJump(addr, addr, to);
}

static inline size_t WritePush(void *addr, uint32_t value)
{
    return WriteU8(addr, 0x68) + WriteU32((char *)addr + 1, value);
}

static inline size_t WriteCall(void *addr, ERegisterName reg)
{
    return WriteBytes(addr, 0xFF, 0xD0 + reg);
}

static inline size_t WriteJump(void *addr, ERegisterName reg)
{
    return WriteBytes(addr, 0xFF, 0xE0 + reg);
}

static inline size_t WritePush(void *addr, ERegisterName reg)
{
    return WriteByte(addr, 0x50 + reg);
}

static inline size_t WritePop(void *addr, ERegisterName reg)
{
    return WriteByte(addr, 0x58 + reg);
}

static inline size_t WriteMov(void *addr, ERegisterName reg, uint32_t value)
{
    return WriteU8(addr, 0xB8 + reg) + WriteU32((char *)addr + 1, value);
}

struct SPatchChunk
{
    uint8_t *Addr;
    std::vector<uint8_t> Data;

    SPatchChunk(void *addr, const void *data, size_t size)
        : Addr((uint8_t *)addr)
        , Data((uint8_t *)data, (uint8_t *)data + size)
    {
    }

    template <size_t Size>
    SPatchChunk(void *addr, const uint8_t (&data)[Size])
        : Addr((uint8_t *)addr)
        , Data(data, data + Size)
    {
    }

    SPatchChunk(void *addr, const std::vector<uint8_t> &data)
        : Addr((uint8_t *)addr)
        , Data(data)
    {
    }
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4512) // assignment operator could not be generated
#endif

struct SPatch
{
    const char *const Name;
    std::vector<SPatchChunk> Chunks;

    SPatch(const char *name = "")
        : Name(name)
        , _finished(false)
    {
    }

    SPatch(void *addr)
        : Name("")
        , _finished(false)
    {
        SetAddr(addr);
    }

    SPatch(const char *name, void *addr)
        : Name(name)
        , _finished(false)
    {
        SetAddr(addr);
    }

    void Finish()
    {
        if (!_finished)
        {
            TryMergeLastChunk();
            for (auto it = Chunks.begin(); it != Chunks.end(); ++it)
                it->Data.shrink_to_fit();
            _finished = true;
        }
    }

    uint8_t *GetAddr()
    {
        assert(!Chunks.empty());
        auto &chunk = Chunks.back();
        return chunk.Addr + chunk.Data.size();
    }

    void SetAddr(void *addr)
    {
        assert(!_finished);
        if (Chunks.empty() || addr != GetAddr())
        {
            TryMergeLastChunk();
            char nothing = 0;
            Chunks.push_back(SPatchChunk(addr, &nothing, 0));
        }
    }

    size_t Skip(size_t size)
    {
        SetAddr(GetAddr() + size);
        return size;
    }

    size_t Write(uint8_t *addr, const void *data, size_t size)
    {
        assert(!_finished);
        if (!Chunks.empty() && addr == GetAddr())
        {
            auto &chunk = Chunks.back();
            size_t prevSize = chunk.Data.size();
            chunk.Data.resize(prevSize + size);
            memcpy(&chunk.Data[prevSize], data, size);
        }
        else
        {
            TryMergeLastChunk();
            Chunks.push_back(SPatchChunk(addr, data, size));
        }
        return size;
    }

    size_t Write(void *addr, const void *data, size_t size)
    {
        return Write((uint8_t *)addr, data, size);
    }

    size_t Write(const void *data, size_t size) { return Write(GetAddr(), data, size); }

    template <size_t Size>
    size_t Write(void *addr, const uint8_t (&data)[Size])
    {
        return Write(addr, data, Size);
    }

    template <size_t Size>
    size_t Write(const uint8_t (&data)[Size])
    {
        return Write(data, Size);
    }

    size_t Write(void *addr, const std::vector<uint8_t> &data)
    {
        return Write(addr, &data[0], data.size());
    }

    size_t Write(const std::vector<uint8_t> &data) { return Write(&data[0], data.size()); }

    size_t WriteBytes(void *addr, int b0 = -1, int b1 = -1, int b2 = -1, int b3 = -1, int b4 = -1,
                      int b5 = -1, int b6 = -1, int b7 = -1, int b8 = -1, int b9 = -1)
    {
        int data[] = { b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, -1 };
        size_t count;
        for (count = 0; data[count] >= 0 && data[count] <= 255; count++)
            WriteByte((uint8_t *)addr + count, (uint8_t)(data[count]));
        return count;
    }

    size_t WriteBytes(int b0 = -1, int b1 = -1, int b2 = -1, int b3 = -1, int b4 = -1, int b5 = -1,
                      int b6 = -1, int b7 = -1, int b8 = -1, int b9 = -1)
    {
        return WriteBytes(GetAddr(), b0, b1, b2, b3, b4, b5, b6, b7, b8, b9);
    }

    // clang-format off
    size_t WriteCharPtr (void *addr, const char  *value) { return Write(addr, &value, sizeof(value)); }
    size_t WritePtr     (void *addr, const void  *value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteFloat   (void *addr, float        value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteDouble  (void *addr, double       value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteI32     (void *addr, int32_t      value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteU32     (void *addr, uint32_t     value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteI16     (void *addr, int16_t      value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteU16     (void *addr, uint16_t     value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteI8      (void *addr, int8_t       value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteU8      (void *addr, uint8_t      value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteSByte   (void *addr, int8_t       value) { return Write(addr, &value, sizeof(value)); }
    size_t WriteByte    (void *addr, uint8_t      value) { return Write(addr, &value, sizeof(value)); }

    size_t WriteCharPtr (const char  *value) { return Write(&value, sizeof(value)); }
    size_t WritePtr     (const void  *value) { return Write(&value, sizeof(value)); }
    size_t WriteFloat   (float        value) { return Write(&value, sizeof(value)); }
    size_t WriteDouble  (double       value) { return Write(&value, sizeof(value)); }
    size_t WriteI32     (int32_t      value) { return Write(&value, sizeof(value)); }
    size_t WriteU32     (uint32_t     value) { return Write(&value, sizeof(value)); }
    size_t WriteI16     (int16_t      value) { return Write(&value, sizeof(value)); }
    size_t WriteU16     (uint16_t     value) { return Write(&value, sizeof(value)); }
    size_t WriteI8      (int8_t       value) { return Write(&value, sizeof(value)); }
    size_t WriteU8      (uint8_t      value) { return Write(&value, sizeof(value)); }
    size_t WriteSByte   (int8_t       value) { return Write(&value, sizeof(value)); }
    size_t WriteByte    (uint8_t      value) { return Write(&value, sizeof(value)); }
    // clang-format on

    size_t WriteFill(void *addr, uint8_t value, size_t size)
    {
        for (size_t i = 0; i < size; i++)
            WriteByte((char *)addr + i, value);
        return size;
    }

    size_t WriteNops(void *addr, size_t count) { return WriteFill(addr, 0x90, count); }

    size_t WriteNopsToAddr(void *addr, const void *to)
    {
        assert(addr < to);
        return WriteNops(addr, (char *)to - (char *)addr);
    }

    size_t WriteCall(void *from, const void *to)
    {
        return WriteU8(from, 0xE8) + WriteI32((char *)from + 1, AddrDelta(from, to, CallJmpSize));
    }

    size_t WriteJump(void *from, const void *to)
    {
        return WriteU8(from, 0xE9) + WriteI32((char *)from + 1, AddrDelta(from, to, CallJmpSize));
    }

    size_t WritePush(void *addr, uint32_t value)
    {
        return WriteU8(addr, 0x68) + WriteU32((char *)addr + 1, value);
    }

    size_t WriteCall(void *addr, ERegisterName reg) { return WriteBytes(addr, 0xFF, 0xD0 + reg); }

    size_t WriteJump(void *addr, ERegisterName reg) { return WriteBytes(addr, 0xFF, 0xE0 + reg); }

    size_t WritePush(void *addr, ERegisterName reg) { return WriteU8(addr, 0x50 + reg); }

    size_t WritePop(void *addr, ERegisterName reg) { return WriteU8(addr, 0x58 + reg); }

    size_t WriteMov(void *addr, ERegisterName reg, uint32_t value)
    {
        return WriteU8(addr, 0xB8 + reg) + WriteU32((char *)addr + 1, value);
    }

    // clang-format off
    size_t WriteFill(uint8_t value, size_t size)       { return WriteFill(GetAddr(), value, size); }
    size_t WriteNops(size_t count)                     { return WriteNops(GetAddr(), count); }
    size_t WriteNopsToAddr(const void *to)             { return WriteNopsToAddr(GetAddr(), to); }
    size_t WriteCall(const void *to)                   { return WriteCall(GetAddr(), to); }
    size_t WriteJump(const void *to)                   { return WriteJump(GetAddr(), to); }
    size_t WritePush(uint32_t value)                   { return WritePush(GetAddr(), value); }
    size_t WriteCall(ERegisterName reg)                { return WriteCall(GetAddr(), reg); }
    size_t WriteJump(ERegisterName reg)                { return WriteJump(GetAddr(), reg); }
    size_t WritePush(ERegisterName reg)                { return WritePush(GetAddr(), reg); }
    size_t WritePop(ERegisterName reg)                 { return WritePop(GetAddr(), reg); }
    size_t WriteMov(ERegisterName reg, uint32_t value) { return WriteMov(GetAddr(), reg, value); }
    // clang-format on

private:
    bool _finished;

    void TryMergeLastChunk()
    {
        if (Chunks.size() < 2)
            return;

        auto &lastChunk = Chunks.back();
        if (lastChunk.Data.empty())
        {
            Chunks.pop_back();
            return;
        }

        for (auto it = Chunks.begin(); &(*it) != &lastChunk; ++it)
        {
            if (it->Addr <= lastChunk.Addr + lastChunk.Data.size() &&
                it->Addr + it->Data.size() >= lastChunk.Addr)
            {
                if (it->Addr > lastChunk.Addr)
                    std::swap(*it, lastChunk);

                auto extraSize = (lastChunk.Addr + lastChunk.Data.size()) -
                                 (it->Addr + it->Data.size());
                if (extraSize > 0)
                    it->Data.resize(it->Data.size() + extraSize);
                memcpy(&it->Data[lastChunk.Addr - it->Addr], &lastChunk.Data[0],
                       lastChunk.Data.size());
                Chunks.pop_back();
                return;
            }
        }
    }
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace Patcher
