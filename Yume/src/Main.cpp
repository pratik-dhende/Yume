#include <iostream>

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "ECS/EventSystem.h"

#include "Rendering/Resources/Material.h"
#include "Rendering/Resources/Mesh.h"
#include "Rendering/Resources/Texture.h"
#include "Rendering/Resources/Shader.h"
#include "Rendering/Core/RenderGraph.h"
#include "Rendering/Core/RenderTarget.h"
#include "Rendering/Core/RenderPass.h"
#include "Rendering/Core/RenderPassManager.h"

#include "Resources/Resource.h"
#include "Resources/ResourceManager.h"
#include "Resources/AsyncResourceManager.h"
#include "Resources/HotReloadResourceManager.h"

#include "Scene/CullingSystem.h"
#include "Scene/Camera.h"
#include "Scene/BoundingBox.h"

int main() {
	std::cout << "Hello, Yume!" << std::endl;

	return 0;
}