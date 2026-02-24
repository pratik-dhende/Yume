#pragma once

#include "D3D12Renderer.h"
#include "Texture.h"
#include "RenderContext.h"
#include "ShaderManager.h"


namespace Yume {

class PostProcessPass {
public:	
	virtual void Execute(const RenderContext& renderContext, ShaderHandle shaderHandle, const TextureHandle input, const TextureHandle output);
};

}