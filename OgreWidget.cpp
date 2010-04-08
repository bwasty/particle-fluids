// from http://www.ogre3d.org/forums/viewtopic.php?p=367010&sid=5f3feeed878d9121b62dec0d82e7d19e#p367010 (orginally from Ogitor)
#include "stdafx.h"
#include "ogrewidget.h"

const QPoint     OgreWidget::invalidMousePoint(-1,-1);
const Ogre::Real OgreWidget::turboModifier(10);

OgreWidget::OgreWidget(QWidget *parent)
:QWidget(parent),
ogreRoot(0), ogreSceneManager(0), ogreRenderWindow(0), ogreViewport(0),
ogreCamera(0), oldPos(invalidMousePoint), selectedNode(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_PaintOnScreen);
    setMinimumSize(240,240);
        setFocusPolicy(Qt::ClickFocus);
}

OgreWidget::~OgreWidget()
{
    if(ogreRenderWindow)
    {
        ogreRenderWindow->removeAllViewports();
    }

    if(ogreRoot)
    {
        ogreRoot->detachRenderTarget(ogreRenderWindow);

        if(ogreSceneManager)
        {
            ogreRoot->destroySceneManager(ogreSceneManager);
        }
    }

    delete ogreRoot;
}

void OgreWidget::setBackgroundColor(QColor c)
{
    if(ogreViewport)
    {
        Ogre::ColourValue ogreColour;
        ogreColour.setAsARGB(c.rgba());
        ogreViewport->setBackgroundColour(ogreColour);
    }
}

void OgreWidget::setCameraPosition(const Ogre::Vector3 &pos)
{
        ogreCamera->setPosition(pos);
        ogreCamera->lookAt(0,50,0);
    update();
    emit cameraPositionChanged(pos);
}

void OgreWidget::keyPressEvent(QKeyEvent *e)
{
        static QMap<int, Ogre::Vector3> keyCoordModificationMapping;
        static bool mappingInitialised = false;

        if(!mappingInitialised)
        {
                keyCoordModificationMapping[Qt::Key_W]         = Ogre::Vector3( 0, 0,-5);
                keyCoordModificationMapping[Qt::Key_S]         = Ogre::Vector3( 0, 0, 5);
                keyCoordModificationMapping[Qt::Key_A]         = Ogre::Vector3(-5, 0, 0);
                keyCoordModificationMapping[Qt::Key_D]         = Ogre::Vector3( 5, 0, 0);
                keyCoordModificationMapping[Qt::Key_PageUp]   = Ogre::Vector3( 0, 5, 0);
                keyCoordModificationMapping[Qt::Key_PageDown] = Ogre::Vector3( 0,-5, 0);

                mappingInitialised = true;
        }

        QMap<int, Ogre::Vector3>::iterator keyPressed =
                keyCoordModificationMapping.find(e->key());
        if(keyPressed != keyCoordModificationMapping.end() && ogreCamera)
        {
                const Ogre::Vector3 &actualCamPos = ogreCamera->getPosition();
                setCameraPosition(actualCamPos + keyPressed.value());

                e->accept();
        }
    else
    {
        e->ignore();
    }
}

void OgreWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->buttons().testFlag(Qt::LeftButton))
    {
        Ogre::Real x = e->pos().x() / (float)width();
        Ogre::Real y = e->pos().y() / (float)height();

        Ogre::Ray ray = ogreCamera->getCameraToViewportRay(x, y);
        Ogre::RaySceneQuery *query = ogreSceneManager->createRayQuery(ray);
        Ogre::RaySceneQueryResult &queryResult = query->execute();
        Ogre::RaySceneQueryResult::iterator queryResultIterator = queryResult.begin();

        if(queryResultIterator != queryResult.end())
        {
            if(queryResultIterator->movable)
            {
                selectedNode = queryResultIterator->movable->getParentSceneNode();
                selectedNode->showBoundingBox(true);
            }
        }
        else
        {
            selectedNode->showBoundingBox(false);
            selectedNode = 0;
        }

        ogreSceneManager->destroyQuery(query);

        update();
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void OgreWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons().testFlag(Qt::LeftButton) && oldPos != invalidMousePoint)
    {
        const QPoint &pos = e->pos();
        Ogre::Real deltaX = pos.x() - oldPos.x();
        Ogre::Real deltaY = pos.y() - oldPos.y();

        if(e->modifiers().testFlag(Qt::ControlModifier))
        {
            deltaX *= turboModifier;
            deltaY *= turboModifier;
        }

        Ogre::Vector3 camTranslation(deltaX, deltaY, 0);
        const Ogre::Vector3 &actualCamPos = ogreCamera->getPosition();
        setCameraPosition(actualCamPos + camTranslation);

        oldPos = pos;
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void OgreWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->buttons().testFlag(Qt::LeftButton))
    {
        oldPos = e->pos();
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void OgreWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(!e->buttons().testFlag(Qt::LeftButton))
    {
        oldPos = QPoint(invalidMousePoint);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void OgreWidget::moveEvent(QMoveEvent *e)
{
    QWidget::moveEvent(e);

    if(e->isAccepted() && ogreRenderWindow)
    {
        ogreRenderWindow->windowMovedOrResized();
        update();
    }
}

QPaintEngine* OgreWidget::paintEngine() const
{
    // We don't want another paint engine to get in the way for our Ogre based paint engine.
    // So we return nothing.
    return NULL;
}

void OgreWidget::paintEvent(QPaintEvent *e)
{
	// TODO!: paintEvent: see other OgreWidget...frameRenderingQueued? also look renderOneFrame!
    ogreRoot->_fireFrameStarted();
        ogreRenderWindow->update();
    ogreRoot->_fireFrameEnded();

    e->accept();
}

void OgreWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    if(e->isAccepted())
    {
        const QSize &newSize = e->size();
        if(ogreRenderWindow)
        {
            ogreRenderWindow->resize(newSize.width(), newSize.height());
            ogreRenderWindow->windowMovedOrResized();
        }
        if(ogreCamera)
        {
            Ogre::Real aspectRatio = Ogre::Real(newSize.width()) / Ogre::Real(newSize.height());
            ogreCamera->setAspectRatio(aspectRatio);
        }
    }
}

void OgreWidget::showEvent(QShowEvent *e)
{
    if(!ogreRoot)
    {
        initOgreSystem();
    }

    QWidget::showEvent(e);
}

void OgreWidget::wheelEvent(QWheelEvent *e)
{
    Ogre::Vector3 zTranslation(0,0, -e->delta() / 60);

    if(e->modifiers().testFlag(Qt::ControlModifier))
    {
        zTranslation.z *= turboModifier;
    }

    const Ogre::Vector3 &actualCamPos = ogreCamera->getPosition();
    setCameraPosition(actualCamPos + zTranslation);

    e->accept();
}

void OgreWidget::initOgreSystem()
{
    ogreRoot = new Ogre::Root();

	// TODO: config options: do better
    Ogre::RenderSystem *renderSystem = ogreRoot->getRenderSystemByName("OpenGL Rendering Subsystem");
    ogreRoot->setRenderSystem(renderSystem);
    ogreRoot->initialise(false);

    ogreSceneManager = ogreRoot->createSceneManager(Ogre::ST_GENERIC);

    Ogre::NameValuePairList viewConfig;
    Ogre::String widgetHandle;
    widgetHandle = Ogre::StringConverter::toString((size_t)((HWND)winId()));
    viewConfig["externalWindowHandle"] = widgetHandle;
    ogreRenderWindow = ogreRoot->createRenderWindow("Ogre rendering window",
                width(), height(), false, &viewConfig);

    ogreCamera = ogreSceneManager->createCamera("Camera");
    Ogre::Vector3 camPos(0, 50,150);
        ogreCamera->setPosition(camPos);
        ogreCamera->lookAt(0,50,0);
    emit cameraPositionChanged(camPos);

	// TODO: near/far clip
    ogreViewport = ogreRenderWindow->addViewport(ogreCamera);
    ogreViewport->setBackgroundColour(Ogre::ColourValue(0,0,0));
    ogreCamera->setAspectRatio(Ogre::Real(width()) / Ogre::Real(height()));

        setupNLoadResources();
        createScene();
}

void OgreWidget::setupNLoadResources() // TODO: setupNLoadResources -> setupResources?
{
        // Load resource paths from config file
        Ogre::ConfigFile cf;
        cf.load("resources.cfg"); 

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
                secName = seci.peekNextKey();
                Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
                Ogre::ConfigFile::SettingsMultiMap::iterator i;
                for (i = settings->begin(); i != settings->end(); ++i)
                {
                        typeName = i->first;
                        archName = i->second;
                        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                                archName, typeName, secName);
                }
        }

        // Initialise, parse scripts etc
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void OgreWidget::createScene()
{
        ogreSceneManager->setAmbientLight(Ogre::ColourValue(1,1,1));

        Ogre::Entity *robotEntity = ogreSceneManager->createEntity("Axs", "axes.mesh");
        Ogre::SceneNode *robotNode = ogreSceneManager->getRootSceneNode()->createChildSceneNode("RobotNode");
        robotNode->attachObject(robotEntity);
        robotNode->yaw(Ogre::Radian(Ogre::Degree(-90)));
}