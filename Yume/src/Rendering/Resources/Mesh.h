#pragma once

#include "Services/ResourceManager/Resource.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <functional>   
#include <cstddef>      
#include <glm/glm.hpp>  
#include <glm/gtx/hash.hpp>

namespace Yume {
	
class Mesh : public Resource {
public:
	struct Vertex {
		glm::vec3 position;   
		glm::vec3 normal;     
		glm::vec2 uv;

        bool operator==(const Vertex& other) const {
            return position == other.position && uv == other.uv;
        }      
	};

public:
    explicit Mesh(const std::string& id) : Resource(id) {}
    ~Mesh();

    uint32_t GetVertexCount() const;
    uint32_t GetIndexCount() const;

    const std::vector<Vertex>& GetVertices() const;
    const std::vector<uint32_t>& GetIndices() const;

protected:
	bool DoLoad() override;
	bool DoUnload() override;

private:   
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

}

namespace std {
    template<>
    struct hash<Yume::Mesh::Vertex> {
        size_t operator()(Yume::Mesh::Vertex const& v) const {
            size_t h1 = hash<glm::vec3>()(v.position);
            size_t h2 = hash<glm::vec2>()(v.uv);

            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
}

