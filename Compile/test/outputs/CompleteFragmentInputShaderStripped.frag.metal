#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct FragmentInput
{
    float4 scale [[color(0), raster_order_group(0)]];
    float4 offset [[color(1), raster_order_group(0)]];
};

struct VertexOut
{
    float4 color;
};

struct Transform
{
    float4x4 transform;
};

struct fragShader_out
{
    float4 color [[color(3), raster_order_group(1)]];
};

struct fragShader_in
{
    float4 VertexOut_color [[user(locn0)]];
};

static inline __attribute__((always_inline))
float4 _11(thread const float4& _10, thread FragmentInput& fragmentInput)
{
    return (_10 * fragmentInput.scale) + fragmentInput.offset;
}

fragment fragShader_out fragShader(fragShader_in in [[stage_in]], FragmentInput fragmentInput, texture2d<float> tex [[texture(0)]], sampler texSmplr [[sampler(0)]])
{
    fragShader_out out = {};
    VertexOut inputs = {};
    inputs.color = in.VertexOut_color;
    float4 _29 = tex.sample(texSmplr, float2(0.5));
    float4 _47 = _29;
    out.color = inputs.color * _11(_47, fragmentInput);
    return out;
}

