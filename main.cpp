#pragma comment(lib,"d3d11.lib")

#pragma comment(lib,"d3dCompiler.lib")
#include <d3d11.h>
#include <d3dCompiler.h>
#include<Windows.h>
#include <tchar.h>
#include <iostream>
#include<vector>
#include <directxmath.h>
#include<debugapi.h>
#include"Ball.h"
using namespace DirectX;
using namespace std;

//定数定義
#define WINDOW_WIDTH 960 //ウィンドウ幅
#define WINDOW_HEIGHT 540 //ウィンドウ高さ
#define D3DX_PI ((FLOAT) 3.141592654f) 
#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))

//#include "pmd.h"
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
ID3D11Buffer* hpConstantBuffer = NULL;// コンスタントバッファ
ID3D11DepthStencilView* DepthStencilView = NULL;//深度ステンシルビュー

ID3D11Buffer* Indexball = NULL;
//ConstantBuffer hConstantBuffer;
XMMATRIX hWorld;		//ワールド変換行列


/*struct Vertex3D {
	float pos[3];	//x-y-z
	float col[4];	//r-g-b-a
	float tex[2];
};
struct Vector3 {
	float x;
	float y;
	float z;
};*/
struct Camera {
	float pos[4] = { 0.0f,22.0f,-2.0f,0.0f };
	float at[4] = { 0.0f,0.0f,0.0f,0.0f };
	float up[4] = { 0.0f,1.0f,0.0f,0.0f };
};
Camera camera ;

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

	ID3D11Texture2D* hpTexture2dDepth = NULL;
	D3D11_TEXTURE2D_DESC hTexture2dDesc;
	hTexture2dDesc.Width = sd.BufferDesc.Width;
	hTexture2dDesc.Height = sd.BufferDesc.Height;
	hTexture2dDesc.MipLevels = 1;
	hTexture2dDesc.ArraySize = 1;
	hTexture2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	hTexture2dDesc.SampleDesc = sd.SampleDesc;
	hTexture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	hTexture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hTexture2dDesc.CPUAccessFlags = 0;
	hTexture2dDesc.MiscFlags = 0;
	if (FAILED(Device->CreateTexture2D(&hTexture2dDesc, NULL, &hpTexture2dDepth))) {
		MessageBoxW(hWnd, L"CreateTexture2D", L"Err", MB_ICONSTOP);
		
	}

	ID3D11DepthStencilState* DepthStencilState = NULL;
	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
	ZeroMemory(&DepthStencilDesc, sizeof(DepthStencilDesc));
	DepthStencilDesc.DepthEnable = TRUE;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DepthStencilDesc.StencilEnable = FALSE;

	Device->CreateDepthStencilState(&DepthStencilDesc, &DepthStencilState);
	//ステンシルターゲット作成

	D3D11_DEPTH_STENCIL_VIEW_DESC hDepthStencilViewDesc;
	hDepthStencilViewDesc.Format = hTexture2dDesc.Format;
	hDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hDepthStencilViewDesc.Flags = 0;
	if (FAILED(Device->CreateDepthStencilView(hpTexture2dDepth, &hDepthStencilViewDesc, &DepthStencilView))) {
		MessageBoxW(hWnd, L"CreateDepthStencilView", L"Err", MB_ICONSTOP);
	
	}


	DeviceContext->OMSetDepthStencilState(DepthStencilState,0);
	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContext->RSSetViewports(1, &vp);
	DeviceContext->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);

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
	
	XMVECTOR hEye = XMVectorSet(camera.pos[0], camera.pos[1], camera.pos[2], camera.pos[3]);	//カメラの位置
	XMVECTOR hAt = XMVectorSet(camera.at[0], camera.at[1], camera.at[2], camera.at[3]);		//焦点の位置
	XMVECTOR hUp = XMVectorSet(camera.up[0], camera.up[1], camera.up[2], camera.up[3]);
	
	hView = XMMatrixLookAtLH(hEye, hAt, hUp);

	XMMATRIX hProjection;	//透視射影変換行列
	hProjection = XMMatrixPerspectiveFovLH(D3DXToRadian(110.0f), 16.0f / 9.0f, 0.1f, 1000.0f);

	ConstantBuffer hConstantBuffer;
	hConstantBuffer.mWorld = XMMatrixTranspose(hWorld);
	hConstantBuffer.mView = XMMatrixTranspose(hView);
	hConstantBuffer.mProjection = XMMatrixTranspose(hProjection);
	DeviceContext->UpdateSubresource(hpConstantBuffer, 0, NULL, &hConstantBuffer, 0, 0);

	//コンテキストに設定
	DeviceContext->VSSetConstantBuffers(0, 1, &hpConstantBuffer);
}
void set() {
	//ワールド変換用行列を生成
	XMMATRIX hWorld;		//ワールド変換行列
	hWorld = XMMatrixIdentity();

	XMMATRIX hView;		//ビュー変換行列

	XMVECTOR hEye = XMVectorSet(camera.pos[0], camera.pos[1], camera.pos[2], camera.pos[3]);	//カメラの位置
	XMVECTOR hAt = XMVectorSet(camera.at[0], camera.at[1], camera.at[2], camera.at[3]);		//焦点の位置
	XMVECTOR hUp = XMVectorSet(camera.up[0], camera.up[1], camera.up[2], camera.up[3]);

	hView = XMMatrixLookAtLH(hEye, hAt, hUp);
	

	XMMATRIX hProjection;	//透視射影変換行列
	hProjection = XMMatrixPerspectiveFovLH(D3DXToRadian(100.0f), 16.0f / 9.0f, 0.1f, 1000.0f);

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

		tmp.pos[0] = _model->vertex[i].pos[0];
		tmp.pos[1] = _model->vertex[i].pos[1];
		tmp.pos[2] = _model->vertex[i].pos[2];
		tmp.col[0] = 0.0f;
		tmp.col[1] = 0.0f;
		tmp.col[2] = 0.0f;
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
	
	int COLORSU = _model->material_count;

	int cnt_idx = 0;	//インデックスカウンター
	for (int i = 0; i < COLORSU; i++) {
		//マテリアルにあるカウンター分回す
		for (int j = 0; j < _model->material[i].face_vert_count; j++) {
			//インデックスから頂点座標の行数を取り出す
			int pos_vec = _model->face_vert_index[cnt_idx];
			cnt_idx++;	//インデックスカウンターを進める

						//その頂点座標のカラーが現在のマテリアルカラー
			_data[pos_vec].col[0] = _model->material[i].diffuse_color[0];	//R
			_data[pos_vec].col[1] = _model->material[i].diffuse_color[1];	//G
			_data[pos_vec].col[2] = _model->material[i].diffuse_color[2];	//B
			_data[pos_vec].col[3] = _model->material[i].alpha;	//A
		}
		
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

void ballCreateVertexBuffer(vector<Vertex3D>& _data, Ball& balls)
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
	if (FAILED(Device->CreateBuffer(&bd, &InitData, &balls.VertexBuffer)))
		return;
}

void ballCreateIndexBuffer(unsigned short* _index, pmd* _model,Ball& balls)
{
	ID3D11Buffer* IndexBuffer = NULL;
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

	if (FAILED(Device->CreateBuffer(&hBufferDesc, &hSubResourceData, &balls.IndexBuffer))) {
		MessageBoxW(hWnd, L"CreateBuffer Index", L"Err", MB_ICONSTOP);
		//goto End;
	}
}

//レンダリング
VOID Render(pmd* _model, ID3D11Buffer* VBuffer, ID3D11Buffer* IBuffer)
{
	float ClearColor[4] = { 0,0,1,1 }; //消去色
	DeviceContext->ClearRenderTargetView(RenderTargetView, ClearColor);//画面クリア 
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);																   //使用するシェーダーの登録
	DeviceContext->VSSetShader(VertexShader, NULL, 0);
	DeviceContext->PSSetShader(PixelShader, NULL, 0);


	unsigned int stride = sizeof(Vertex3D);
	unsigned int offset = 0;
	//頂点バッファセット
	DeviceContext->IASetVertexBuffers(0, 1, &VBuffer, &stride, &offset);
	//そのインデックスバッファをコンテキストに設定
	DeviceContext->IASetIndexBuffer(IBuffer, DXGI_FORMAT_R16_UINT, 0);

	//プリミティブ・トポロジーをセット
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	int vertex_start = 0;
	for (int i = 0; i < _model->material_count;i++) {
				
				DeviceContext->DrawIndexed(_model->material[i].face_vert_count, vertex_start, 0);
				vertex_start = vertex_start + _model->material[i].face_vert_count;//マテリアルの頂点数すすめる
	}		

	

}

VOID Renderball(pmd* _model, ID3D11Buffer* VBuffer, ID3D11Buffer* IBuffer)
{

	DeviceContext->VSSetShader(VertexShader, NULL, 0);
	DeviceContext->PSSetShader(PixelShader, NULL, 0);


	unsigned int stride = sizeof(Vertex3D);
	unsigned int offset = 0;
	//頂点バッファセット
	DeviceContext->IASetVertexBuffers(0, 1, &VBuffer, &stride, &offset);
	//そのインデックスバッファをコンテキストに設定
	DeviceContext->IASetIndexBuffer(IBuffer, DXGI_FORMAT_R16_UINT, 0);

	//プリミティブ・トポロジーをセット
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	
	

	DeviceContext->DrawIndexed(_model->face_vert_count,0, 0);
		
	



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
	
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
			camera.pos[0] = camera.pos[0] - 10.0f;
			
			OutputDebugString("L");
			break;

		case VK_RIGHT:
			camera.pos[0] = camera.pos[0] + 10.0f;
			OutputDebugString("R");
			break;
		case VK_UP:
			camera.pos[1] = camera.pos[1] + 10.0f;
			break;
		case VK_DOWN:
			camera.pos[1] = camera.pos[1] - 10.0f;
			break;
		case VK_F9:
			camera.pos[2] = camera.pos[2] + 10.0f;
			break;
		case VK_F10:
			camera.pos[2] = camera.pos[2] - 10.0f;
			break;
		case VK_F1:
			camera.at[0] = camera.at[0] + 10.0f;
			break;
		case VK_F2:
			camera.at[1] = camera.at[1] + 10.0f;
			break;
		case VK_F3:
			camera.at[0] = camera.at[0] - 10.0f;
			break;
		case VK_F4:
			camera.at[1] = camera.at[1] - 10.0f;
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void createBall(Vector3 positions) {
	
	XMMATRIX hTrans;
	hTrans = XMMatrixTranslation(positions.x, 0.0f, positions.z);
	hWorld = XMMatrixMultiply(hWorld, hTrans);

	ConstantBuffer hConstantBuffer;
	hConstantBuffer.mWorld = XMMatrixTranspose(hWorld);
	DeviceContext->UpdateSubresource(hpConstantBuffer, 0, NULL, &hConstantBuffer, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &hpConstantBuffer);
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
	const float root3 = 1.7320508f;
	const float r = 0.036f + 0.0001f;
	const float d = 0.8f;

	
	
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
		
		vector<Ball*> balls;
		Vector3 positions[15];
		int num = 0;
		pmd *model[15];
		LPTSTR filenames[15] = {
			TEXT("1.pmd"), TEXT("2.pmd"), TEXT("3.pmd"), TEXT("4.pmd"), TEXT("5.pmd"),
			TEXT("6.pmd"), TEXT("7.pmd"), TEXT("8.pmd"), TEXT("9.pmd"), TEXT("10.pmd"),
			TEXT("11.pmd"), TEXT("12.pmd"), TEXT("13.pmd"), TEXT("14.pmd"), TEXT("15.pmd")
		};
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < i + 1; ++j) {
				model[num] = new pmd(filenames[num]);
				positions[num].x = i*r - j * 2 * r;
				positions[num].y = 9.8;
				positions[num].z = d + i*root3*r;
				balls.push_back(new Ball(model[num], positions[num]));
				num++;
			}
		}
		
		
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
				set();
				Render(modeldata, VertexBuffer, IndexBuffer);
				for (int i = 0; i < 15; i++) {
					ConstantBuffer hConstantBuffer;
					hConstantBuffer.mWorld = XMMatrixTranspose(balls[i]->World);

					DeviceContext->UpdateSubresource(hpConstantBuffer, 0, NULL, &hConstantBuffer, 0, 0);
					DeviceContext->VSSetConstantBuffers(0, 1, &hpConstantBuffer);
					Renderball(model[i], balls[i]->VertexBuffer, balls[i]->IndexBuffer);
					
				}
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