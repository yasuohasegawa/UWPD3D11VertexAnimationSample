#include "pch.h"
#include "SampleUIRenderer.h"

#include "Common/DirectXHelper.h"

using namespace Dx11Test;
using namespace Microsoft::WRL;

SampleUIRenderer::SampleUIRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) : 
	m_text(L""),
	m_textTitle(L""),
	m_deviceResources(deviceResources)
{
	ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));
	ZeroMemory(&m_textTitleMetrics, sizeof(DWRITE_TEXT_METRICS));

	ComPtr<IDWriteTextFormat> textFormat;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			32.0f,
			L"en-US",
			&textFormat
			)
		);

	ComPtr<IDWriteTextFormat> textTitleFormat;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			36.0f,
			L"en-US",
			&textTitleFormat
		)
	);

	DX::ThrowIfFailed(
		textFormat.As(&m_textFormat)
		);

	DX::ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
		);

	DX::ThrowIfFailed(
		textTitleFormat.As(&m_textTitleFormat)
	);

	DX::ThrowIfFailed(
		m_textTitleFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
	);

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
		);	

	CreateDeviceDependentResources();
}

void SampleUIRenderer::Update(DX::StepTimer const& timer)
{
	uint32 fps = timer.GetFramesPerSecond();

	m_text = (fps > 0) ? std::to_wstring(fps) + L" FPS" : L" - FPS";
	m_textTitle = L"Test Title";

	// fps text
	ComPtr<IDWriteTextLayout> textLayout;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			m_text.c_str(),
			(uint32) m_text.length(),
			m_textFormat.Get(),
			240.0f,// text width
			50.0f, // text height
			&textLayout
			)
		);

	DX::ThrowIfFailed(
		textLayout.As(&m_textLayout)
		);

	DX::ThrowIfFailed(
		m_textLayout->GetMetrics(&m_textMetrics)
		);

	// title text
	ComPtr<IDWriteTextLayout> textLayout2;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextLayout(
			m_textTitle.c_str(),
			(uint32)m_textTitle.length(),
			m_textTitleFormat.Get(),
			500.0f, // // text width
			50.0f, // text height
			&textLayout2
		)
	);

	DX::ThrowIfFailed(
		textLayout2.As(&m_textTitleLayout)
	);

	DX::ThrowIfFailed(
		m_textTitleLayout->GetMetrics(&m_textTitleMetrics)
	);
	
}

void SampleUIRenderer::Render()
{
	ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
	Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	/*
	// you can move the context.
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		logicalSize.Width - m_textMetrics.layoutWidth,
		logicalSize.Height - m_textMetrics.height
		);

	context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());
	*/

	DX::ThrowIfFailed(
		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
		);

	context->DrawTextLayout(
		D2D1::Point2F(logicalSize.Width - m_textMetrics.layoutWidth, logicalSize.Height - m_textMetrics.height),
		m_textLayout.Get(),
		m_whiteBrush.Get()
		);


	DX::ThrowIfFailed(
		m_textTitleFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
	);

	context->DrawTextLayout(
		D2D1::Point2F(-(m_textTitleMetrics.layoutWidth - m_textTitleMetrics.width), 0.0f),
		m_textTitleLayout.Get(),
		m_whiteBrush.Get()
	);

	// test drawing lines
	float lineSize = 30.0f;
	context->DrawLine(
		D2D1::Point2F(logicalSize.Width / 2.0f - lineSize, logicalSize.Height / 2.0f),
		D2D1::Point2F(logicalSize.Width / 2.0f + lineSize, logicalSize.Height / 2.0f),
		m_whiteBrush.Get(),
		3.0f
	);
	context->DrawLine(
		D2D1::Point2F(logicalSize.Width / 2.0f, logicalSize.Height / 2.0f - lineSize),
		D2D1::Point2F(logicalSize.Width / 2.0f, logicalSize.Height / 2.0f + lineSize),
		m_whiteBrush.Get(),
		3.0f
	);

	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
	}

	context->RestoreDrawingState(m_stateBlock.Get());
}

void SampleUIRenderer::CreateDeviceDependentResources()
{
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
		);
}
void SampleUIRenderer::ReleaseDeviceDependentResources()
{
	m_whiteBrush.Reset();
}