// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator SRI_TEXEL_DIMENSION 8 16
#pragma anki mutator SHARED_MEMORY 0 1

#include <AnKi/Shaders/Functions.glsl>
#include <AnKi/Shaders/TonemappingFunctions.glsl>

// Find the maximum luma derivative in x and y, relative to the average luma of the block.
// Each thread handles a 2x4 region.

layout(set = 0, binding = 0) uniform ANKI_RP texture2D u_inputTex;
layout(set = 0, binding = 1) uniform sampler u_nearestClampSampler;

const UVec2 REGION_SIZE = UVec2(2u, 4u);

const UVec2 WORKGROUP_SIZE = UVec2(SRI_TEXEL_DIMENSION) / REGION_SIZE;
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;

layout(set = 0, binding = 2) uniform writeonly uimage2D u_sriImg;

layout(push_constant, std430) uniform b_pc
{
	Vec2 u_oneOverViewportSize;
	F32 u_threshold;
	F32 u_padding0;
};

#if SHARED_MEMORY
// Ideally, we'd be able to calculate the min/max/average using subgroup operations, but there's no guarantee
// subgroupSize is large enough so we need shared memory as a fallback. We need gl_NumSubgroups entries, but it is not a
// constant, so estimate it assuming a subgroupSize of at least 8.
const U32 SHARED_MEMORY_ENTRIES = WORKGROUP_SIZE.x * WORKGROUP_SIZE.y / 8u;
shared F32 s_averageLuma[SHARED_MEMORY_ENTRIES];
shared Vec2 s_maxDerivative[SHARED_MEMORY_ENTRIES];
#endif

#define sampleLuma(offsetX, offsetY) \
	computeLuminance( \
		textureLodOffset(sampler2D(u_inputTex, u_nearestClampSampler), uv, 0.0, IVec2(offsetX, offsetY)).xyz)

void main()
{
	const Vec2 uv = Vec2(gl_GlobalInvocationID.xy) * Vec2(REGION_SIZE) * u_oneOverViewportSize;

	// Get luminance.
	//       l2.z
	// l1.z  l1.w  l2.y
	// l1.x  l1.y
	// l0.z  l0.w  l2.x
	// l0.x  l0.y
	Vec4 l0;
	l0.x = sampleLuma(0, 0);
	l0.y = sampleLuma(1, 0);
	l0.z = sampleLuma(0, 1);
	l0.w = sampleLuma(1, 1);

	Vec4 l1;
	l1.x = sampleLuma(0, 2);
	l1.y = sampleLuma(1, 2);
	l1.z = sampleLuma(0, 3);
	l1.w = sampleLuma(1, 3);

	Vec3 l2;
	l2.x = sampleLuma(2, 1);
	l2.y = sampleLuma(2, 3);
	l2.z = sampleLuma(1, 4);

	// Calculate derivatives.
	Vec4 a = Vec4(l0.y, l2.x, l1.y, l2.y);
	Vec4 b = Vec4(l0.x, l0.w, l1.x, l1.w);
	const Vec4 dx = abs(a - b);

	a = Vec4(l0.z, l0.w, l1.z, l2.z);
	b = Vec4(l0.x, l0.y, l1.x, l1.w);
	const Vec4 dy = abs(a - b);

	F32 maxDerivativeX = max(max(dx.x, dx.y), max(dx.z, dx.w));
	F32 maxDerivativeY = max(max(dy.x, dy.y), max(dy.z, dy.w));
	maxDerivativeX = subgroupMax(maxDerivativeX);
	maxDerivativeY = subgroupMax(maxDerivativeY);

	// Calculate average luma in block.
	const Vec4 sumL0L1 = l0 + l1;
	F32 averageLuma = (sumL0L1.x + sumL0L1.y + sumL0L1.z + sumL0L1.w) / 8.0;
	averageLuma = subgroupAdd(averageLuma);

#if SHARED_MEMORY
	// Store results in shared memory.
	ANKI_BRANCH if(subgroupElect())
	{
		s_averageLuma[gl_SubgroupID] = averageLuma;
		s_maxDerivative[gl_SubgroupID] = Vec2(maxDerivativeX, maxDerivativeY);
	}

	memoryBarrierShared();
	barrier();
#endif

	// Write the result
	ANKI_BRANCH if(gl_LocalInvocationIndex == 0u)
	{
		// Get max across all subgroups.
#if SHARED_MEMORY
		averageLuma = s_averageLuma[0];
		Vec2 maxDerivative = s_maxDerivative[0];

		for(U32 i = 1u; i < gl_NumSubgroups; ++i)
		{
			averageLuma += s_averageLuma[i];
			maxDerivative = max(maxDerivative, s_maxDerivative[i]);
		}
#else
		const Vec2 maxDerivative = Vec2(maxDerivativeX, maxDerivativeY);
#endif

		// Determine shading rate.
		const F32 avgLuma = averageLuma / F32(WORKGROUP_SIZE.x * WORKGROUP_SIZE.y);
		const Vec2 lumaDiff = maxDerivative / avgLuma;
		const F32 threshold1 = u_threshold;
		const F32 threshold2 = threshold1 * 0.4;

		UVec2 rate;
		rate.x = (lumaDiff.x > threshold1) ? 1u : ((lumaDiff.x > threshold2) ? 2u : 4u);
		rate.y = (lumaDiff.y > threshold1) ? 1u : ((lumaDiff.y > threshold2) ? 2u : 4u);

		// 1x4 and 4x1 shading rates don't exist.
		if(rate == UVec2(1u, 4u))
		{
			rate = UVec2(1u, 2u);
		}
		else if(rate == UVec2(4u, 1u))
		{
			rate = UVec2(2u, 1u);
		}

		const UVec2 outTexelCoord = gl_WorkGroupID.xy;
		imageStore(u_sriImg, IVec2(outTexelCoord), UVec4(encodeVrsRate(rate)));
	}
}
