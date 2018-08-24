﻿#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include <string>
#include <vector>
#include "..\Common\DirectXHelper.h"
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

using namespace Dx11Test;

using namespace DirectX;
using namespace Windows::Foundation;


// ファイルから頂点とピクセル シェーダーを読み込み、キューブのジオメトリをインスタンス化します。
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

float Sample3DSceneRenderer::rand_FloatRange(float a, float b) {
	return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

// ウィンドウのサイズが変更されたときに、ビューのパラメーターを初期化します。
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// これは、アプリケーションが縦向きビューまたはスナップ ビュー内にあるときに行うことのできる
	// 変更の簡単な例です。
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// OrientationTransform3D マトリックスは、シーンの方向を表示方向と
	// 正しく一致させるため、ここで事後乗算されます。
	// この事後乗算ステップは、スワップ チェーンのターゲット ビットマップに対して行われるすべての
	//描画呼び出しで実行する必要があります。他のターゲットに対する呼び出しでは、
	// 適用する必要はありません。

	// このサンプルでは、行優先のマトリックスを使用した右辺座標系を使用しています。
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// 視点は (0,0.7,1.5) の位置にあり、y 軸に沿って上方向のポイント (0,-0.1,0) を見ています。
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	count = 0.0f;
}

// フレームごとに 1 回呼び出し、キューブを回転させてから、モデルおよびビューのマトリックスを計算します。
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// 度をラジアンに変換し、秒を回転角度に変換します
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));
		Rotate(radians);
		
		m_currentTime = timer.GetTotalSeconds();

		float time = static_cast<float>(m_currentTime);
		
		// whay it's not so simple.........
		XMVECTOR vtime = XMVectorSet(time, time, time, time);
		XMStoreFloat4(&m_constantBufferData.time, vtime);
	

		/*
		OutputDebugString(L"m_currentTime ");
		OutputDebugString((L""+ std::to_wstring(m_currentTime)).c_str());
		OutputDebugString(L"\n");
		*/
	}
}

//3D キューブ モデルを、ラジアン単位で設定された大きさだけ回転させます。
void Sample3DSceneRenderer::Rotate(float radians)
{
	//更新されたモデル マトリックスをシェーダーに渡す準備をします
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// 追跡時に、出力画面の幅方向を基準としてポインターの位置を追跡することにより、3D キューブを Y 軸に沿って回転させることができます。
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

// 頂点とピクセル シェーダーを使用して、1 つのフレームを描画します。
void Sample3DSceneRenderer::Render()
{
	// 読み込みは非同期です。読み込みが完了した後にのみ描画してください。
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// 定数バッファーを準備して、グラフィックス デバイスに送信します。
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
		);

	// 各頂点は、VertexPositionColor 構造体の 1 つのインスタンスです。
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, //各インデックスは、1 つの 16 ビット符号なし整数 (short) です。
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// 頂点シェーダーをアタッチします。
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	// 定数バッファーをグラフィックス デバイスに送信します。
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
		);

	// ピクセル シェーダーをアタッチします。
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);

	// オブジェクトを描画します。
	context->DrawIndexed(
		m_indexCount,
		0,
		0
		);
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	// シェーダーを非同期で読み込みます。
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// 頂点シェーダー ファイルを読み込んだ後、シェーダーと入力レイアウトを作成します。
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
				)
			);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
				)
			);
	});

	// ピクセル シェーダー ファイルを読み込んだ後、シェーダーと定数バッファーを作成します。
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);

	});


	// 両方のシェーダーの読み込みが完了したら、メッシュを作成します。
	auto createCubeTask = (createPSTask && createVSTask).then([this] () {

		int latitudeBands = 10;
		int longitudeBands = 10;
		
		int vindex = 0;
		VertexPositionColor cubeVertices[121];

		float radius = 0.5f;
		for (int i = 0; i <= latitudeBands; i++) {
			float theta = i * XM_PI / latitudeBands;
			float sinTheta = sin(theta);
			float cosTheta = cos(theta)*radius;

			for (int j = 0; j <= longitudeBands; j++) {
				float phi = j * 2 * XM_PI / longitudeBands;
				float sinPhi = sin(phi)*radius;
				float cosPhi = cos(phi)*radius;

				float x = cosPhi * sinTheta;
				float y = cosTheta;
				float z = sinPhi * sinTheta;
				
				float r = rand_FloatRange(0.0f,1.0f);
				float g = rand_FloatRange(0.0f, 1.0f);
				float b = rand_FloatRange(0.0f, 1.0f);

				cubeVertices[vindex] = { XMFLOAT3(x, y, z), XMFLOAT3(r, g, b) };

				vindex++;
			}
		}

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
				)
			);

		// メッシュのインデックスを読み込みます。インデックスの 3 つ 1 組の値のそれぞれは、次のものを表します
		// 画面上に描画される三角形を表します。
		// たとえば、0,2,1 とは、頂点バッファーのインデックス
		//0、2、1 にある頂点が、このメッシュの
		// 最初の三角形を構成することを意味します。
		unsigned short cubeIndices[726];

		int indCount = 0;
		for (int i = 0; i < latitudeBands; i++) {
			for (int j = 0; j < longitudeBands; j++) {
				int first = (i * (longitudeBands + 1)) + j;
				int second = first + longitudeBands + 1;

				cubeIndices[indCount++] = first;
				cubeIndices[indCount++] = second;
				cubeIndices[indCount++] = first + 1;

				cubeIndices[indCount++] = second;
				cubeIndices[indCount++] = second + 1;
				cubeIndices[indCount++] = first + 1;
			}
		}


		m_indexCount = ARRAY_LENGTH(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
				)
			);
	});

	// キューブが読み込まれたら、オブジェクトを描画する準備が完了します。
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}