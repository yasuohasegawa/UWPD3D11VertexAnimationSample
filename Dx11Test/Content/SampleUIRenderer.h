﻿#pragma once

#include <string>
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"

namespace Dx11Test
{
	class SampleUIRenderer
	{
	public:
		SampleUIRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// text
		std::wstring                                    m_text;
		DWRITE_TEXT_METRICS	                            m_textMetrics;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_whiteBrush;
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;

		Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textFormat;

		std::wstring                                    m_textTitle;
		DWRITE_TEXT_METRICS	                            m_textTitleMetrics;
		Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textTitleLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textTitleFormat;
	};
}