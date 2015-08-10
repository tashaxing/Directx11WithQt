#include "mainwidget.h"
#include "d3d11renderwidget.h"

MainWidget::MainWidget(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//Ìí¼ÓD3D×Ó´°¿Ú
	D3d11RenderWidget *widget=new D3d11RenderWidget(this);
	setCentralWidget(widget);
}

MainWidget::~MainWidget()
{

}
