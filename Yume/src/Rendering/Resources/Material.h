#pragma once

#include <glm/glm.hpp>
#include <string>

namespace Yume {

class Material {
public:
	void Bind();
	void SetUniform(const std::string& name, const glm::mat4& value);
};

}