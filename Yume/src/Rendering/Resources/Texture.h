#pragma once

#include "Services/ResourceManager/Resource.h"

#include <string>
#include <vulkan/vulkan.hpp>
#include <stb_image.h>

namespace Yume {

class Texture : public Resource {
public:
    explicit Texture(const std::string& id) : Resource(id) {}

    ~Texture() override;

    stbi_uc* GetPixels() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetChannels() const;

protected:
    bool DoLoad() override;
    bool DoUnload() override;

private:
    int m_width = 0;                
    int m_height = 0;               
    int m_channels = 0; 
    stbi_uc* m_pixels = nullptr;
};
}