#include <QKeyEvent>
#include "d3d11renderwidget.h"

//Xxnamath数学库里面的变量只能写成全局的，不要写为类成员
//二个cube的世界矩阵
XMMATRIX cube1WorldMat;
XMMATRIX cube2WorldMat;
//物体的变换矩阵
XMMATRIX rotation;     //选中矩阵
XMMATRIX scale;        //缩放矩阵
XMMATRIX translation;  //平移矩阵
float rotAngle=0.1f; //旋转角
float rotateSpeed=1.0f; //旋转速度
//摄像机参数
XMMATRIX camView;
XMMATRIX camProjection;

XMVECTOR camPosition;
XMVECTOR camTarget;
XMVECTOR camUp;

//fps相关的全局变量
double countsPerSecond=0.0;
__int64 CounterStart=0;

int frameCount=0;
int fps=0;

__int64 frameTimeOld=0;
double frameTime;

bool isWireFrame=true; //是否线框渲染模式

D3d11RenderWidget::D3d11RenderWidget(QWidget *parent)
	: QWidget(parent),
	m_d3dDevice(0),       //初始化各种指针,也可用NULL或nullptr
	m_d3dDevContext(0),
	m_swapChain(0),
	m_depthStencilBuffer(0),
	m_depthStencilView(0),
	m_renderTargetView(0),
	m_rasterizeState(0),
	m_squareVertexBuffer(0),
	m_squareIndiceBuffer(0),
	m_VS(0),
	m_PS(0),
	m_VS_buffer(0),
	m_PS_buffer(0),
	m_inputLayout(0),
	m_VS_objectBuffer(0)
{
	//设置窗口属性，关键步骤，否则D3D绘制出问题
	setAttribute(Qt::WA_PaintOnScreen,true);
	setAttribute(Qt::WA_NativeWindow,true);
	//在这里初始化D3D和场景
	InitD3D();
	InitScene();

}

D3d11RenderWidget::~D3d11RenderWidget()
{
	//在这里释放接口
	CleanUp();
}

//fps相关函数
void startFPStimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	countsPerSecond=double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	CounterStart=frequencyCount.QuadPart;
}

double getTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart-CounterStart)/countsPerSecond;
}

//获取每帧间隔时间
double getFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount=currentTime.QuadPart-frameTimeOld;
	frameTimeOld=currentTime.QuadPart;

	if(tickCount < 0.0f)
		tickCount=0.0f;

	return float(tickCount)/countsPerSecond;
}

void D3d11RenderWidget::paintEvent(QPaintEvent *event)
{
	//计算fps
	frameCount++;
	if(getTime() > 1.0f)
	{
		fps=frameCount;
		frameCount=0;
		startFPStimer();
		//设置父窗口标题显示fps值
		parentWidget()->setWindowTitle("FPS: "+QString::number(fps));
	}
	frameTime=getFrameTime();
	//更新场景和渲染场景
	UpdateScene(frameTime);
	RenderScene();
	//保证此函数体每一帧都调用
	update();
}

void D3d11RenderWidget::resizeEvent(QResizeEvent *event)
{
	//当窗口尺寸变化时，必须重新设置帧缓存、深度缓存、模板缓存、视口

	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_depthStencilBuffer);

	HRESULT hr;
	m_swapChain->ResizeBuffers(1,width(),height(),DXGI_FORMAT_R8G8B8A8_UNORM,0);
	//Create our BackBuffer
	ID3D11Texture2D* backBuffer;
	m_swapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&backBuffer);

	//Create our Render Target
	hr=m_d3dDevice->CreateRenderTargetView(backBuffer,NULL,&m_renderTargetView);
	SAFE_RELEASE(backBuffer);

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width=width();
	depthStencilDesc.Height=height();
	depthStencilDesc.MipLevels=1;
	depthStencilDesc.ArraySize=1;
	depthStencilDesc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count=1;
	depthStencilDesc.SampleDesc.Quality=0;
	depthStencilDesc.Usage=D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags=D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags=0;
	depthStencilDesc.MiscFlags=0;

	//Create the Depth/Stencil View
	m_d3dDevice->CreateTexture2D(&depthStencilDesc,NULL,&m_depthStencilBuffer);
	m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer,NULL,&m_depthStencilView);

	//Set our Render Target
	m_d3dDevContext->OMSetRenderTargets(1,&m_renderTargetView,m_depthStencilView);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX=0;
	viewport.TopLeftY=0;
	viewport.Width=width();
	viewport.Height=height();
	viewport.MinDepth=0.0f;
	viewport.MaxDepth=1.0f;

	//Set the Viewport
	m_d3dDevContext->RSSetViewports(1,&viewport);
}

//键盘事件可用于场景漫游导航
void D3d11RenderWidget::keyPressEvent(QKeyEvent *event)
{
	//子窗口必须获得焦点才能捕获键盘事件，可以用setFocus和clearFocus获得和取消焦点
	
}

void D3d11RenderWidget::keyReleaseEvent(QKeyEvent *event)
{

}

//鼠标和滚轮事件可用于场景拖拽和视距缩放
void D3d11RenderWidget::mousePressEvent(QMouseEvent *event)
{
	int key=event->button();
	if(key==Qt::LeftButton)
		isWireFrame=true;
	else
		isWireFrame=false;
}

void D3d11RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{

}

void D3d11RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
	
}

void D3d11RenderWidget::wheelEvent(QWheelEvent *event)
{
	int delta=event->delta();
	if(delta>0)
		isWireFrame=true;
	else
		isWireFrame=false;
}

void D3d11RenderWidget::InitD3D()
{
	HRESULT hr;
	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc,sizeof(DXGI_MODE_DESC));

	bufferDesc.Width=width();
	bufferDesc.Height=height();
	bufferDesc.RefreshRate.Numerator=60;
	bufferDesc.RefreshRate.Denominator=1;
	bufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering=DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling=DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc,sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc=bufferDesc;
	swapChainDesc.SampleDesc.Count=1;
	swapChainDesc.SampleDesc.Quality=0;
	swapChainDesc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount=1;
	swapChainDesc.OutputWindow=(HWND)winId();
	swapChainDesc.Windowed=TRUE;
	swapChainDesc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;


	//Create our SwapChain
	hr=D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,NULL,NULL,NULL,
		D3D11_SDK_VERSION,&swapChainDesc,&m_swapChain,&m_d3dDevice,NULL,&m_d3dDevContext);

	//Create our BackBuffer
	ID3D11Texture2D* backBuffer;
	m_swapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&backBuffer);

	//Create our Render Target
	hr=m_d3dDevice->CreateRenderTargetView(backBuffer,NULL,&m_renderTargetView);
	SAFE_RELEASE(backBuffer);

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width=width();
	depthStencilDesc.Height=height();
	depthStencilDesc.MipLevels=1;
	depthStencilDesc.ArraySize=1;
	depthStencilDesc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count=1;
	depthStencilDesc.SampleDesc.Quality=0;
	depthStencilDesc.Usage=D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags=D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags=0;
	depthStencilDesc.MiscFlags=0;

	//Create the Depth/Stencil View
	m_d3dDevice->CreateTexture2D(&depthStencilDesc,NULL,&m_depthStencilBuffer);
	m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer,NULL,&m_depthStencilView);

	//Set our Render Target
	m_d3dDevContext->OMSetRenderTargets(1,&m_renderTargetView,m_depthStencilView);

}

void D3d11RenderWidget::InitScene()
{
	HRESULT hr;
	//Compile Shaders from shader file
	hr=D3DX11CompileFromFile(L"Effects.fx",0,0,"VS","vs_4_0",0,0,0,&m_VS_buffer,0,0);
	hr=D3DX11CompileFromFile(L"Effects.fx",0,0,"PS","ps_4_0",0,0,0,&m_PS_buffer,0,0);

	//Create the Shader Objects
	hr=m_d3dDevice->CreateVertexShader(m_VS_buffer->GetBufferPointer(),m_VS_buffer->GetBufferSize(),NULL,&m_VS);
	hr=m_d3dDevice->CreatePixelShader(m_PS_buffer->GetBufferPointer(),m_PS_buffer->GetBufferSize(),NULL,&m_PS);

	//Set Vertex and Pixel Shaders
	m_d3dDevContext->VSSetShader(m_VS,0,0);
	m_d3dDevContext->PSSetShader(m_PS,0,0);

	//Create the vertex (在实际的开发中，不同的物体的顶点缓存和索引缓存都要写到对应的类中)
	Vertex v[]=
	{
		Vertex(-1.0f,-1.0f,-1.0f,1.0f,0.0f,0.0f,1.0f),
		Vertex(-1.0f,+1.0f,-1.0f,0.0f,1.0f,0.0f,1.0f),
		Vertex(+1.0f,+1.0f,-1.0f,0.0f,0.0f,1.0f,1.0f),
		Vertex(+1.0f,-1.0f,-1.0f,1.0f,1.0f,0.0f,1.0f),
		Vertex(-1.0f,-1.0f,+1.0f,0.0f,1.0f,1.0f,1.0f),
		Vertex(-1.0f,+1.0f,+1.0f,1.0f,1.0f,1.0f,1.0f),
		Vertex(+1.0f,+1.0f,+1.0f,1.0f,0.0f,1.0f,1.0f),
		Vertex(+1.0f,-1.0f,+1.0f,1.0f,0.0f,0.0f,1.0f),
	};
	//create index 
	DWORD indices[]={
		// front face
		0,1,2,
		0,2,3,

		// back face
		4,6,5,
		4,7,6,

		// left face
		4,5,1,
		4,1,0,

		// right face
		3,2,6,
		3,6,7,

		// top face
		1,5,6,
		1,6,2,

		// bottom face
		4,0,3,
		4,3,7
	};

	//index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc,sizeof(indexBufferDesc));

	indexBufferDesc.Usage=D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth=sizeof(DWORD)*12*3;
	indexBufferDesc.BindFlags=D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags=0;
	indexBufferDesc.MiscFlags=0;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem=indices;
	m_d3dDevice->CreateBuffer(&indexBufferDesc,&indexData,&m_squareIndiceBuffer);

	m_d3dDevContext->IASetIndexBuffer(m_squareIndiceBuffer,DXGI_FORMAT_R32_UINT,0);

	//vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc,sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage=D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth=sizeof(Vertex)*8;
	vertexBufferDesc.BindFlags=D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags=0;
	vertexBufferDesc.MiscFlags=0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData,sizeof(vertexBufferData));
	vertexBufferData.pSysMem=v;
	hr=m_d3dDevice->CreateBuffer(&vertexBufferDesc,&vertexBufferData,&m_squareVertexBuffer);

	//Set the vertex buffer
	UINT stride=sizeof(Vertex);
	UINT offset=0;
	m_d3dDevContext->IASetVertexBuffers(0,1,&m_squareVertexBuffer,&stride,&offset);
	//Create the Input Layout
	hr=m_d3dDevice->CreateInputLayout(in_layout,numElements,m_VS_buffer->GetBufferPointer(),
		m_VS_buffer->GetBufferSize(),&m_inputLayout);
	//Set the Input Layout
	m_d3dDevContext->IASetInputLayout(m_inputLayout);
	//Set Primitive Topology
	m_d3dDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX=0;
	viewport.TopLeftY=0;
	viewport.Width=width();
	viewport.Height=height();
	viewport.MinDepth=0.0f;
	viewport.MaxDepth=1.0f;

	//Set the Viewport
	m_d3dDevContext->RSSetViewports(1,&viewport);

	//Create the buffer to send to the cbuffer in effect file
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd,sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage=D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth=sizeof(VS_ConstantBuffer);
	cbbd.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags=0;
	cbbd.MiscFlags=0;

	hr=m_d3dDevice->CreateBuffer(&cbbd,NULL,&m_VS_objectBuffer);

	//Camera information
	camPosition=XMVectorSet(0.0f,3.0f,-8.0f,0.0f);
	camTarget=XMVectorSet(0.0f,0.0f,0.0f,0.0f);
	camUp=XMVectorSet(0.0f,1.0f,0.0f,0.0f);

	//Set the View matrix
	camView=XMMatrixLookAtLH(camPosition,camTarget,camUp);

	//Set the Projection matrix
	camProjection=XMMatrixPerspectiveFovLH(0.4f*3.14f,(float)width()/height(),1.0f,1000.0f);

	//set the rasterization state
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc,sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode=D3D11_FILL_WIREFRAME; //绘制方式是线框
	wfdesc.CullMode=D3D11_CULL_NONE;
	hr=m_d3dDevice->CreateRasterizerState(&wfdesc,&m_rasterizeState);

	m_d3dDevContext->RSSetState(m_rasterizeState);

}

void D3d11RenderWidget::UpdateScene(double deltaTime)
{
	//使物体旋转速度不依赖于fps
	rotAngle+=rotateSpeed*deltaTime;
	if(rotAngle>=6.28f)
		rotAngle=0.0f;

	//Reset cube1World
	cube1WorldMat=XMMatrixIdentity();

	//Define cube1's world space matrix
	XMVECTOR rotaxis=XMVectorSet(0.0f,1.0f,0.0f,0.0f);
	rotation=XMMatrixRotationAxis(rotaxis,rotAngle);
	translation=XMMatrixTranslation(0.0f,0.0f,4.0f);

	//Set cube1's world space using the transformations
	cube1WorldMat=translation * rotation;  //矩阵相乘的顺序会影响实际的旋转效果
	
	//Reset cube2World
	cube2WorldMat=XMMatrixIdentity();

	//Define cube2's world space matrix
	rotation=XMMatrixRotationAxis(rotaxis,-rotAngle);
	scale=XMMatrixScaling(1.3f,1.3f,1.3f);

	//Set cube2's world space matrix
	cube2WorldMat=rotation * scale;

	//set the rasterization state
	HRESULT hr;
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc,sizeof(D3D11_RASTERIZER_DESC));
	if(isWireFrame)
		wfdesc.FillMode=D3D11_FILL_WIREFRAME; //绘制方式切换
	else
		wfdesc.FillMode=D3D11_FILL_SOLID;
	wfdesc.CullMode=D3D11_CULL_NONE;
	hr=m_d3dDevice->CreateRasterizerState(&wfdesc,&m_rasterizeState);

	m_d3dDevContext->RSSetState(m_rasterizeState);
}

void D3d11RenderWidget::RenderScene()
{
	//Clear our backbuffer
	float bgColor[4]={(0.0f,0.0f,0.0f,0.0f)};
	m_d3dDevContext->ClearRenderTargetView(m_renderTargetView,bgColor);

	//Refresh the Depth/Stencil view
	m_d3dDevContext->ClearDepthStencilView(m_depthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);

	//1
	//Set the WVP matrix and send it to the constant buffer in effect file
	XMMATRIX WVP;
	WVP=cube1WorldMat * camView * camProjection;
	m_VS_ConstantBuffer.WVP=XMMatrixTranspose(WVP);
	m_d3dDevContext->UpdateSubresource(m_VS_objectBuffer,0,NULL,&m_VS_ConstantBuffer,0,0);
	m_d3dDevContext->VSSetConstantBuffers(0,1,&m_VS_objectBuffer);

	//Draw the first cube
	m_d3dDevContext->DrawIndexed(36,0,0);

	//2
	WVP=cube2WorldMat * camView * camProjection;
	m_VS_ConstantBuffer.WVP=XMMatrixTranspose(WVP);
	m_d3dDevContext->UpdateSubresource(m_VS_objectBuffer,0,NULL,&m_VS_ConstantBuffer,0,0);
	m_d3dDevContext->VSSetConstantBuffers(0,1,&m_VS_objectBuffer);

	//Draw the second cube
	m_d3dDevContext->DrawIndexed(36,0,0);

	//Present the backbuffer to the screen
	m_swapChain->Present(0,0);
}

void D3d11RenderWidget::CleanUp()
{
	//释放COM和一些资源
	SAFE_RELEASE(m_d3dDevice);
	SAFE_RELEASE(m_d3dDevContext);
	SAFE_RELEASE(m_swapChain);
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_rasterizeState);
	SAFE_RELEASE(m_squareVertexBuffer);
	SAFE_RELEASE(m_squareIndiceBuffer);
	SAFE_RELEASE(m_VS);
	SAFE_RELEASE(m_PS);
	SAFE_RELEASE(m_VS_buffer);
	SAFE_RELEASE(m_PS_buffer);
	SAFE_RELEASE(m_inputLayout);
	SAFE_RELEASE(m_VS_objectBuffer);
}

