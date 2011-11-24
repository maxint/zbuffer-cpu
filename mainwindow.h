#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QMenu;
class QAction;
class QActionGroup;
class CAccessObj;
class CScanLine;
class QDoubleSpinBox;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	MainWindow(const QString &fileName);
	~MainWindow();

protected:
	 bool eventFilter(QObject *obj, QEvent *e);
	 QSize sizeHint() const;

private:
	void setupUi();
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void init();
	void initRenderSystem();
	void drawCubeTest();
	void openObjFile(const QString& fileName);
	void renderObj();
	void saveAsImageFile(const QString& fileName);
	void setResolution(int width, int height);
	QString strippedName(const QString& fullFileName);
	void rotateBy(double xAngle, double yAngle, double zAngle);

private slots:
	void open();
	void saveAs();
	void resolution();
	void shadeModel(QAction* act);
	void toggleView(QAction* act);
	void newFrustumOrLight();
	void about();

private:
	QWidget *mImgView;
	QImage mImage;
	QMenu *mFileMenu;
	QMenu *mViewMenu;
	QMenu *mEditMenu;
	QMenu *mHelpMenu;
	QToolBar *mFileToolBar;
	QToolBar *mEditToolBar;
	QToolBar *mCameraLightToolBar;
	QAction *mOpenAct;
	QAction *mQuitAct;
	QAction *mSaveAsImageAct;
	QAction *mResolutionAct;
	QActionGroup *mShadeActGroup;
	QAction *mShadeFlatAct;
	QAction *mShadeSmoothAct;
	QAction *mToggleLightingAct;
	QAction *mRandomColorAct;
	QAction *mPointLightAct;
	QAction *mDirLightAct;
	QActionGroup *mViewActGroup;
	QAction *mViewToolBarAct;
	QAction *mAboutAct;
	QAction *mAboutQtAct;
	QAction *mMouseOpAct;

	QDoubleSpinBox *mSpinEyeX;
	QDoubleSpinBox *mSpinEyeY;
	QDoubleSpinBox *mSpinEyeZ;
	QDoubleSpinBox *mSpinLightX;
	QDoubleSpinBox *mSpinLightY;
	QDoubleSpinBox *mSpinLightZ;

	CAccessObj *mpAccessObj;
	CScanLine *mpRenderSystem;

	// mouse operations
	QPoint lastPos;
	int xRot;
	int yRot;
	int zRot;
};

#endif // MAINWINDOW_H
