#include "mainwindow.h"
#include "AccessObj.h"
#include "ScanLine.h"
#include <QtGui>
#include <ctime>

const QString WIDGET_NAME = "ImageView";
const QString WINDOW_TITLE = "CG ZBuffer";
const Vec3d EYE_POS(3, 4, 5);
const Vec4d LIGHT_POS(2, 3, 4, 1);

MainWindow::MainWindow()
: mImage(800, 600, QImage::Format_ARGB32)
, mpAccessObj(0)
, mpRenderSystem(0)
{
	init();
}

MainWindow::MainWindow(const QString &fileName)
: mImage(800, 600, QImage::Format_ARGB32)
, mpAccessObj(0)
, mpRenderSystem(0)
{
	init();
	if (QFile::exists(fileName))
	{
		openObjFile(fileName);
		statusBar()->showMessage(tr("File loaded"), 3000);
	}
	else
	{
		statusBar()->showMessage(tr("File load fail"));
	}
}

MainWindow::~MainWindow()
{
	SAFE_DELETE(mpAccessObj);
	SAFE_DELETE(mpRenderSystem);
}

void MainWindow::init()
{
	srand((unsigned int)time(0));
	xRot = yRot = zRot = 30;
	
	initRenderSystem();

	setupUi();
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	
	renderObj();
	//rotateBy(0, 0, 0);
}

void MainWindow::initRenderSystem()
{
	mpAccessObj = new CAccessObj;
	mImage.fill(qRgb(200, 200, 200));
	mpRenderSystem = new CScanLine(mImage.width(), mImage.height(), &mImage);

	// set camera
	Vec3d at(0, 0, 0);
	Vec3d up(0, 1, 0);
	mpRenderSystem->lookAt(EYE_POS, at, up);
	mpRenderSystem->perspective(3.14/6, mImage.width()*1.0/mImage.height(), 1, 100);
	//mpRenderSystem.ortho(-2, 2, -2, 2, 1, 100);

	// set light
	mpRenderSystem->mLight.type = SL_LIGHT_POINT;
	//lit.type = SL_LIGHT_DIRECTIONAL;
	mpRenderSystem->mLight.direction = Vec3d(-1.0, -1.0, -0.5);
	//lit.position = Vec4d(3, 4, 5, 1);
	mpRenderSystem->mLight.position = LIGHT_POS;

	mpRenderSystem->mMaterial.specular = Color4d(1.0, 1.0, 1.0, 1.0);
	mpRenderSystem->mMaterial.shiness = 60;
}

void MainWindow::setupUi()
{
	mImgView = new QWidget;
	mImgView->setObjectName(WIDGET_NAME);
	mImgView->setFixedSize(mImage.size());
	this->setCentralWidget(mImgView);
	mImgView->installEventFilter(this);
	this->layout()->setSizeConstraint(QLayout::SetFixedSize);

	setWindowTitle(WINDOW_TITLE);
}

QSize MainWindow::sizeHint() const
{
	return mImage.size();
}

void MainWindow::createActions()
{
	mOpenAct = new QAction(QIcon(":/images/open.png"), tr("&Open Obj..."), this);
	mOpenAct->setShortcut(QKeySequence::Open);
	mOpenAct->setStatusTip(tr("Open an obj model file"));
	connect(mOpenAct, SIGNAL(triggered()), this, SLOT(open()));

	mSaveAsImageAct = new QAction(QIcon(":/images/save.png"), tr("&Save As Image..."), this);
	mSaveAsImageAct->setShortcut(QKeySequence::Save);
	mSaveAsImageAct->setStatusTip(tr("Save the result as an image"));
	connect(mSaveAsImageAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	mQuitAct = new QAction(tr("&Quit"), this);
	mQuitAct->setShortcut(tr("Ctrl+Q"));
	mQuitAct->setStatusTip(tr("Exit the application"));
	connect(mQuitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

	mResolutionAct = new QAction(tr("Set &Canvas..."), this);
	mResolutionAct->setShortcut(tr("Ctrl+C"));
	mResolutionAct->setStatusTip(tr("Set the resolution"));
	connect(mResolutionAct, SIGNAL(triggered()), this, SLOT(resolution()));

	// shading actions
	mShadeFlatAct = new QAction(tr("&Flat Shading"), this);
	mShadeFlatAct->setStatusTip(tr("Per vertex shading model (e.g. Gouraud Shading )"));
	mShadeFlatAct->setCheckable(true);
	mShadeFlatAct->setChecked(mpRenderSystem->renderState().isFlatShading());

	mShadeSmoothAct = new QAction(tr("&Smooth Shading"), this);
	mShadeSmoothAct->setStatusTip(tr("Per pixel shading model (e.g. Phong Shading )"));
	mShadeSmoothAct->setCheckable(true);
	mShadeSmoothAct->setChecked(mpRenderSystem->renderState().isSmoothShading() );

	// lighting actions
	mToggleLightingAct = new QAction(tr("&Lighting"), this);
	mToggleLightingAct->setStatusTip(tr("Enable/Disable lighting"));
	mToggleLightingAct->setCheckable(true);
	mToggleLightingAct->setChecked(mpRenderSystem->renderState().isLighting());

	mRandomColorAct = new QAction(tr("&Random Color"), this);
	mRandomColorAct->setStatusTip(tr("Enable/Disable generating random color"));
	mRandomColorAct->setCheckable(true);
	mRandomColorAct->setChecked(true);

	mPointLightAct = new QAction(tr("&Point Light"), this);
	mPointLightAct->setStatusTip(tr("Choose point light"));
	mPointLightAct->setCheckable(true);
	mPointLightAct->setChecked(mpRenderSystem->mLight.type == SL_LIGHT_POINT);

	mDirLightAct = new QAction(tr("&Dir Light"), this);
	mDirLightAct->setStatusTip(tr("Choose directional light"));
	mDirLightAct->setCheckable(true);
	mDirLightAct->setChecked(mpRenderSystem->mLight.type == SL_LIGHT_DIRECTIONAL);

	mShadeActGroup = new QActionGroup(this);
	mShadeActGroup->setExclusive(false);
	mShadeActGroup->addAction(mShadeFlatAct);
	mShadeActGroup->addAction(mShadeSmoothAct);
	mShadeActGroup->addAction(mToggleLightingAct);
	mShadeActGroup->addAction(mRandomColorAct);
	mShadeActGroup->addAction(mPointLightAct);
	mShadeActGroup->addAction(mDirLightAct);
	connect(mShadeActGroup, SIGNAL(triggered(QAction*)), this, SLOT(shadeModel(QAction*)));

	// view menu
	mViewToolBarAct = new QAction(tr("&Toolbar"), this);
	mViewToolBarAct->setStatusTip(tr("Toggle toolbar"));
	mViewToolBarAct->setCheckable(true);
	mViewToolBarAct->setChecked(true);
	mViewToolBarAct->setShortcut(tr("Ctrl+T"));

	mViewActGroup = new QActionGroup(this);
	mViewActGroup->addAction(mViewToolBarAct);
	mViewActGroup->setExclusive(false);
	connect(mViewActGroup, SIGNAL(triggered(QAction*)), this, SLOT(toggleView(QAction*)));

	// help menu
	mAboutAct = new QAction(tr("&About"), this);
	mAboutAct->setStatusTip(tr("Show the application's About box"));
	connect(mAboutAct, SIGNAL(triggered()), this, SLOT(about()));
	mAboutQtAct = new QAction(tr("About &Qt"), this);
	mAboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(mAboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// mouse operation
	mMouseOpAct = new QAction(tr("&Mouse Op"), this);
	mMouseOpAct->setStatusTip(tr("Change camera position with mouse"));
	mMouseOpAct->setCheckable(true);
	mMouseOpAct->setChecked(true);
	mMouseOpAct->setShortcut(tr("Ctrl+M"));
}

void MainWindow::shadeModel(QAction* act)
{
	if (act == mShadeFlatAct)
	{
		mpRenderSystem->setRenderState(SL_SHADE_FLAT, true);
		mShadeFlatAct->setChecked(true);
		mShadeSmoothAct->setChecked(false);
		statusBar()->showMessage(tr("Flat shading finished"), 3000);
	}
	else if (act == mShadeSmoothAct)
	{
		mpRenderSystem->setRenderState(SL_SHADE_SMOOTH, true);
		mShadeSmoothAct->setChecked(true);
		mShadeFlatAct->setChecked(false);
		statusBar()->showMessage(tr("Smooth shading finished"), 3000);
	}
	else if (act == mToggleLightingAct)
	{
		mpRenderSystem->setRenderState(SL_LIGHTING, mToggleLightingAct->isChecked());
		statusBar()->showMessage(mToggleLightingAct->isChecked() ? tr("Enabled lighting") :
			tr("Disabled lighting"), 3000);
	}
	else if (act == mRandomColorAct)
	{
		if (act->isChecked())
			statusBar()->showMessage(tr("Random color generated"), 3000);
		else
			statusBar()->showMessage(tr("Disable random color generation"), 3000);
	}
	else if (act == mPointLightAct)
	{
		act->setChecked(true);
		mDirLightAct->setChecked(false);
		mpRenderSystem->mLight.type = SL_LIGHT_POINT;
		statusBar()->showMessage(tr("Point lighting"), 3000);
	}
	else if (act == mDirLightAct)
	{
		act->setChecked(true);
		mPointLightAct->setChecked(false);
		mpRenderSystem->mLight.type = SL_LIGHT_DIRECTIONAL;
		statusBar()->showMessage(tr("Directional lighting"), 3000);
	}
	else
	{
		statusBar()->showMessage(tr("Invalid operation"), 3000);
		return;
	}

	renderObj();
}

void MainWindow::toggleView(QAction *act)
{
	if (act == mViewToolBarAct)
	{
		bool bShow = act->isChecked();
		mFileToolBar->setVisible(bShow);
		mEditToolBar->setVisible(bShow);
	}
}

void MainWindow::createMenus()
{
	mFileMenu = menuBar()->addMenu(tr("&File"));
	mFileMenu->addAction(mOpenAct);
	mFileMenu->addAction(mSaveAsImageAct);
	mFileMenu->addSeparator();
	mFileMenu->addAction(mQuitAct);
	
	menuBar()->addSeparator();

	mViewMenu = menuBar()->addMenu(tr("&View"));
	mViewMenu->addAction(mViewToolBarAct);

	menuBar()->addSeparator();

	mEditMenu = menuBar()->addMenu(tr("&Edit"));
	mEditMenu->addAction(mResolutionAct);
	mEditMenu->addAction(mRandomColorAct);
	mEditMenu->addSeparator();
	mEditMenu->addAction(mToggleLightingAct);
	mEditMenu->addAction(mPointLightAct);
	mEditMenu->addAction(mDirLightAct);
	mEditMenu->addSeparator();
	mEditMenu->addAction(mShadeFlatAct);
	mEditMenu->addAction(mShadeSmoothAct);

	menuBar()->addSeparator();
	mHelpMenu = menuBar()->addMenu(tr("&Help"));
	mHelpMenu->addAction(mAboutAct);
	mHelpMenu->addAction(mAboutQtAct);
}

void MainWindow::createToolBars()
{
	mFileToolBar = addToolBar(tr("File"));
	mFileToolBar->addAction(mOpenAct);
	mFileToolBar->addAction(mSaveAsImageAct);

	mEditToolBar = addToolBar(tr("Edit"));
	mEditToolBar->addAction(mResolutionAct);
	mEditToolBar->addAction(mRandomColorAct);
	mEditToolBar->addSeparator();
	mEditToolBar->addAction(mToggleLightingAct);
	mEditToolBar->addAction(mPointLightAct);
	mEditToolBar->addAction(mDirLightAct);
	mEditToolBar->addSeparator();
	mEditToolBar->addAction(mShadeFlatAct);
	mEditToolBar->addAction(mShadeSmoothAct);

	// camera and light toolbar
	mSpinEyeX = new QDoubleSpinBox;
	mSpinEyeY = new QDoubleSpinBox;
	mSpinEyeZ = new QDoubleSpinBox;
	mSpinLightX = new QDoubleSpinBox;
	mSpinLightY = new QDoubleSpinBox;
	mSpinLightZ = new QDoubleSpinBox;
	mSpinEyeX->setMinimum(-100.0);
	mSpinEyeY->setMinimum(-100.0);
	mSpinEyeZ->setMinimum(-100.0);
	mSpinLightX->setMinimum(-100.0);
	mSpinLightY->setMinimum(-100.0);
	mSpinLightX->setMinimum(-100.0);
	mSpinEyeX->setSingleStep(0.2);
	mSpinEyeY->setSingleStep(0.2);
	mSpinEyeZ->setSingleStep(0.2);
	mSpinLightX->setSingleStep(0.2);
	mSpinLightY->setSingleStep(0.2);
	mSpinLightZ->setSingleStep(0.2);
	mSpinEyeX->setValue(EYE_POS[0]);
	mSpinEyeY->setValue(EYE_POS[1]);
	mSpinEyeZ->setValue(EYE_POS[2]);
	mSpinLightX->setValue(LIGHT_POS[0]);
	mSpinLightY->setValue(LIGHT_POS[1]);
	mSpinLightZ->setValue(LIGHT_POS[2]);

	mCameraLightToolBar = addToolBar(tr("CameraAndLight"));
	addToolBar(Qt::BottomToolBarArea, mCameraLightToolBar);
	mCameraLightToolBar->setToolTip(tr("Set position of camera or light"));

	mCameraLightToolBar->addWidget(new QLabel(tr("Camera ")));
	mCameraLightToolBar->addWidget(new QLabel(tr("x:")));
	mCameraLightToolBar->addWidget(mSpinEyeX);
	mCameraLightToolBar->addWidget(new QLabel(tr("y:")));
	mCameraLightToolBar->addWidget(mSpinEyeY);
	mCameraLightToolBar->addWidget(new QLabel(tr("z:")));
	mCameraLightToolBar->addWidget(mSpinEyeZ);
	mCameraLightToolBar->addSeparator();
	mCameraLightToolBar->addWidget(new QLabel(tr("Light ")));
	mCameraLightToolBar->addWidget(new QLabel(tr("x:")));
	mCameraLightToolBar->addWidget(mSpinLightX);
	mCameraLightToolBar->addWidget(new QLabel(tr("y:")));
	mCameraLightToolBar->addWidget(mSpinLightY);
	mCameraLightToolBar->addWidget(new QLabel(tr("z:")));
	mCameraLightToolBar->addWidget(mSpinLightZ);

	connect(mSpinEyeX, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	connect(mSpinEyeY, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	connect(mSpinEyeZ, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	connect(mSpinLightX, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	connect(mSpinLightY, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	connect(mSpinLightZ, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::open()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Obj Model"), "objs", tr("Obj Fils (*.obj)"));
	if (!fileName.isEmpty())
	{
		openObjFile(fileName);
	}
}

void MainWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, 
		tr("Save Result"), "res.png", tr("Images (*.png *.jpg *.bmp)"));
	
	saveAsImageFile(fileName);
}

void MainWindow::resolution()
{
	bool ok;
	QString strSize = QInputDialog::getText(this, tr("Set Resolution"),
		tr("New Resolution (e.g. 800 600) : "), QLineEdit::Normal,
		tr("800 600"), &ok);

	int w = strSize.section(' ', 0, 0).toInt();
	int h = strSize.section(' ', 1, 1).toInt();
	if (ok && w>0 && h>0)
	{
		setResolution(w, h);
		mImgView->setFixedSize(mImage.size());
		statusBar()->showMessage(tr("New resolution (%1, %2) is applied").arg(w).arg(h), 3000);
	}
	else
	{
		statusBar()->showMessage(tr("Invalid resolution"), 3000);
	}
}

void MainWindow::openObjFile(const QString& fileName)
{
	if (mpAccessObj->LoadOBJ(fileName.toStdString().c_str()))
	{
		mpAccessObj->UnifiedModel();

		renderObj();

		setWindowTitle( tr("%1 - %2")
			.arg(strippedName(fileName))
			.arg(WINDOW_TITLE) );
	}
	else
	{
		QMessageBox::warning(this, tr("OBJ File Invalid"), 
			tr("File %1 is not a valid obj model file.").arg(fileName));
	}
}

void MainWindow::renderObj()
{
	clock_t tt = clock();

	mpRenderSystem->clear(SL_COLOR_BUFFER | SL_DEPTH_BUFFER, Color4u(200, 200, 200, 255), 1.0);
	if (mpAccessObj->m_pModel)
	{
		// clear frame buffer

		mpRenderSystem->begin(SL_TRIANGLES);
		CPoint3D *vpVertices = mpAccessObj->m_pModel->vpVertices;
		CPoint3D *vpNormals = mpAccessObj->m_pModel->vpNormals;
		if (!mRandomColorAct->isChecked())
			mpRenderSystem->color3i(255, 255, 255);
		for (unsigned int i=0; i<mpAccessObj->m_pModel->nTriangles; ++i)
		{
			COBJtriangle &pTri = mpAccessObj->m_pModel->pTriangles[i];
			if (mRandomColorAct->isChecked())
			{
				byte iR = rand() % 256;
				byte iG = rand() % 256;
				byte iB = rand() % 256;
				mpRenderSystem->color3i(iR, iG, iB);
			}
			if (vpNormals) mpRenderSystem->normal3fv(vpNormals[pTri.nindices[0]]);
			mpRenderSystem->vertex3fv(vpVertices[pTri.vindices[0]]);
			if (vpNormals) mpRenderSystem->normal3fv(vpNormals[pTri.nindices[1]]);
			mpRenderSystem->vertex3fv(vpVertices[pTri.vindices[1]]);
			if (vpNormals) mpRenderSystem->normal3fv(vpNormals[pTri.nindices[2]]);
			mpRenderSystem->vertex3fv(vpVertices[pTri.vindices[2]]);
		}
		mpRenderSystem->end();
	}
	else
	{
		drawCubeTest();
	}

	statusBar()->showMessage(tr("Rendering finished in %1 ms. Triangles: %2")
		.arg(clock()-tt)
		.arg(mpAccessObj->m_pModel ? mpAccessObj->m_pModel->nTriangles : 12), 5000);

	mImgView->update();
}

void MainWindow::saveAsImageFile(const QString& fileName)
{
	mImage.save(fileName);
}

void MainWindow::setResolution(int width, int height)
{
	mImage = mImage.scaled(width, height);
	mpRenderSystem->setRenderTarget(mImage.width(), mImage.height(), &mImage);
	mpRenderSystem->perspective(3.14/6, mImage.width()*1.0/mImage.height(), 1, 100);
	mpRenderSystem->clear(SL_DEPTH_BUFFER | SL_COLOR_BUFFER, Color4u(200, 200, 200, 255));

	renderObj();
}

void MainWindow::newFrustumOrLight()
{
	// set camera
	Vec3d eye(mSpinEyeX->value(), mSpinEyeY->value(), mSpinEyeZ->value());
	Vec3d at(0, 0, 0);
	Vec3d up(0, 1, 0);
	mpRenderSystem->lookAt(eye, at, up);
	mpRenderSystem->perspective(3.14/6, mImage.width()*1.0/mImage.height(), 1, 100);

	mpRenderSystem->mLight.position = 
		Vec4d(mSpinLightX->value(), mSpinLightY->value(), mSpinLightZ->value(), 1);

	renderObj();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About CG Z-Buffer"),
		tr("<p>A <b>Z-Buffer</b> project writed by maxint, in Janary, 2010.</p>"
		"<p>Email: <a href='mailto:lnychina@gmail.com'>lnychina@gmail.com</a></p>"
		"<p>Blog: <a href='http://hi.baidu.com/maxint'>http://hi.baidu.com/maxint</a></p>"));
}

void MainWindow::rotateBy(double xAngle, double yAngle, double zAngle)
{
	static Mat22d matRot;
	static Vec3d eyePos;

	xRot += xAngle;
	yRot += yAngle;
	zRot += zAngle;
	xRot %= 360;
	yRot %= 360;
	zRot %= 360;

	eyePos[0] = mSpinEyeX->value();
	eyePos[1] = mSpinEyeY->value();
	eyePos[2] = mSpinEyeZ->value();
	double len = eyePos.length();
	eyePos[0] = 0;
	eyePos[1] = 0;
	eyePos[2] = len;

	matRot.rotate(-zRot);
	matRot.map(eyePos[0], eyePos[1], &eyePos[0], &eyePos[1]);
	matRot.rotate(yRot);
	matRot.map(eyePos[0], eyePos[2], &eyePos[0], &eyePos[2]);
	matRot.rotate(-xRot);
	matRot.map(eyePos[1], eyePos[2], &eyePos[1], &eyePos[2]);

	mSpinEyeX->setValue(eyePos[0]);
	mSpinEyeY->setValue(eyePos[1]);
	mSpinEyeZ->setValue(eyePos[2]);

	newFrustumOrLight();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
	if (obj->isWidgetType() &&
		obj->objectName() == WIDGET_NAME)
	{
		QWidget *wid = static_cast<QWidget*>(obj);
		if (e->type() == QEvent::Paint)
		{
			QPainter painter(wid);
			painter.drawImage(QPoint(0,0), mImage);
			e->accept();
			return true;
		}
		//else if (mMouseOpAct->isChecked() && e->type() == QEvent::MouseMove)
		//{
		//	QMouseEvent *mouseE = static_cast<QMouseEvent*>(e);

		//	double dx = mouseE->x() - lastPos.x();
		//	double dy = mouseE->y() - lastPos.y();
		//	dx = std::sqrt(dx);
		//	dy = std::sqrt(dy);

		//	if (mouseE->buttons() & Qt::LeftButton) {
		//		rotateBy(dy, dx, 0);
		//	} else if (mouseE->buttons() & Qt::RightButton) {
		//		rotateBy(dy, 0, dx);
		//	}
		//	lastPos = mouseE->pos();
		//	mouseE->accept();
		//	return true;
		//}
		//else if (mMouseOpAct->isChecked() && e->type() == QEvent::MouseButtonPress)
		//{
		//	QMouseEvent *mouseE = static_cast<QMouseEvent*>(e);
		//	lastPos = mouseE->pos();
		//	mouseE->accept();
		//	return true;
		//}
	}
	
	return QMainWindow::eventFilter(obj, e);
}

QString MainWindow::strippedName(const QString& fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::drawCubeTest()
{
	// cube ///////////////////////////////////////////////////////////////////////
	//    v6----- v5
	//   /|      /|
	//  v1------v0|
	//  | |     | |
	//  | |v7---|-|v4
	//  |/      |/
	//  v2------v3

	mpRenderSystem->begin(SL_QUADS);
	// face v0-v1-v2-v3
	mpRenderSystem->normal3d(0,0,1);
	mpRenderSystem->color3f(1,1,1);
	mpRenderSystem->vertex3d(1,1,1);
	mpRenderSystem->color3f(1,1,0);
	mpRenderSystem->vertex3d(-1,1,1);
	mpRenderSystem->color3f(1,0,0);
	mpRenderSystem->vertex3d(-1,-1,1);
	mpRenderSystem->color3f(1,0,1);
	mpRenderSystem->vertex3d(1,-1,1);

	// face v0-v3-v4-v6
	mpRenderSystem->normal3d(1,0,0);
	mpRenderSystem->color3f(1,1,1);
	mpRenderSystem->vertex3d(1,1,1);
	mpRenderSystem->color3f(1,0,1);
	mpRenderSystem->vertex3d(1,-1,1);
	mpRenderSystem->color3f(0,0,1);
	mpRenderSystem->vertex3d(1,-1,-1);
	mpRenderSystem->color3f(0,1,1);
	mpRenderSystem->vertex3d(1,1,-1);

	// face v0-v5-v6-v1
	mpRenderSystem->normal3d(0,1,0);
	mpRenderSystem->color3f(1,1,1);
	mpRenderSystem->vertex3d(1,1,1);
	mpRenderSystem->color3f(0,1,1);
	mpRenderSystem->vertex3d(1,1,-1);
	mpRenderSystem->color3f(0,1,0);
	mpRenderSystem->vertex3d(-1,1,-1);
	mpRenderSystem->color3f(1,1,0);
	mpRenderSystem->vertex3d(-1,1,1);

	// face  v1-v6-v7-v2
	mpRenderSystem->normal3d(-1,0,0);
	mpRenderSystem->color3f(1,1,0);
	mpRenderSystem->vertex3d(-1,1,1);
	mpRenderSystem->color3f(0,1,0);
	mpRenderSystem->vertex3d(-1,1,-1);
	mpRenderSystem->color3f(0,0,0);
	mpRenderSystem->vertex3d(-1,-1,-1);
	mpRenderSystem->color3f(1,0,0);
	mpRenderSystem->vertex3d(-1,-1,1);

	// face v7-v4-v3-v2
	mpRenderSystem->normal3d(0,-1,0);
	mpRenderSystem->color3f(0,0,0);
	mpRenderSystem->vertex3d(-1,-1,-1);
	mpRenderSystem->color3f(0,0,1);
	mpRenderSystem->vertex3d(1,-1,-1);
	mpRenderSystem->color3f(1,0,1);
	mpRenderSystem->vertex3d(1,-1,1);
	mpRenderSystem->color3f(1,0,0);
	mpRenderSystem->vertex3d(-1,-1,1);

	// face v4-v7-v6-v5
	mpRenderSystem->normal3d(0,0,-1);
	mpRenderSystem->color3f(0,0,1);
	mpRenderSystem->vertex3d(1,-1,-1);
	mpRenderSystem->color3f(0,0,0);
	mpRenderSystem->vertex3d(-1,-1,-1);
	mpRenderSystem->color3f(0,1,0);
	mpRenderSystem->vertex3d(-1,1,-1);
	mpRenderSystem->color3f(0,1,1);
	mpRenderSystem->vertex3d(1,1,-1);
	mpRenderSystem->end();
}