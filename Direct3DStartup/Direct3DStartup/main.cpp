#include "AppUtil.h"
#include "WinApp.h"

//定义一个新类，继承框架主类：WinApp
class Basic : public WinApp
{
public:
	Basic(HINSTANCE hInst, std::wstring title = L"D3D11学习程序框架", int width = 640, int height = 480) :
		WinApp(hInst, title, width, height)
	{}

	//该两个成员函数必需重写
	bool Update(float delta);
	bool Render();

private:
	//在这里定义具体程序所需的相关成员变量
};

//程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow)
{
	//建立程序对象
	Basic app(hInstance);

	//初始化
	if (!app.Init())
		return -1;

	//如果有其他初始化相关的成员函数，可以在这里调用

	//开始运行
	return app.Run();
}

bool Basic::Update(float delta)
{
	//更新

	return true;
}

bool Basic::Render()
{
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Green));

	//渲染场景

	m_swapChain->Present(0, 0);

	return true;
}