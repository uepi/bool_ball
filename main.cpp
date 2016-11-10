#pragma comment(lib,"d3d11.lib")

#pragma comment(lib,"d3dCompiler.lib")
#include <d3d11.h>
#include <d3dCompiler.h>
#include<Windows.h>
#include <tchar.h>
#include <iostream>
#include<vector>
#include <directxmath.h>

using namespace DirectX;
using namespace std;

//定数定義
#define WINDOW_WIDTH 320 //ウィンドウ幅
#define WINDOW_HEIGHT 240 //ウィンドウ高さ
#define D3DX_PI ((FLOAT) 3.141592654f) 
#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))
int pmd1 = 0;
#include "pmd.h"
//安全に解放する
#define SAFE_RELEASE(x) if(x){x->Release(); x=NULL;}

//グローバル変数
HWND hWnd = NULL;
ID3D11Device* Device = NULL;//デバイス
ID3D11DeviceContext* DeviceContext = NULL;
IDXGISwapChain* SwapChain = NULL;//スワップチェイン
ID3D11RenderTargetView* RenderTargetView = NULL;
ID3D11InputLayout* VertexLayout = NULL;
ID3D11Buffer* VertexBuffer = NULL;
ID3D11Buffer* IndexBuffer = NULL;
ID3D11VertexShader* VertexShader = NULL;//頂点シェーダー
ID3D11PixelShader* PixelShader = NULL;//ピクセルシェーダー



struct Vertex3D {
	float pos[3];	//x-y-z
	float col[4];	//r-g-b-a
	float tex[2];
};

//シェーダーに送る
struct ConstantBuffer
{
	XMMATRIX mWorld;		//ワールド変換行列
	XMMATRIX mView;			//ビュー変換行列
	XMMATRIX mProjection;	//透視射影変換行列
};
/*

//頂点データ(三角ポリゴン1枚)
Vertex3D hVectorData[TYOUTEN] = {
	{ { +0.0f, +0.8f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { +0.5f, +0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, +0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { +0.5f, -0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } }
};

*/

//Direct3Dの初期化関数
HRESULT InitD3D(HWND hWnd)
{
	// デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WINDOW_WIDTH;
	sd.BufferDesc.Height = WINDOW_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	D3D_FEATURE_LEVEL  FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_REFERENCE, NULL, 0,
		&FeatureLevel, 1, D3D11_SDK_VERSION, &sd, &SwapChain, &Device, NULL, &DeviceContext)))
	{
		return FALSE;
	}
	//レンダーターゲットビューの作成
	ID3D11Texture2D *BackBuffer;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
	Device->CreateRenderTargetView(BackBuffer, NULL, &RenderTargetView);
	BackBuffer->Release();
	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, NULL);

	//ラスタライザ
	ID3D11RasterizerState* hpRasterizerState = NULL;
	D3D11_RASTERIZER_DESC hRasterizerDesc = {
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE,	//ポリゴンの裏表を無くす
		FALSE,
		0,
		0.0f,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE
	};
	Device->CreateRasterizerState(&hRasterizerDesc, &hpRasterizerState);

	//ラスタライザーをコンテキストに設定
	DeviceContext->RSSetState(hpRasterizerState);

	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContext->RSSetViewports(1, &vp);


	return S_OK;
}

HRESULT CreateShader()
{
	//hlslファイル読み込み
	ID3DBlob *pCompiledShader = NULL;
	ID3DBlob *pErrors = NULL;

	//ブロブから頂点シェーダー作成
	D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "VS", "vs_4_0", 0, 0, &pCompiledShader, nullptr);


	if (FAILED(Device->CreateVertexShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &VertexShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, "頂点シェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//頂点インプットレイアウトを定義 
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	//頂点インプットレイアウトを作成
	if (FAILED(Device->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &VertexLayout)))
		return FALSE;

	SAFE_RELEASE(pCompiledShader);
	//頂点インプットレイアウトをセット
	DeviceContext->IASetInputLayout(VertexLayout);


	//ブロブからピクセルシェーダー作成
	D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "PS", "ps_4_0", 0, 0, &pCompiledShader, nullptr);

	if (FAILED(Device->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &PixelShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, "ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);
}

void CreateConstantBuffer()
{
	//constantバッファ生成
	ID3D11Buffer* hpConstantBuffer = NULL;
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);
	if (FAILED(Device->CreateBuffer(&hBufferDesc, NULL, &hpConstantBuffer))) {
		MessageBoxW(hWnd, L"Create ConstantBuffer", L"Err", MB_ICONSTOP);
	}

	//ワールド変換用行列を生成
	XMMATRIX hWorld;		//ワールド変換行列
	hWorld = XMMatrixIdentity();

	XMMATRIX hView;		//ビュー変換行列
	XMVECTOR hEye = XMVectorSet(5.0f, 0.0f, -2.0f, 0.0f);	//カメラの位置
	XMVECTOR hAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		//焦点の位置
	XMVECTOR hUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	hView = XMMatrixLookAtLH(hEye, hAt, hUp);

	XMMATRIX hProjection;	//透視射影変換行列
	hProjection = XMMatrixPerspectiveFovLH(D3DXToRadian(45.0f), 16.0f / 9.0f, 0.1f, 1000.0f);

	ConstantBuffer hConstantBuffer;
	hConstantBuffer.mWorld = XMMatrixTranspose(hWorld);
	hConstantBuffer.mView = XMMatrixTranspose(hView);
	hConstantBuffer.mProjection = XMMatrixTranspose(hProjection);
	DeviceContext->UpdateSubresource(hpConstantBuffer, 0, NULL, &hConstantBuffer, 0, 0);

	//コンテキストに設定
	DeviceContext->VSSetConstantBuffers(0, 1, &hpConstantBuffer);
}

void CreateVertexData(pmd* _model, vector<Vertex3D>& _data, unsigned short** _index)
{
	for (int i = 0; i < _model->vert_count; ++i) {
		Vertex3D tmp;

		tmp.pos[0] = _model->vertex[i].pos[0] * 0.1f;
		tmp.pos[1] = _model->vertex[i].pos[1] * 0.1f;
		tmp.pos[2] = _model->vertex[i].pos[2] * 0.1f;
		tmp.col[0] = 0.0f;
		tmp.col[1] = 0.0f;
		tmp.col[2] = 0.5f;
		tmp.col[3] = 1.0f;
		tmp.tex[0] = 0.0f;
		tmp.tex[1] = 1.0f;

		_data.push_back(tmp);
	}

	//インデックスデータを取得
	int INDEXSU = _model->face_vert_count;
	*_index = new unsigned short[INDEXSU];

	for (int i = 0; i < INDEXSU; i++) {
		_index[0][i] = _model->face_vert_index[i];
	}
}

void CreateVertexBuffer(vector<Vertex3D>& _data)
{
	//頂点バッファ作成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3D) * _data.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = (void*)&_data[0];
	if (FAILED(Device->CreateBuffer(&bd, &InitData, &VertexBuffer)))
		return;
}

void CreateIndexBuffer(unsigned short* _index,pmd* _model)
{
	
	//インデックスバッファ作成
	D3D11_BUFFER_DESC hBufferDesc;
	ZeroMemory(&hBufferDesc, sizeof(hBufferDesc));

	hBufferDesc.ByteWidth = _model->face_vert_count * sizeof(unsigned short);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(unsigned short);

	D3D11_SUBRESOURCE_DATA hSubResourceData;
	hSubResourceData.pSysMem = _index;
	hSubResourceData.SysMemPitch = 0;
	hSubResourceData.SysMemSlicePitch = 0;

	if (FAILED(Device->CreateBuffer(&hBufferDesc, &hSubResourceData, &IndexBuffer))) {
		MessageBoxW(hWnd, L"CreateBuffer Index", L"Err", MB_ICONSTOP);
		//goto End;
	}
}

//レンダリング
VOID Render(pmd* _model)
{
	float ClearColor[4] = { 0,0,1,1 }; //消去色
	DeviceContext->ClearRenderTargetView(RenderTargetView, ClearColor);//画面クリア 
																	   //使用するシェーダーの登録
	DeviceContext->VSSetShader(VertexShader, NULL, 0);
	DeviceContext->PSSetShader(PixelShader, NULL, 0);


	unsigned int stride = sizeof(Vertex3D);
	unsigned int offset = 0;
	//頂点バッファセット
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
	//そのインデックスバッファをコンテキストに設定
	DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//プリミティブ・トポロジーをセット
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->DrawIndexed(_model->face_vert_count, 0, 0);
	//プリミティブをレンダリング

}
//終了時解放処理
VOID Cleanup()
{
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(VertexBuffer);
	SAFE_RELEASE(VertexLayout);
	SAFE_RELEASE(IndexBuffer);
	SAFE_RELEASE(SwapChain);
	SAFE_RELEASE(RenderTargetView);
	SAFE_RELEASE(DeviceContext);
	SAFE_RELEASE(Device);
}
//メッセージプロシージャ
LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY://終了時
		Cleanup();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}



//メイン関数
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, INT iCmdShow)
{
	//ウインドウクラスの登録
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"Window1", NULL };
	RegisterClassEx(&wc);
	//タイトルバーとウインドウ枠の分を含めてウインドウサイズを設定
	RECT rect;
	SetRect(&rect, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	rect.right = rect.right - rect.left;
	rect.bottom = rect.bottom - rect.top;
	rect.top = 0;
	rect.left = 0;
	//ウインドウの生成
	hWnd = CreateWindow("Window1", "三角ポリゴン",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom,
		NULL, NULL, wc.hInstance, NULL);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	//Direct3D初期化
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		//シェーダー読み込み
		if (FAILED(CreateShader()))
			return 0;

		CreateConstantBuffer();

		//モデル読み込み
		pmd *modeldata;
		modeldata = new pmd("board.pmd");
		vector<Vertex3D>TYOUTEN;

		unsigned short* hIndexData = nullptr;

		CreateVertexData(modeldata, TYOUTEN, &hIndexData);
		CreateVertexBuffer(TYOUTEN);
		CreateIndexBuffer(hIndexData,modeldata);

		//ウインドウ表示
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Render(modeldata);

				SwapChain->Present(0, 0);//フリップ
			}
		}
		delete[] hIndexData;
		delete modeldata;
	}
	
	Cleanup();
	//終了
	return 0;
}













//// pmdのデータをファイル名から入れる
//void pmdload(){
//	
//	pmd *modeldata;
//	modeldata = new pmd("board.pmd");
//
//	vector<Vertex3D>TYOUTEN;
//
//	for (int i = 0; i < modeldata->vert_count; ++i) {
//		Vertex3D tmp;
//
//		tmp.pos[0] = modeldata->vertex[i].pos[0];
//		tmp.pos[1] = modeldata->vertex[i].pos[1];
//		tmp.pos[2] = modeldata->vertex[i].pos[2];
//		tmp.col[0] = 0.0f;
//		tmp.col[1] = 0.0f;
//		tmp.col[2] = 0.5f;
//		tmp.col[3] = 1.0f;
//		tmp.tex[0] = 0.0f;
//		tmp.tex[1] = 1.0f;
//
//		TYOUTEN.push_back(tmp);
//	}
//	//インデックスデータを取得
//	int INDEXSU = modeldata->face_vert_count;
//
//	unsigned short *hIndexData;
//	hIndexData = new unsigned short[INDEXSU];
//
//	for (int i = 0; i < INDEXSU; i++) {
//		hIndexData[i] = modeldata->face_vert_index[i];
//	}
//	
//	//インデックスバッファ作成
//	D3D11_BUFFER_DESC hBufferDesc;
//	hBufferDesc.ByteWidth = INDEXSU * sizeof(unsigned short);
//	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	hBufferDesc.CPUAccessFlags = 0;
//	hBufferDesc.MiscFlags = 0;
//	//hBufferDesc.StructureByteStride = sizeof(unsigned short);
//
//	D3D11_SUBRESOURCE_DATA hSubResourceData;
//	hSubResourceData.pSysMem = hIndexData;
//	hSubResourceData.SysMemPitch = 0;
//	hSubResourceData.SysMemSlicePitch = 0;
//
//	ID3D11Buffer* hpIndexBuffer;
//	if (FAILED(Device->CreateBuffer(&hBufferDesc, &hSubResourceData, &hpIndexBuffer))) {
//		MessageBoxW(hWnd, L"CreateBuffer Index", L"Err", MB_ICONSTOP);
//		//goto End;
//	}
//
//	//そのインデックスバッファをコンテキストに設定
//	DeviceContext->IASetIndexBuffer(hpIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
//	
//
//}