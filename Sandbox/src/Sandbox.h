#pragma once
#include "Yume.h"

class Sandbox : public Yume::Application
{
public:
	void init() override;
	void update() override;

private:
	void buildCbvHeap();
	void buildConstantBuffer();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
};