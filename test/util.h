// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include <charls/charls.h>
#include <vector>
#include <exception>

struct Size
{
    Size(size_t width, size_t height) noexcept
        :
        cx(width),
        cy(height)
    {}
    size_t cx;
    size_t cy;
};


void FixEndian(std::vector<uint8_t>* buffer, bool littleEndianData);
std::vector<uint8_t> ReadFile(const char* filename, long offset = 0, size_t bytes = 0);
void TestFile(const char* filename, int offset, Size size2, int bitsPerSample, int componentCount, bool littleEndianFile = false, int loopCount = 1);
void TestRoundTrip(const char* strName, const std::vector<uint8_t>& decodedBuffer, Size size, int bitsPerSample, int componentCount, int loopCount = 1);
void TestRoundTrip(const char* strName, const std::vector<uint8_t>& originalBuffer, JlsParameters& params, int loopCount = 1);
void WriteFile(const char* filename, std::vector<uint8_t>& buffer);
void test_portable_anymap_file(const char* filename, int loopCount = 1);

class UnitTestException : public std::exception {
public:
    explicit UnitTestException() = default;
};

class Assert
{
public:
    static void IsTrue(bool condition)
    {
        if (!condition)
            throw UnitTestException();
    }
};

#ifdef _MSC_VER
#define MSVC_WARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable : x))  // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses)
#define MSVC_WARNING_UNSUPPRESS() __pragma(warning(pop))
#else
#define MSVC_WARNING_SUPPRESS(x)
#define MSVC_WARNING_UNSUPPRESS()
#endif
