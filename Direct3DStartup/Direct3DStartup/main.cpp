#include <Windows.h>
#include <string>
#include <D3D11.h>
#include <DirectXMath.h>

using namespace DirectX;
using namespace std;

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

//重要的全局变量
//在后面学习过程中，这些全局将作为框架类的成员变量存在
HINSTANCE	g_hInstance(NULL);
HWND		g_hWnd(NULL);

wstring		g_clsName(L"d3d11");
wstring		g_wndTitle(L"D3D11初始化");

UINT		g_winWidth(640);
UINT		g_winHeight(480);

//D3D相关全局变量
ID3D11Device		*g_device(NULL);
ID3D11DeviceContext	*g_deviceContext(NULL);
IDXGISwapChain		*g_swapChain(NULL);

ID3D11DepthStencilView	*g_depthStencilView(NULL);
ID3D11RenderTargetView	*g_renderTargetView(NULL);

//初始化
BOOL InitWin32();
//初始化D3D
BOOL InitD3D();
void Render();
//主循环
int	 Run();
//回收资源
void Release();

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow)
{
	g_hInstance = hInstance;

	if (!InitWin32())
		return -1;
	if (!InitD3D())
		return -1;

	return Run();
}

BOOL InitWin32()
{
	//创建窗口类
	WNDCLASS wndcls;
	wndcls.cbClsExtra = 0;
	wndcls.cbWndExtra = 0;
	wndcls.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndcls.hInstance = g_hInstance;
	wndcls.lpfnWndProc = WinProc;
	wndcls.lpszClassName = g_clsName.c_str();
	wndcls.lpszMenuName = NULL;
	wndcls.style = CS_HREDRAW | CS_VREDRAW;

	//注册窗口类
	if (!RegisterClass(&wndcls))
	{
		MessageBox(NULL, L"窗口注册失败!", L"错误", MB_OK);
		return FALSE;
	}

	//创建窗口
	g_hWnd = CreateWindow(g_clsName.c_str(),
		g_wndTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		g_winWidth, g_winHeight,
		NULL,
		NULL,
		g_hInstance,
		NULL);
	if (!g_hWnd)
	{
		MessageBox(NULL, L"窗口创建失败!", L"错误", MB_OK);
		return FALSE;
	}

	//显示、更新窗口
	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	//Win32初始化完毕
	return TRUE;
}

BOOL InitD3D()
{
	//创建设备及上下文
	D3D_FEATURE_LEVEL features[3] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3 };
	D3D_FEATURE_LEVEL myFeatureLevel;
	HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, 0, 0, features, 3, D3D11_SDK_VERSION, &g_device, &myFeatureLevel, &g_deviceContext);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建d3d11设备失败！", L"错误", MB_OK);
		return FALSE;
	}

	//检测4x采样等级
	UINT msaa4xQuality(0);
	g_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaa4xQuality);

	//创建交换链
	//先描述其属性
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = g_winWidth;					//宽、高
	swapChainDesc.BufferDesc.Height = g_winHeight;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;			//刷新率
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					//恒定参数，按照这样指明即可
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//同上
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						//数据格式，一个为RGBA四元色格式
	swapChainDesc.BufferCount = 1;														//后缓冲区个数，1个足够
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						//Usage，很好理解
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = g_hWnd;							//主窗口句柄
	swapChainDesc.SampleDesc.Count = 4;								//多重采样
	swapChainDesc.SampleDesc.Quality = msaa4xQuality - 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			//交换：绝大多数情况用DISCARD
	swapChainDesc.Windowed = true;									//窗口模式

	//获取IDXGIFactory以创建交换链
	IDXGIDevice *dxgiDevice(NULL);
	g_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
	IDXGIAdapter *dxgiAdapter(NULL);
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter));
	IDXGIFactory *dxgiFactory(NULL);
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));
	hr = dxgiFactory->CreateSwapChain(g_device, &swapChainDesc, &g_swapChain);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建d3d11交换链失败！", L"错误", MB_OK);
		return FALSE;
	}
	dxgiFactory->Release();
	dxgiAdapter->Release();
	dxgiDevice->Release();

	//创建视图
	//需要先获得资源
	ID3D11Texture2D *backBuffer(NULL);
	g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	hr = g_device->CreateRenderTargetView(backBuffer, NULL, &g_renderTargetView);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建RenderTargetView失败！", L"错误", MB_OK);
		return FALSE;
	}
	backBuffer->Release();

	//创建深度缓冲区视图
	//要先创建对应缓冲区
	//创建缓冲区Texture2D，要先给出描述
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = g_winWidth;
	desc.Height = g_winHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 4;
	desc.SampleDesc.Quality = msaa4xQuality - 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	ID3D11Texture2D *depthStencilBuffer(NULL);
	hr = g_device->CreateTexture2D(&desc, NULL, &depthStencilBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建d3d11深度缓冲区失败！", L"错误", MB_OK);
		return FALSE;
	}
	hr = g_device->CreateDepthStencilView(depthStencilBuffer, NULL, &g_depthStencilView);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"创建DepthStencilView失败！", L"错误", MB_OK);
		return FALSE;
	}

	//有了视图要绑定到管线相应阶段
	g_deviceContext->OMSetRenderTargets(1, &g_renderTargetView, g_depthStencilView);
	depthStencilBuffer->Release();

	//最后设置视口
	D3D11_VIEWPORT viewPort = { 0 };
	viewPort.Width = static_cast<FLOAT>(g_winWidth);
	viewPort.Height = static_cast<FLOAT>(g_winHeight);
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	g_deviceContext->RSSetViewports(1, &viewPort);

	return TRUE;
}

void Render()
{
	//渲染一个绿色的窗口
	XMVECTORF32 color = { 0.f, 1.f, 0.f, 1.6f };
	g_deviceContext->ClearRenderTargetView(g_renderTargetView, reinterpret_cast<float*>(&color));
	g_deviceContext->ClearDepthStencilView(g_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	//全部的场景绘制工作在这里面进行.....

	//最后显示
	g_swapChain->Present(0, 0);
}

int Run()
{
	MSG msg = { 0 };
	//主消息循环，也是游戏当中的主循环
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//在这里做该做的事:渲染
			Render();
		}
	}

	Release();

	return msg.wParam;
}

void Release()
{
	g_depthStencilView->Release();
	g_renderTargetView->Release();
	g_swapChain->Release();
	g_deviceContext->Release();
	g_device->Release();
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}