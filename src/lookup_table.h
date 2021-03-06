// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include <cstring>
#include <array>
#include <cassert>

namespace charls
{

// Tables for fast decoding of short Golomb Codes.
struct Code final
{
    Code() = default;

    Code(int32_t value, int32_t length) noexcept :
        value_{value},
        length_{length}
    {
    }

    int32_t GetValue() const noexcept
    {
        return value_;
    }

    int32_t GetLength() const noexcept
    {
        return length_;
    }

    int32_t value_{};
    int32_t length_{};
};


class CTable final
{
public:
    static constexpr size_t byte_bit_count = 8;

    CTable() noexcept
    {
        std::memset(types_.data(), 0, sizeof(types_)); // TODO: analyze if needed
    }

    void AddEntry(uint8_t value, Code c) noexcept
    {
        const int32_t length = c.GetLength();
        ASSERT(static_cast<size_t>(length) <= byte_bit_count);

        for (int32_t i = 0; i < 1 << (byte_bit_count - length); ++i)
        {
            ASSERT(types_[(static_cast<size_t>(value) << (byte_bit_count - length)) + i].GetLength() == 0);
            types_[(static_cast<size_t>(value) << (byte_bit_count - length)) + i] = c;
        }
    }

    FORCE_INLINE const Code& Get(int32_t value) const noexcept
    {
        return types_[value];
    }

private:
    std::array<Code, 1 << byte_bit_count> types_;
};

} // namespace charls
