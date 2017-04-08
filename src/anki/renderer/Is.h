// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

/// @addtogroup renderer
/// @{

/// Illumination stage
class Is : public RenderingPass
{
anki_internal:
	Is(Renderer* r);

	~Is();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error binLights(RenderingContext& ctx);

	void setPreRunBarriers(RenderingContext& ctx);

	void run(RenderingContext& ctx);

	TexturePtr getRt() const
	{
		return m_rt;
	}

	const LightBin& getLightBin() const
	{
		return *m_lightBin;
	}

private:
	/// The IS render target
	TexturePtr m_rt;

	Array<U32, 3> m_clusterCounts = {{0, 0, 0}};
	U32 m_clusterCount = 0;

	/// The IS FBO
	FramebufferPtr m_fb;

	// Light shaders
	ShaderResourcePtr m_lightVert;
	Array<ShaderResourcePtr, IS_SHADER_PERMOUTATION_COUNT> m_lightFrags;
	Array<ShaderProgramPtr, IS_SHADER_PERMOUTATION_COUNT> m_lightProgs;

	LightBin* m_lightBin = nullptr;

	/// @name Limits
	/// @{
	U32 m_maxLightIds;
	/// @}

	/// Called by init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	void updateCommonBlock(RenderingContext& ctx);
};
/// @}

} // end namespace anki
