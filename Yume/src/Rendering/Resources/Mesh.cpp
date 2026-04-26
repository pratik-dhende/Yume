#include "Mesh.h"
#include "ServiceLocator/ServiceLocator.h"
#include "Services/ResourceManager/ResourceManager.h"

#include <string>
#include <vector>

namespace Yume {

Mesh::~Mesh() {
    Unload();                           
}

const std::vector<Mesh::Vertex>& Mesh::GetVertices() const {
    return m_vertices;
}

const std::vector<uint32_t>& Mesh::GetIndices() const {
    return m_indices;
}

uint32_t Mesh::GetVertexCount() const { return static_cast<uint32_t>(m_vertices.size()); }
uint32_t Mesh::GetIndexCount() const { return static_cast<uint32_t>(m_indices.size()); }

bool Mesh::DoLoad() {
    return ServiceLocator::GetService<ResourceManager>().LoadModel<Mesh>(GetId(), m_vertices, m_indices);
}

bool Mesh::DoUnload() {
    if (IsLoaded()) {
        m_vertices.clear();
        m_indices.clear();
    }
    return true;
}

}