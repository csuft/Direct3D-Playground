#ifndef _WIN_APP_H_
#define _WIN_APP_H_

#include <Windows.h>
#include <string>
#include <D3D11.h>

#include "Timer.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

class WinApp
{
public:
	WinApp(HINSTANCE hInst, std::wstring title = L"D3D11学习程序框架", int width = 640, int height = 480);
	~WinApp();

	//基本内联成员函数
	HINSTANCE	AppInstance()	const				{ return m_hInstance;		}
	HWND		Window()		const				{ return m_hWnd;			}
	int			Width()			const				{ return m_clientWidth;		}
	int			Height()		const				{ return m_clientHeight;	}
	void		SetWindowTitle(std::wstring title)	{ SetWindowText(m_hWnd,title.c_str()); }

	/*
	  在子类中重写这些函数以实现自定义的功能
	  对于个别函数，在重写时，要先调用父类的函数，再添加自定义的功能，
	  比如：Init(),在子类Init()中，需要先调用WinApp::Init()。
	  同样也适合于OnResize()。
	 */
	virtual bool	Init();							//程序初始化
	virtual bool	OnResize();						//当窗口大小改变时调用
	virtual bool	Update(float timeDelt) = 0;		//每帧更新
	virtual bool	Render() = 0;					//渲染

	virtual LRESULT CALLBACK WinProc(HWND,UINT,WPARAM,LPARAM);
	
	int		Run();		//主循环

protected:
	bool	InitWindow();		//初始化Win32窗口
	bool	InitD3D();			//初始化D3D11
	
	void	CalculateFPS();		//计算帧率

protected:
	HINSTANCE	m_hInstance;		//应用程序实例句柄
	HWND		m_hWnd;				//窗口句柄

	int			m_clientWidth;		//窗口大小
	int			m_clientHeight;

	bool		m_isMinimized;		//是否最小化
	bool		m_isMaximized;		//是否最大化
	bool		m_isPaused;			//是否暂停运行
	bool		m_isResizing;		//当鼠标正在改变窗口尺寸时

	ID3D11Device			*m_d3dDevice;				//D3D11设备
	ID3D11DeviceContext		*m_deviceContext;			//设备上下文
	IDXGISwapChain			*m_swapChain;				//交换链
	ID3D11Texture2D			*m_depthStencilBuffer;		//深度/模板缓冲区
	ID3D11RenderTargetView	*m_renderTargetView;		//渲染对象视图
	ID3D11DepthStencilView	*m_depthStencilView;		//深度/模板视图
	
	std::wstring	m_winTitle;			//窗口名称
	Timer			m_timer;			//应用程序定时器

private:
	//避免复制
	WinApp(const WinApp&);
	WinApp& operator = (const WinApp&);
};

#endif	//_WIN_APP_H_