// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "jpeg_stream_writer.h"

#include "constants.h"
#include "jpeg_marker_code.h"
#include "jpeg_stream_reader.h"
#include "jpegls_preset_parameters_type.h"
#include "util.h"

#include <array>
#include <cassert>
#include <vector>

using std::array;
using std::vector;

namespace charls
{
JpegStreamWriter::JpegStreamWriter() noexcept
    : destination_{}
{
}


JpegStreamWriter::JpegStreamWriter(const ByteStreamInfo& destination) noexcept
    : destination_{destination}
{
}


void JpegStreamWriter::WriteStartOfImage()
{
    WriteMarker(JpegMarkerCode::StartOfImage);
}


void JpegStreamWriter::WriteEndOfImage()
{
    WriteMarker(JpegMarkerCode::EndOfImage);
}


void JpegStreamWriter::WriteSpiffHeaderSegment(const charls_spiff_header& spiff_header)
{
    ASSERT(spiff_header.height > 0);
    ASSERT(spiff_header.width > 0);

    // Create a JPEG APP8 segment in Still Picture Interchange File Format (SPIFF), v1.0
    vector<uint8_t> segment{'S', 'P', 'I', 'F', 'F', '\0'};
    segment.push_back(1);
    segment.push_back(0);
    segment.push_back(static_cast<uint8_t>(spiff_header.profile_id));
    segment.push_back(spiff_header.component_count);
    push_back(segment, static_cast<uint32_t>(spiff_header.height));
    push_back(segment, static_cast<uint32_t>(spiff_header.width));
    segment.push_back(static_cast<uint8_t>(spiff_header.color_space));
    segment.push_back(spiff_header.bits_per_sample);
    segment.push_back(static_cast<uint8_t>(spiff_header.compression_type));
    segment.push_back(static_cast<uint8_t>(spiff_header.resolution_units));
    push_back(segment, static_cast<uint32_t>(spiff_header.vertical_resolution));
    push_back(segment, static_cast<uint32_t>(spiff_header.horizontal_resolution));

    WriteSegment(JpegMarkerCode::ApplicationData8, segment.data(), segment.size());
}


void JpegStreamWriter::WriteSpiffEndOfDirectoryEntry()
{
    // Note: ISO/IEC 10918-3, Annex F.2.2.3 documents that the EOD entry segment should have a length of 8
    // but only 6 data bytes. This approach allows to postfix existing bitstreams\encoders with a SPIFF header.
    // In this implementation the SOI marker is added as data bytes to simplify the design.

    array<uint8_t, 6> segment{0, 0, 0, 1, 0xFF, 0xD8};
    WriteSegment(JpegMarkerCode::ApplicationData8, segment.data(), segment.size());
}


void JpegStreamWriter::WriteJpegFileInterchangeFormatSegment(const JfifParameters& params)
{
    ASSERT(params.units == 0 || params.units == 1 || params.units == 2);
    ASSERT(params.Xdensity > 0);
    ASSERT(params.Ydensity > 0);
    ASSERT(params.Xthumbnail >= 0 && params.Xthumbnail < 256);
    ASSERT(params.Ythumbnail >= 0 && params.Ythumbnail < 256);

    // Create a JPEG APP0 segment in the JPEG File Interchange Format (JFIF), v1.02
    vector<uint8_t> segment{'J', 'F', 'I', 'F', '\0'};
    push_back(segment, static_cast<uint16_t>(params.version));
    segment.push_back(static_cast<uint8_t>(params.units));
    push_back(segment, static_cast<uint16_t>(params.Xdensity));
    push_back(segment, static_cast<uint16_t>(params.Ydensity));

    // thumbnail
    segment.push_back(static_cast<uint8_t>(params.Xthumbnail));
    segment.push_back(static_cast<uint8_t>(params.Ythumbnail));
    if (params.Xthumbnail > 0)
    {
        if (params.thumbnail)
            throw jpegls_error{jpegls_errc::invalid_argument_thumbnail};

        segment.insert(segment.end(), static_cast<uint8_t*>(params.thumbnail),
                       static_cast<uint8_t*>(params.thumbnail) + static_cast<size_t>(3) * params.Xthumbnail * params.Ythumbnail);
    }

    WriteSegment(JpegMarkerCode::ApplicationData0, segment.data(), segment.size());
}


void JpegStreamWriter::WriteStartOfFrameSegment(int width, int height, int bitsPerSample, int componentCount)
{
    ASSERT(width >= 0 && width <= UINT16_MAX);
    ASSERT(height >= 0 && height <= UINT16_MAX);
    ASSERT(bitsPerSample >= MinimumBitsPerSample && bitsPerSample <= MaximumBitsPerSample);
    ASSERT(componentCount > 0 && componentCount <= UINT8_MAX);

    // Create a Frame Header as defined in ISO/IEC 14495-1, C.2.2 and T.81, B.2.2
    vector<uint8_t> segment;
    segment.push_back(static_cast<uint8_t>(bitsPerSample)); // P = Sample precision
    push_back(segment, static_cast<uint16_t>(height));      // Y = Number of lines
    push_back(segment, static_cast<uint16_t>(width));       // X = Number of samples per line

    // Components
    segment.push_back(static_cast<uint8_t>(componentCount)); // Nf = Number of image components in frame

    // Use by default 1 as the start component identifier to remain compatible with the
    // code sample of ISO/IEC 14495-1, H.4 and the JPEG-LS ISO conformance sample files.
    for (auto componentId = 1; componentId <= componentCount; ++componentId)
    {
        // Component Specification parameters
        segment.push_back(static_cast<uint8_t>(componentId)); // Ci = Component identifier
        segment.push_back(0x11);                              // Hi + Vi = Horizontal sampling factor + Vertical sampling factor
        segment.push_back(0);                                 // Tqi = Quantization table destination selector (reserved for JPEG-LS, should be set to 0)
    }

    WriteSegment(JpegMarkerCode::StartOfFrameJpegLS, segment.data(), segment.size());
}


void JpegStreamWriter::WriteColorTransformSegment(ColorTransformation transformation)
{
    array<uint8_t, 5> segment{'m', 'r', 'f', 'x', static_cast<uint8_t>(transformation)};
    WriteSegment(JpegMarkerCode::ApplicationData8, segment.data(), segment.size());
}


void JpegStreamWriter::WriteJpegLSPresetParametersSegment(const JpegLSPresetCodingParameters& params)
{
    vector<uint8_t> segment;

    segment.push_back(static_cast<uint8_t>(JpegLSPresetParametersType::PresetCodingParameters));

    push_back(segment, static_cast<uint16_t>(params.MaximumSampleValue));
    push_back(segment, static_cast<uint16_t>(params.Threshold1));
    push_back(segment, static_cast<uint16_t>(params.Threshold2));
    push_back(segment, static_cast<uint16_t>(params.Threshold3));
    push_back(segment, static_cast<uint16_t>(params.ResetValue));

    WriteSegment(JpegMarkerCode::JpegLSPresetParameters, segment.data(), segment.size());
}


void JpegStreamWriter::WriteStartOfScanSegment(int componentCount, int allowedLossyError, InterleaveMode interleaveMode)
{
    ASSERT(componentCount > 0 && componentCount <= UINT8_MAX);
    ASSERT(allowedLossyError >= 0 && allowedLossyError <= UINT8_MAX);
    ASSERT(interleaveMode == InterleaveMode::None ||
           interleaveMode == InterleaveMode::Line ||
           interleaveMode == InterleaveMode::Sample);

    // Create a Scan Header as defined in T.87, C.2.3 and T.81, B.2.3
    vector<uint8_t> segment;

    segment.push_back(static_cast<uint8_t>(componentCount));
    for (auto i = 0; i < componentCount; ++i)
    {
        segment.push_back(static_cast<uint8_t>(componentId_));
        componentId_++;
        segment.push_back(0); // Mapping table selector (0 = no table)
    }

    segment.push_back(static_cast<uint8_t>(allowedLossyError)); // NEAR parameter
    segment.push_back(static_cast<uint8_t>(interleaveMode));    // ILV parameter
    segment.push_back(0);                                       // transformation

    WriteSegment(JpegMarkerCode::StartOfScan, segment.data(), segment.size());
}


void JpegStreamWriter::WriteSegment(JpegMarkerCode markerCode, const void* data, size_t dataSize)
{
    ASSERT(dataSize <= UINT16_MAX - sizeof(uint16_t));

    WriteMarker(markerCode);
    WriteUInt16(static_cast<uint16_t>(dataSize + sizeof(uint16_t)));
    WriteBytes(data, dataSize);
}

} // namespace charls
