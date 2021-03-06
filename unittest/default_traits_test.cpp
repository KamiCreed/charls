// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "../src/default_traits.h"

using Microsoft::VisualStudio::CppUnitTestFramework::Assert;

namespace CharLSUnitTest
{
    TEST_CLASS(DefaultTraitsTest)
    {
    public:
        TEST_METHOD(Create)
        {
            const charls::DefaultTraits<uint8_t, uint8_t> traits((1 << 8) - 1, 0);

            Assert::AreEqual(255, traits.MAXVAL);
            Assert::AreEqual(256, traits.RANGE);
            Assert::AreEqual(0, traits.NEAR);
            Assert::AreEqual(8, traits.qbpp);
            Assert::AreEqual(8, traits.bpp);
            Assert::AreEqual(32, traits.LIMIT);
            Assert::AreEqual(64, traits.RESET);
        }
    };
}
