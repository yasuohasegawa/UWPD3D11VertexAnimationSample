#include "pch.h"
#include "Dx11TestMain.h"
#include "Common\DirectXHelper.h"

using namespace Dx11Test;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

Dx11TestMain::Dx11TestMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	m_deviceResources->RegisterDeviceNotify(this);

	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_uiRenderer = std::unique_ptr<SampleUIRenderer>(new SampleUIRenderer(m_deviceResources));

	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

Dx11TestMain::~Dx11TestMain()
{
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

void Dx11TestMain::CreateWindowSizeDependentResources() 
{
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void Dx11TestMain::Update() 
{
	m_timer.Tick([&]()
	{
		m_sceneRenderer->Update(m_timer);
		m_uiRenderer->Update(m_timer);
	});
}

bool Dx11TestMain::Render() 
{
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_sceneRenderer->Render();
	m_uiRenderer->Render();

	return true;
}

void Dx11TestMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_uiRenderer->ReleaseDeviceDependentResources();
}

void Dx11TestMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_uiRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
