#pragma comment(lib,"d3d11.lib")

#pragma comment(lib,"d3dCompiler.lib")
#include <d3d11.h>
#include <d3dCompiler.h>
#include<Windows.h>
#include <tchar.h>
#include <iostream>
#include<vector>
#include <directxmath.h>
#include <tchar.h>
using namespace DirectX;
using namespace std;

//定数定義
#define WINDOW_WIDTH 320 //ウィンドウ幅
#define WINDOW_HEIGHT 240 //ウィンドウ高さ
#define D3DX_PI ((FLOAT) 3.141592654f) 
#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))

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
ID3D11VertexShader* VertexShader = NULL;//頂点シェーダー
ID3D11PixelShader* PixelShader = NULL;//ピクセルシェーダー
									
class VECTOR3
{
public:
	VECTOR3(float, float, float);
	float x;
	float y;
	float z;
};
VECTOR3::VECTOR3(float a, float b, float c)
{
	x = a; y = b; z = c;
}
//頂点の構造体
struct SimpleVertex
{
	VECTOR3 Pos;//位置
};
const int TYOUTEN = 5;

struct Vertex3D {
	float pos[3];	//x-y-z
	float col[4];	//r-g-b-a
	//float tex[2];

};
//頂点データ(三角ポリゴン1枚)
Vertex3D hVectorData[TYOUTEN] = {
	{ { +0.0f, +0.8f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { +0.5f, +0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, +0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { +0.5f, -0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, +0.5f },{ 1.0f, 1.0f, 1.0f, 1.0f } }
};

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
	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContext->RSSetViewports(1, &vp);
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

	//hlslファイル読み込み
	ID3DBlob *pCompiledShader = NULL;
	ID3DBlob *pErrors = NULL;
	//ブロブから頂点シェーダー作成
	D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "VS", "vs_4_0", 0, 0, &pCompiledShader, nullptr);
	SAFE_RELEASE(pErrors);
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
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	//頂点インプットレイアウトを作成
	if (FAILED(Device->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &VertexLayout)))
		return FALSE;
	//頂点インプットレイアウトをセット
	DeviceContext->IASetInputLayout(VertexLayout);
	//ブロブからピクセルシェーダー作成
	D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "PS", "ps_4_0", 0, 0, &pCompiledShader, nullptr);
	SAFE_RELEASE(pErrors);
	if (FAILED(Device->CreatePixelShader(pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), NULL, &PixelShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		MessageBox(0, "ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3D) * TYOUTEN;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = hVectorData;
	if (FAILED(Device->CreateBuffer(&bd, &InitData, &VertexBuffer)))
		return FALSE;
	//バーテックスバッファーをセット
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
	//プリミティブ・トポロジーをセット
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	/*
	//ワールド変換用行列を生成
	XMMATRIX hWorld;		//ワールド変換行列
	hWorld = XMMatrixIdentity();

	XMMATRIX hView;		//ビュー変換行列
	XMVECTOR hEye = XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);	//カメラの位置
	XMVECTOR hAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		//焦点の位置
	XMVECTOR hUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	hView = XMMatrixLookAtLH(hEye, hAt, hUp);

	XMMATRIX hProjection;	//透視射影変換行列
	hProjection = XMMatrixPerspectiveFovLH(D3DXToRadian(45.0f), 16.0f / 9.0f, 0.0f, 1000.0f);

	//それらをシェーダーに送る
	struct ConstantBuffer
	{
		XMMATRIX mWorld;		//ワールド変換行列
		XMMATRIX mView;			//ビュー変換行列
		XMMATRIX mProjection;	//透視射影変換行列
	};


	//このconstatnバッファを登録する場所がシェーダー側にないわ
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
		//goto End;
	}

	ConstantBuffer hConstantBuffer;
	hConstantBuffer.mWorld = XMMatrixTranspose(hWorld);
	hConstantBuffer.mView = XMMatrixTranspose(hView);
	hConstantBuffer.mProjection = XMMatrixTranspose(hProjection);
	DeviceContext->UpdateSubresource(hpConstantBuffer, 0, NULL, &hConstantBuffer, 0, 0);

	//コンテキストに設定
	DeviceContext->VSSetConstantBuffers(0, 1, &hpConstantBuffer);
	*/
	return S_OK;
}
//レンダリング
VOID Render(int num)
{
	float ClearColor[4] = { 0,0,1,1 }; //消去色
	DeviceContext->ClearRenderTargetView(RenderTargetView, ClearColor);//画面クリア 
																	   //使用するシェーダーの登録
	DeviceContext->VSSetShader(VertexShader, NULL, 0);
	DeviceContext->PSSetShader(PixelShader, NULL, 0);
	//プリミティブをレンダリング
	DeviceContext->Draw(num, 0);
	SwapChain->Present(0, 0);//フリップ
}
//終了時解放処理
VOID Cleanup()
{
	SAFE_RELEASE(VertexShader);
	SAFE_RELEASE(PixelShader);
	SAFE_RELEASE(VertexBuffer);
	SAFE_RELEASE(VertexLayout);
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
// pmdのデータをファイル名から入れる
void pmdload(){
	
	pmd *modeldata;
	modeldata = new pmd("board.pmd");

	vector<Vertex3D>TYOUTEN;

	for (int i = 0; i < modeldata->vert_count; ++i) {
		Vertex3D tmp;

		tmp.pos[0] = modeldata->vertex[i].pos[0];
		tmp.pos[1] = modeldata->vertex[i].pos[1];
		tmp.pos[2] = modeldata->vertex[i].pos[2];
		tmp.col[0] = 0.0f;
		tmp.col[1] = 0.0f;
		tmp.col[2] = 0.5f;
		tmp.col[3] = 1.0f;
		//tmp.tex[0] = 0.0f;
		//tmp.tex[1] = 1.0f;

		TYOUTEN.push_back(tmp);
	}
	//インデックスデータを取得
	int INDEXSU = modeldata->face_vert_count;

	unsigned short *hIndexData;
	hIndexData = new unsigned short[INDEXSU];

	for (int i = 0; i < INDEXSU; i++) {
		hIndexData[i] = modeldata->face_vert_index[i];
	}
	
	

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
				Render(TYOUTEN);
			}
		}
	}
	//終了
	return 0;
}