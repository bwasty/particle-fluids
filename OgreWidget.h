// from http://www.ogre3d.org/forums/viewtopic.php?p=367010&sid=5f3feeed878d9121b62dec0d82e7d19e#p367010 (orginally from Ogitor)

#ifndef OGREWIDGET_H
#define OGREWIDGET_H

#include <QtGui>
#include <Ogre.h>

namespace NxOgre {
	class World;
	class TimeController;
	class Scene;
	class VisualDebugger;
}

class OGRE3DRenderSystem;
class OGRE3DRenderable;

class OgreWidget : public QWidget, public Ogre::FrameListener
{
    Q_OBJECT

public:
    OgreWidget(QWidget *parent = 0);
    ~OgreWidget();

    // Override QWidget::paintEngine to return NULL
    QPaintEngine* paintEngine() const; // Turn off QTs paint engine for the Ogre widget.

	// FrameListener
	//bool frameStarted(const Ogre::FrameEvent &evt);
	bool frameRenderingQueued(const Ogre::FrameEvent &evt);
	bool frameEnded(const Ogre::FrameEvent &evt);

public slots:
    void setBackgroundColor(QColor c);
    //void setCameraPosition(const Ogre::Vector3 &pos);

signals:
    //void cameraPositionChanged(const Ogre::Vector3 &pos);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void moveEvent(QMoveEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private:
    void initOgreSystem();
    void setupResources();
	void setupNxOgre();
    void createScene();

	void updateFrameStats();

private:
    static const Ogre::Real turboModifier;
    static const QPoint invalidMousePoint;

private:
    Ogre::Root          *mRoot;
    Ogre::SceneManager  *mSceneMgr;
    Ogre::RenderWindow  *mRenderWindow;
    Ogre::Viewport      *mViewport;
    Ogre::Camera        *mCamera;

    QPoint oldPos;
    Ogre::SceneNode *selectedNode;

	// NxOgre
	NxOgre::World* mPhysicsWorld;
	NxOgre::TimeController* mPhysicsTimeController;
	NxOgre::Scene* mPhysicsScene;
	OGRE3DRenderSystem* mPhysicsRenderSystem;
	NxOgre::VisualDebugger*	mVisualDebugger;
	OGRE3DRenderable*		mVisualDebuggerRenderable;
	Ogre::SceneNode*		mVisualDebuggerNode;

	Ogre::Overlay* mDebugOverlay;
};

#endif OGREWIDGET_H