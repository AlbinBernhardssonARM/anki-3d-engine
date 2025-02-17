// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Global illumination.
class IndirectDiffuse : public RendererObject
{
public:
	IndirectDiffuse(Renderer* r)
		: RendererObject(r)
	{
		registerDebugRenderTarget("IndirectDiffuse");
	}

	~IndirectDiffuse();

	ANKI_USE_RESULT Error init();

	void populateRenderGraph(RenderingContext& ctx);

	void getDebugRenderTarget(CString rtName, RenderTargetHandle& handle,
							  ShaderProgramPtr& optionalShaderProgram) const override
	{
		ANKI_ASSERT(rtName == "IndirectDiffuse");
		handle = m_runCtx.m_mainRtHandles[WRITE];
	}

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_mainRtHandles[WRITE];
	}

private:
	Array<TexturePtr, 2> m_rts;
	FramebufferDescription m_fbDescr;
	Bool m_rtsImportedOnce = false;

	static constexpr U32 READ = 0;
	static constexpr U32 WRITE = 1;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_main;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		Array<ShaderProgramPtr, 2> m_grProgs;
	} m_denoise;

	class
	{
	public:
		Array<RenderTargetHandle, 2> m_mainRtHandles;
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal();
};
/// @}

} // namespace anki
