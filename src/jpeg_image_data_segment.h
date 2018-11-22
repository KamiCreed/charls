// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include <charls/public_types.h>

#include "jpeg_segment.h"

namespace charls
{

class JpegImageDataSegment final : public JpegSegment
{
public:
    JpegImageDataSegment(ByteStreamInfo rawStream, const JlsParameters& params, int componentCount) noexcept
        : _componentCount(componentCount),
          _rawStreamInfo(rawStream),
          _params(params)
    {
    }

    void Serialize(JpegStreamWriter& streamWriter) override;

private:
    int _componentCount;
    ByteStreamInfo _rawStreamInfo;
    JlsParameters _params;
};

} // namespace charls