#include "ympch.h"

#include "RetroArcade.h"

void RetroArcade::Init() {
	
}

void RetroArcade::Update(const Yume::StepTimer& coreTimer) {

}

void RetroArcade::Render() {
	m_pass.Execute(m_renderContext, m_shaderHandle, m_inputTexture, m_outputTexture);
}