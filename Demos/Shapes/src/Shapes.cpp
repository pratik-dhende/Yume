#include "ympch.h"
#include "Shapes.h"
#include "FrameResource.h"
#include "RenderItem.h"

constexpr float MOUSE_SENSITIVITY = 0.01f;
constexpr int totalBufferFrameResources = 3;

void Shapes::init()
{
	Yume::EventDispatcher::registerEventHandler([&](const Yume::Event& event) {
		this->handleMouseMove(event);
		});

	Yume::EventDispatcher::registerEventHandler([&](const Yume::Event& event) {
		this->handleWindowResize(event);
		});

	// Puts command list in the recording state
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_commandList->Reset(m_renderer->m_commandAllocator.Get(), nullptr));

	buildRenderItems();
	buildBufferFrameResources();
	buildCbvHeap();
	buildConstantBuffers();
	buildRootSignature();
	buildShadersAndInputLayout();
	buildPipelineStateObject();

	// Execute the initialization commands
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_commandList->Close());
	ID3D12CommandList* commandLists[] = { m_renderer->m_commandList.Get() };
	m_renderer->m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	m_renderer->flushCommandQueue();

	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, 1.0f * m_window->getWidth() / m_window->getHeight(), m_nearZ, m_farZ);
	DirectX::XMStoreFloat4x4(&m_projection, projection);
}

void Shapes::update(const Yume::StepTimer& coreTimer) {
	updateCamera();

	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % totalBufferFrameResources;

	if (m_bufferFrameResources[m_currentFrameResourceIndex]->m_fenceValue > 0 && m_renderer->getGPUFenceValue() < m_bufferFrameResources[m_currentFrameResourceIndex]->m_fenceValue) {
		m_renderer->sync(m_bufferFrameResources[m_currentFrameResourceIndex]->m_fenceValue);
	}

	updateObjectConstants();
	updatePassConstants(coreTimer);
}

void Shapes::draw()
{	
	auto commandAllocator = m_bufferFrameResources[m_currentFrameResourceIndex]->m_commandAllocator;

	// Clear command allocator and command list
	YM_THROW_IF_FAILED_DX_EXCEPTION(commandAllocator->Reset());
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_commandList->Reset(commandAllocator.Get(), m_pipelineStateObject.Get()));

	// Set the view port and scissor rects
	m_renderer->m_commandList->RSSetViewports(1, &m_renderer->m_screenViewport);
	m_renderer->m_commandList->RSSetScissorRects(1, &m_renderer->m_scissorRect);

	// Switch from present to target
	auto presentTargetTransition = CD3DX12_RESOURCE_BARRIER::Transition(m_renderer->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_renderer->m_commandList->ResourceBarrier(1, &presentTargetTransition);

	// Clear the render target and dsv
	m_renderer->m_commandList->ClearRenderTargetView(m_renderer->getCurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	m_renderer->m_commandList->ClearDepthStencilView(m_renderer->getDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Set render targets
	auto backBufferView = m_renderer->getCurrentBackBufferView();
	auto depthStencilView = m_renderer->getDepthStencilView();
	m_renderer->m_commandList->OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

	// Set descriptor heaps
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	m_renderer->m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// Set root signature
	m_renderer->m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// Set pass constants
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGPUHeapHandle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	cbvGPUHeapHandle.Offset(static_cast<int>(m_renderItems.size()) * totalBufferFrameResources + m_currentFrameResourceIndex, m_renderer->getCbvSrvUavDescriptorHandleIncrementSize());
	m_renderer->m_commandList->SetGraphicsRootDescriptorTable(1, cbvGPUHeapHandle);

	drawRenderItems();

	// Switch from target to present
	auto targetPresentTransition = CD3DX12_RESOURCE_BARRIER::Transition(m_renderer->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_renderer->m_commandList->ResourceBarrier(1, &targetPresentTransition);

	// Close command list
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_commandList->Close());

	// Execute command list
	ID3D12CommandList* commandLists[] = { m_renderer->m_commandList.Get() };
	m_renderer->m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	// Present
	// TODO: Why did we use 0s as the parameters?
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_swapChain->Present(0, 0));
	m_renderer->switchBackBuffer();

	m_renderer->signalFence();
	m_bufferFrameResources[m_currentFrameResourceIndex]->m_fenceValue = m_renderer->getCPUFenceValue();
}

std::unique_ptr<Yume::Application> Yume::createApplication() {
	return std::make_unique<Shapes>();
}

void Shapes::drawRenderItems() {
	for (const auto& renderItem : m_renderItems) {
		
		auto vertexBufferView = renderItem->m_mesh->getVertexBufferView();
		auto indexBufferView = renderItem->m_mesh->getIndexBufferView();
		m_renderer->m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		m_renderer->m_commandList->IASetIndexBuffer(&indexBufferView);
		m_renderer->m_commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set Object Constants
		CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGPUHeapHandle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvGPUHeapHandle.Offset(m_currentFrameResourceIndex * static_cast<int>(m_renderItems.size()) + renderItem->m_constantBufferOffset, m_renderer->getCbvSrvUavDescriptorHandleIncrementSize());

		m_renderer->m_commandList->SetGraphicsRootDescriptorTable(0, cbvGPUHeapHandle);

		auto renderingMesh = renderItem->m_mesh->subMeshes["box"];
		m_renderer->m_commandList->DrawIndexedInstanced(renderingMesh.indexCount, 1, renderingMesh.startIndex, renderingMesh.baseVertexLocation, 0);
	}
}

void Shapes::updateCamera() {
	m_cameraPhi -= m_mouseXDelta;
	m_cameraTheta -= m_mouseYDelta;
	m_cameraTheta = std::clamp(m_cameraTheta, 0.1f, DirectX::XM_PI - 0.1f);

	m_cameraPositionWorld.x = m_cameraRadius * std::sin(m_cameraTheta) * std::cos(m_cameraPhi);
	m_cameraPositionWorld.y = m_cameraRadius * std::cos(m_cameraTheta);
	m_cameraPositionWorld.z = m_cameraRadius * std::sin(m_cameraTheta) * std::sin(m_cameraPhi);

	DirectX::XMVECTOR position = DirectX::XMVectorSet(m_cameraPositionWorld.x, m_cameraPositionWorld.y, m_cameraPositionWorld.z, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorZero();
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(position, target, up);
	DirectX::XMStoreFloat4x4(&m_view, view);
}

void Shapes::updateObjectConstants() {
	for (const auto& renderItem : m_renderItems) {
		if (renderItem->m_objectConstantsUpdates > 0) {
			ObjectConstants objectConstants;
			objectConstants.m_world = renderItem->m_world;
			m_bufferFrameResources[m_currentFrameResourceIndex]->m_objectConstants->updateBuffer(renderItem->m_constantBufferOffset, objectConstants);

			--renderItem->m_objectConstantsUpdates;
		}
	}
}

void Shapes::updatePassConstants(const Yume::StepTimer& timer) {
	PassConstants passConstants;
	passConstants.m_cameraPositionWorld = m_cameraPositionWorld;
	passConstants.m_farZ = m_farZ;
	passConstants.m_nearZ = m_nearZ;

	auto projection = DirectX::XMLoadFloat4x4(&m_projection);
	auto view = DirectX::XMLoadFloat4x4(&m_view);
	auto viewProjection = view * projection;

	auto projectionDeterminant = DirectX::XMMatrixDeterminant(projection);
	auto viewDeterminant = DirectX::XMMatrixDeterminant(view);
	auto viewProjectionDeterminant = DirectX::XMMatrixDeterminant(viewProjection);

	auto inverseView = DirectX::XMMatrixInverse(&viewDeterminant, view);
	auto inverseProjection = DirectX::XMMatrixInverse(&projectionDeterminant, projection);
	auto inverseViewProjection = DirectX::XMMatrixInverse(&viewProjectionDeterminant, viewProjection);

	DirectX::XMStoreFloat4x4(&passConstants.m_projection, DirectX::XMMatrixTranspose(projection));
	DirectX::XMStoreFloat4x4(&passConstants.m_inverseProjection, DirectX::XMMatrixTranspose(inverseProjection));
	DirectX::XMStoreFloat4x4(&passConstants.m_view, DirectX::XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&passConstants.m_inverseView, DirectX::XMMatrixTranspose(inverseView));
	DirectX::XMStoreFloat4x4(&passConstants.m_viewProjection, DirectX::XMMatrixTranspose(viewProjection));
	DirectX::XMStoreFloat4x4(&passConstants.m_inverseViewProjection, DirectX::XMMatrixTranspose(inverseViewProjection));

	passConstants.m_renderTargetSize = DirectX::XMFLOAT2(m_renderer->getRenderTargetWidth(), m_renderer->getRenderTargetHeight());
	passConstants.m_inverseRenderTargetSize = DirectX::XMFLOAT2(1.0f / m_renderer->getRenderTargetWidth(), 1.0f / m_renderer->getRenderTargetHeight());

	passConstants.m_totalTime = timer.GetTotalSeconds();
	passConstants.m_deltaTime = timer.GetElapsedSeconds();

	m_bufferFrameResources[m_currentFrameResourceIndex]->m_passConstants->updateBuffer(0, passConstants);
}

void Shapes::buildBufferFrameResources() {
	m_bufferFrameResources.reserve(totalBufferFrameResources);

	for (int i = 0; i < totalBufferFrameResources; ++i) {
		m_bufferFrameResources.emplace_back(std::make_unique<FrameResource>(m_renderer->m_device.Get(), static_cast<int>(m_renderItems.size()), 1));
	}
}

void Shapes::buildRenderItems() {
	m_renderItems.emplace_back(std::make_unique<RenderItem>());

	Yume::Geometry geometry;
	auto boxMesh = geometry.getBox(1.0f, 1.0f, 1.0f, 0.0f);

	std::vector<Vertex> vertices;
	vertices.reserve(boxMesh.m_vertices.size());

	auto indices = boxMesh.getIndices16();

	for (int i = 0; i < boxMesh.m_vertices.size(); ++i) {
		vertices.emplace_back(boxMesh.m_vertices[i].m_position, DirectX::XMFLOAT4(DirectX::Colors::LightGreen));
	}
	
	m_renderItems.back()->m_objectConstantsUpdates = totalBufferFrameResources;

	m_renderItems.back()->m_mesh = std::make_unique<Yume::Mesh>("BoxMesh", m_renderer->m_device.Get(), m_renderer->m_commandList.Get(), static_cast<void*>(vertices.data()), sizeof(vertices[0]), vertices.size(), static_cast<void*>(indices.data()), sizeof(indices[0]), indices.size());
	m_renderItems.back()->m_mesh->subMeshes["box"] = Yume::SubMesh{ static_cast<UINT>(boxMesh.m_indices.size()), 0, 0 };
}

void Shapes::buildCbvHeap()
{	
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = (static_cast<int>(m_renderItems.size()) + 1) * totalBufferFrameResources;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.ReleaseAndGetAddressOf())));
}

void Shapes::buildConstantBuffers()
{	
	const int totalRenderItems = static_cast<int>(m_renderItems.size());
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCPUDescriptorHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

	auto objectConstantsSize = Yume::nextMultiple256(sizeof(ObjectConstants));

	for (int i = 0; i < totalBufferFrameResources; ++i) {
		auto uploadBuffer = m_bufferFrameResources[i]->m_objectConstants->getResource();

		for (const auto& renderItem : m_renderItems) {
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = uploadBuffer->GetGPUVirtualAddress() + renderItem->m_constantBufferOffset * objectConstantsSize;
			cbvDesc.SizeInBytes = objectConstantsSize;

			m_renderer->m_device->CreateConstantBufferView(&cbvDesc, cbvCPUDescriptorHandle);

			cbvCPUDescriptorHandle.Offset(1, m_renderer->getCbvSrvUavDescriptorHandleIncrementSize());
		}
	}

	auto passConstantsSize = Yume::nextMultiple256(sizeof(PassConstants));
	for (int i = 0; i < totalBufferFrameResources; ++i) {
		auto uploadBuffer = m_bufferFrameResources[i]->m_passConstants->getResource();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = uploadBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = passConstantsSize;

		m_renderer->m_device->CreateConstantBufferView(&cbvDesc, cbvCPUDescriptorHandle);

		cbvCPUDescriptorHandle.Offset(1, m_renderer->getCbvSrvUavDescriptorHandleIncrementSize());
	}
}

void Shapes::buildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE descTable0Range;
	descTable0Range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE descTable1Range;
	descTable1Range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].InitAsDescriptorTable(1, &descTable0Range);
	rootParameters[1].InitAsDescriptorTable(1, &descTable1Range);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRoot;
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootError;

	HRESULT hrSerializedRoot = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, serializedRoot.GetAddressOf(), serializedRootError.GetAddressOf());

	if (serializedRootError)
	{
		YM_ERROR(static_cast<char*>(serializedRootError->GetBufferPointer()));
	}

	YM_THROW_IF_FAILED_DX_EXCEPTION(hrSerializedRoot);

	m_renderer->m_device->CreateRootSignature(0, serializedRoot->GetBufferPointer(), serializedRoot->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf()));
}

void Shapes::buildShadersAndInputLayout()
{
	m_vsByteCode = m_renderer->compileShader(L"Shaders/VS.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = m_renderer->compileShader(L"Shaders/PS.hlsl", nullptr, "PS", "ps_5_0");

	m_inputLayout = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void Shapes::buildPipelineStateObject()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
	ZeroMemory(&pipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	pipelineStateDesc.pRootSignature = m_rootSignature.Get();
	pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsByteCode.Get());
	pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(m_psByteCode.Get());
	pipelineStateDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ m_inputLayout.data(), static_cast<UINT>(m_inputLayout.size()) };
	pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineStateDesc.DSVFormat = m_renderer->m_depthStencilFormat;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = m_renderer->m_backBufferFormat;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;

	YM_THROW_IF_FAILED_DX_EXCEPTION(m_renderer->m_device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(m_pipelineStateObject.ReleaseAndGetAddressOf())));
}

void Shapes::handleMouseMove(const Yume::Event& event) {
	if (event.getEventType() == Yume::EventType::MouseMoved) {
		const Yume::MouseMovedEvent& mouseMovedEvent = static_cast<const Yume::MouseMovedEvent&>(event);
		if (mouseMovedEvent.isDelta()) {
			m_mouseXDelta = MOUSE_SENSITIVITY * mouseMovedEvent.getX();
			m_mouseYDelta = MOUSE_SENSITIVITY * mouseMovedEvent.getY();
		}
	}
}

void Shapes::handleWindowResize(const Yume::Event& event) {
	if (event.getEventType() == Yume::EventType::WindowResize) {
		const Yume::WindowResizeEvent& windowResizeEvent = static_cast<const Yume::WindowResizeEvent&>(event);

		DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, 1.0f * windowResizeEvent.getWidth() / windowResizeEvent.getHeight(), m_nearZ, m_farZ);
		DirectX::XMStoreFloat4x4(&m_projection, projection);
	}
}