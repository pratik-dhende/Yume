#pragma once

#include "RetroArcadePass.h"
#include "Yume.h"

class RetroArcade : public Yume::Application
{
public:
	void Init() override;
	void Update(const Yume::StepTimer& coreTimer) override;
	void Render() override;

private:
	Yume::TextureHandle m_inputTexture;
	Yume::TextureHandle m_outputTexture;

	RetroArcadePass m_pass;
	Yume::ShaderHandle m_shaderHandle;
	Yume::RenderContext m_renderContext;
};

