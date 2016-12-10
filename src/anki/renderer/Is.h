// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>

namespace anki
{

// Forward
class FrustumComponent;
class LightBin;
enum class ShaderVariantBit : U8;

/// @addtogroup renderer
/// @{

/// Illumination stage
class Is : public RenderingPass
{
anki_internal:
	TexturePtr m_stencilRt;

	Is(Renderer* r);

	~Is();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error binLights(RenderingContext& ctx);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	TexturePtr getRt(U idx) const
	{
		return m_rt[idx];
	}

	/// Get the number of mips for IS's render target.
	U getRtMipmapCount() const
	{
		return m_rtMipCount;
	}

	const LightBin& getLightBin() const
	{
		return *m_lightBin;
	}

private:
	/// The IS render target
	Array<TexturePtr, 2> m_rt;
	U8 m_rtMipCount = 0;
	U32 m_clusterCount = 0;

	/// The IS FBO
	Array<FramebufferPtr, 2> m_fb;

	TexturePtr m_dummyTex;

	// Light shaders
	ShaderResourcePtr m_lightVert;
	ShaderResourcePtr m_lightFrag;
	ShaderProgramPtr m_lightProg;

	class ShaderVariant
	{
	public:
		ShaderResourcePtr m_lightFrag;
		ShaderProgramPtr m_lightProg;
	};

	using Key = ShaderVariantBit;

	/// Hash the hash.
	class Hasher
	{
	public:
		U64 operator()(const Key& b) const
		{
			return U64(b);
		}
	};

	HashMap<Key, ShaderVariant, Hasher> m_shaderVariantMap;

	LightBin* m_lightBin = nullptr;

	/// @name Limits
	/// @{
	U32 m_maxLightIds;
	/// @}

	class Alt
	{
	public:
		ShaderResourcePtr m_frag;
		ShaderProgramPtr m_prog;
	} m_alt;

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void updateCommonBlock(RenderingContext& ctx);

	ANKI_USE_RESULT Error getOrCreateProgram(
		ShaderVariantBit variantMask, RenderingContext& ctx, ShaderProgramPtr& prog);
};
/// @}

} // end namespace anki
