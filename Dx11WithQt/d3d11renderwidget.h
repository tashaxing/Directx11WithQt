/*
//D3D渲染的Widget
//没有加各种安全检测的语句，为了代码的简洁
*/

#ifndef D3D11RENDERWIDGET_H
#define D3D11RENDERWIDGET_H

#include <QWidget>

//包含D3D相关的头文件
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>	

//安全释放COM口定义的宏
#define SAFE_RELEASE(p) {if(p){(p)->Release(); p=0;}}

//顶点结构
struct Vertex
{
	Vertex(){}
	Vertex(float x,float y,float z,
		float cr,float cg,float cb,float ca)
		: pos(x,y,z),color(cr,cg,cb,ca){}

	XMFLOAT3 pos;
	XMFLOAT4 color;
};
//顶点layout (加个const才能编译过，不知道为什么)
const D3D11_INPUT_ELEMENT_DESC in_layout[]=
{
	{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
	{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
};
const UINT numElements=ARRAYSIZE(in_layout);
//传到shader的constant buffer
struct VS_ConstantBuffer
{
	XMMATRIX WVP;
};


class D3d11RenderWidget : public QWidget
{
	Q_OBJECT

public:
	D3d11RenderWidget(QWidget *parent);
	~D3d11RenderWidget();
	virtual QPaintEngine *paintEngine() const
	{
		return NULL;
	}

private:
	//重写虚函数
	virtual void resizeEvent(QResizeEvent *event); //当窗口尺寸改变时响应
	virtual void paintEvent(QPaintEvent *event);   //窗口绘制函数，用于render三维场景
	//键盘和鼠标监听
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

private:
	//D3D相关的变量
	ID3D11Device *m_d3dDevice;
	ID3D11DeviceContext *m_d3dDevContext;
	IDXGISwapChain *m_swapChain;
	ID3D11Texture2D *m_depthStencilBuffer;
	ID3D11DepthStencilView *m_depthStencilView;
	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11RasterizerState *m_rasterizeState;

	ID3D11Buffer *m_squareVertexBuffer;
	ID3D11Buffer *m_squareIndiceBuffer;
	ID3D11VertexShader *m_VS;
	ID3D11PixelShader *m_PS;
	ID3D10Blob *m_VS_buffer;
	ID3D10Blob *m_PS_buffer;
	ID3D11InputLayout *m_inputLayout;
	ID3D11Buffer *m_VS_objectBuffer; 


	VS_ConstantBuffer m_VS_ConstantBuffer;

private:
	void InitD3D();     //初始化D3D11
	void ResizeD3D();   //重置D3D
	void InitScene();   //初始化场景
	void UpdateScene(double deltaTime); //更新场景
	void RenderScene(); //渲染场景
	void CleanUp();     //释放资源
};

#endif // D3D11RENDERWIDGET_H
