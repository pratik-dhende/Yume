#pragma once

#include "Texture.h"

#include <string>

namespace Yume {

	class TextureManager {
	public:
		TextureHandle LoadFromFile(const std::string& filepath);
	};

}
