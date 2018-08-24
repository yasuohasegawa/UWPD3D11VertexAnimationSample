#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleUIRenderer.h"

// Direct2D
namespace Dx11Test
{
	class Dx11TestMain : public DX::IDeviceNotify
	{
	public:
		Dx11TestMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Dx11TestMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleUIRenderer> m_uiRenderer;

		DX::StepTimer m_timer;
	};
}