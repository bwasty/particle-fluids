// base OgreWidget functionality from http://www.ogre3d.org/forums/viewtopic.php?p=367010&sid=5f3feeed878d9121b62dec0d82e7d19e#p367010 (orginally from Ogitor)
#include "stdafx.h"
#include "ogrewidget.h"
#include "particlefluids.h"

#include <Critter.h>
//#include "NxOgreSharedPointer.h"

using namespace Ogre;

const QPoint     OgreWidget::invalidMousePoint(-1,-1);
const Ogre::Real OgreWidget::turboModifier(10);

OgreWidget::OgreWidget(QWidget *parent)
:QWidget(parent),
mRoot(0), mSceneMgr(0), mRenderWindow(0), mViewport(0),
mCamera(0), oldPos(invalidMousePoint), selectedNode(0), mFluid(0), mSimulationSpeed(1)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_PaintOnScreen);
    setMinimumSize(240,240);
        setFocusPolicy(Qt::ClickFocus);

	// initial fluid parameters
	mFluidDescription.mMaxParticles = 60000;
	mFluidDescription.mKernelRadiusMultiplier = 2.0f;
	mFluidDescription.mRestParticlesPerMetre = 7.0f;
	mFluidDescription.mRestDensity = 1000.0f;
	mFluidDescription.mViscosity = 40.0f;
	mFluidDescription.mStiffness = 50.0f;
	mFluidDescription.mDamping = 0; // nonnegative
	mFluidDescription.mSurfaceTension = 0;
	mFluidDescription.mMotionLimitMultiplier = 3.0f;
	mFluidDescription.mPacketSizeMultiplier = 8;
	mFluidDescription.mCollisionDistanceMultiplier = 0.12f;
	mFluidDescription.mSimulationMethod = NxOgre::Enums::FluidSimulationMethod_SPH;
	mFluidDescription.mFlags |= NxOgre::Enums::FluidFlags_Hardware; // FluidFlags_CollisionTwoWay NxOgre::Enums::FluidFlags_DisableGravity
	mFluidDescription.mFlags ^= NxOgre::Enums::FluidFlags_Visualisation;
	//mFluidDescription.mExternalAcceleration.set(0,-9.81, 0);

	mEmitterDescription.mPose.set(0, 8, 0);
	mEmitterDescription.mParticleLifetime = 0;
	mEmitterDescription.mRate = 180;
	mEmitterDescription.mFluidSpeed = 3;
	mEmitterDescription.mType = NxOgre::Enums::FluidEmitterType_FlowRate;// NxOgre::Enums::FluidEmitterType_FlowRate FluidEmitterType_Pressure
	mEmitterDescription.mShape = NxOgre::Enums::FluidEmitterShape_Ellipse;
	mEmitterDescription.mRandomAngle = 0.25f;
	mEmitterDescription.mRandomPosition.set(0.25f, 0.25f, 0.25f);
	mEmitterDescription.mReplusionCoefficient = 0.02f;
	//edesc.mReplusionCoefficient = 0.8f; --> from other code snippet
	//edesc.mDimensionX = 4.0f;
	//edesc.mDimensionY = 4.0f;
}

OgreWidget::~OgreWidget()
{
    if(mRenderWindow)
    {
        mRenderWindow->removeAllViewports();
    }

    if(mRoot)
    {
        mRoot->detachRenderTarget(mRenderWindow);

        if(mSceneMgr)
        {
            mRoot->destroySceneManager(mSceneMgr);
        }
    }

    delete mRoot;
}

void OgreWidget::setBackgroundColor(QColor c)
{
    if(mViewport)
    {
        Ogre::ColourValue ogreColour;
        ogreColour.setAsARGB(c.rgba());
        mViewport->setBackgroundColour(ogreColour);
    }
}

//void OgreWidget::setCameraPosition(const Ogre::Vector3 &pos)
//{
//        mCamera->setPosition(pos);
//        //mCamera->lookAt(0,50,0);
//		//update();
//		//emit cameraPositionChanged(pos);
//}

void OgreWidget::keyPressEvent(QKeyEvent *e)
{
	// TODO?: camera moving speed?
    static QMap<int, Ogre::Vector3> keyCoordModificationMapping;
    static bool mappingInitialised = false;

    if(!mappingInitialised)
    {
            keyCoordModificationMapping[Qt::Key_W]         = Ogre::Vector3( 0, 0,-1);
            keyCoordModificationMapping[Qt::Key_S]         = Ogre::Vector3( 0, 0, 1);
            keyCoordModificationMapping[Qt::Key_A]         = Ogre::Vector3(-1, 0, 0);
            keyCoordModificationMapping[Qt::Key_D]         = Ogre::Vector3( 1, 0, 0);
            keyCoordModificationMapping[Qt::Key_E]		   = Ogre::Vector3( 0, 1, 0);
            keyCoordModificationMapping[Qt::Key_Q]		   = Ogre::Vector3( 0,-1, 0);

            mappingInitialised = true;
    }

    QMap<int, Ogre::Vector3>::iterator keyPressed =
            keyCoordModificationMapping.find(e->key());
    if(keyPressed != keyCoordModificationMapping.end() && mCamera)
    {
            //const Ogre::Vector3 &actualCamPos = mCamera->getPosition();
            //setCameraPosition(actualCamPos + keyPressed.value());
			mCamera->moveRelative(keyPressed.value() * 3);

            e->accept();
    }
	else if (e->key() == Qt::Key_P) {
		// TODO: visual debugger: Button in Toolbar
		static bool visualDebuggerOn = false;
		if (!visualDebuggerOn)
			mVisualDebugger->setVisualisationMode(NxOgre::Enums::VisualDebugger_ShowAll);
		else
			mVisualDebugger->setVisualisationMode(NxOgre::Enums::VisualDebugger_ShowNone);
		visualDebuggerOn = !visualDebuggerOn;
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
		// TODO: raytest: show pos in status bar instead showing bb
        Ogre::Real x = e->pos().x() / (float)width();
        Ogre::Real y = e->pos().y() / (float)height();

        Ogre::Ray ray = mCamera->getCameraToViewportRay(x, y);
        Ogre::RaySceneQuery *query = mSceneMgr->createRayQuery(ray);
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

        mSceneMgr->destroyQuery(query);

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

        //Ogre::Vector3 camTranslation(deltaX, deltaY, 0);
        //const Ogre::Vector3 &actualCamPos = mCamera->getPosition();
        //setCameraPosition(actualCamPos + camTranslation);
		static Real cameraRotationPerMouseMovement = 0.2;
		mCamera->yaw(Degree(-cameraRotationPerMouseMovement * deltaX));
        mCamera->pitch(Degree(-cameraRotationPerMouseMovement * deltaY));

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

    if(e->isAccepted() && mRenderWindow)
    {
        mRenderWindow->windowMovedOrResized();
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
    //mRoot->_fireFrameStarted();
    //mRenderWindow->update();

	mRoot->renderOneFrame();

    //mRoot->_fireFrameEnded();

    e->accept();
}

bool OgreWidget::frameRenderingQueued(const Ogre::FrameEvent &evt) {
	//mPhysicsTimeController->advance(evt.timeSinceLastFrame*mSimulationSpeed);//1.0f/60.0f);///evt.timeSinceLastFrame*mSimulationSpeed);//1.0f/60.0f);
	mPhysicsWorld->advance(evt.timeSinceLastFrame*mSimulationSpeed);
	mVisualDebugger->draw();
	mVisualDebuggerNode->needUpdate();

	updateFrameStats();

	return true;
}


bool OgreWidget::frameEnded(const Ogre::FrameEvent &evt) {


	return true;
}

void OgreWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    if(e->isAccepted())
    {
        const QSize &newSize = e->size();
        if(mRenderWindow)
        {
            mRenderWindow->resize(newSize.width(), newSize.height());
            mRenderWindow->windowMovedOrResized();
        }
        if(mCamera)
        {
            Ogre::Real aspectRatio = Ogre::Real(newSize.width()) / Ogre::Real(newSize.height());
            mCamera->setAspectRatio(aspectRatio);
        }
    }
}

void OgreWidget::showEvent(QShowEvent *e)
{
    if(!mRoot)
    {
        initOgreSystem();
    }

    QWidget::showEvent(e);
}

void OgreWidget::wheelEvent(QWheelEvent *e)
{
    //Ogre::Vector3 zTranslation(0,0, -e->delta() / 60);

    //if(e->modifiers().testFlag(Qt::ControlModifier))
    //{
    //    zTranslation.z *= turboModifier;
    //}

    //const Ogre::Vector3 &actualCamPos = mCamera->getPosition();
    //setCameraPosition(actualCamPos + zTranslation);
	// TODO?: mouse wheel zoom

    e->accept();
}

void OgreWidget::initOgreSystem()
{
    mRoot = new Ogre::Root();

	// TODO?: config options: do better
    Ogre::RenderSystem *renderSystem = mRoot->getRenderSystemByName("OpenGL Rendering Subsystem");
    mRoot->setRenderSystem(renderSystem);
    mRoot->initialise(false);

    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

    Ogre::NameValuePairList viewConfig;
    Ogre::String widgetHandle;
    widgetHandle = Ogre::StringConverter::toString((size_t)((HWND)winId()));
    viewConfig["externalWindowHandle"] = widgetHandle;
	//viewConfig["useNVPerfHUD"] = "true";
    mRenderWindow = mRoot->createRenderWindow("Ogre rendering window",
                width(), height(), false, &viewConfig);

    mCamera = mSceneMgr->createCamera("Camera");
    Ogre::Vector3 camPos(0, 10,30);
        mCamera->setPosition(camPos);
        mCamera->lookAt(0,0,0);
    //emit cameraPositionChanged(camPos);

    mViewport = mRenderWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(Ogre::ColourValue(0,0,0));
    mCamera->setAspectRatio(Ogre::Real(width()) / Ogre::Real(height()));
	mCamera->setNearClipDistance(1.0);

    setupResources();
	setupNxOgre();
    createScene();

	mDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	mDebugOverlay->remove2D(mDebugOverlay->getChild("Core/LogoPanel"));
	mDebugOverlay->show();

	mRoot->addFrameListener(this);
}

void OgreWidget::setupResources()
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

void OgreWidget::setupNxOgre() {
	mPhysicsWorld = NxOgre::World::createWorld();
	mPhysicsScene = mPhysicsWorld->createScene();
	mPhysicsScene->setGravity(NxOgre::Vec3(0.0f,-9.81f,0.0f));
	mPhysicsRenderSystem = new Critter::RenderSystem(mPhysicsScene);
	
	//mPhysicsTimeController = NxOgre::TimeController::getSingleton();

	mVisualDebugger = mPhysicsWorld->getVisualDebugger();
	mVisualDebuggerRenderable = new Critter::Renderable(NxOgre::Enums::RenderableType_VisualDebugger);
	mVisualDebugger->setRenderable(mVisualDebuggerRenderable);
	mVisualDebuggerNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mVisualDebuggerNode->attachObject(mVisualDebuggerRenderable);
	mVisualDebugger->setVisualisationMode(NxOgre::Enums::VisualDebugger_ShowNone);

	// Remote Debugger
	mPhysicsWorld->getRemoteDebugger()->connect();
}

void OgreWidget::createScene()
{
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5,0.5,0.5));


	Light *light = mSceneMgr->createLight("SunLight");
	light->setType(Light::LT_DIRECTIONAL);
	light->setDirection(-Vector3::UNIT_Y+Vector3::UNIT_X*0.2);
	light->setDiffuseColour(0.5, 0.5, 0.5);
	light->setSpecularColour(1.0, 1.0, 1.0);

    //Ogre::Entity *entity = mSceneMgr->createEntity("Axes", "axes.mesh");
    //Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("node");
    //node->attachObject(entity);
    //node->yaw(Ogre::Radian(Ogre::Degree(-90)));

	// visual ground plane
	Plane plane(Vector3::UNIT_Y, 0);
	MeshManager::getSingleton().createPlane("ground",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
		500,500,20,20,true,1,5,5,Vector3::UNIT_Z);

	Entity* groundEnt = mSceneMgr->createEntity("GroundEntity", "ground");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEnt);
	groundEnt->setMaterialName("Ground");
	groundEnt->setCastShadows(false);

	// physical ground plane
	NxOgre::PlaneGeometryDescription pdesc;
	mPhysicsScene->createSceneGeometry(pdesc);

	// box for surrounding the fluid
	mPhysicsScene->createSceneGeometry(NxOgre::BoxDescription(16,25,1), NxOgre::Vec3(0,0,-4));
	mPhysicsScene->createSceneGeometry(NxOgre::BoxDescription(16,25,1), NxOgre::Vec3(0,0, 4));
	mPhysicsScene->createSceneGeometry(NxOgre::BoxDescription(1,25,16), NxOgre::Vec3(-4,0, 0));
	mPhysicsScene->createSceneGeometry(NxOgre::BoxDescription(1,25,16), NxOgre::Vec3(4,0, 0));
	//mPhysicsScene->createSceneGeometry(NxOgre::BoxDescription(16,1,18), NxOgre::Vec3(0,4, 0));
	
	createFluid();

	//mPhysicsScene->createActor(NxOgre::BoxDescription(1,1,1), NxOgre::Vec3(0,10,0));
}

void OgreWidget::createFluid() {
	if (mFluid) {
		// TODO!!!!: unstable physx: Fluid::destroyEmitter not linking
		//mFluid->destroyEmitter(mEmitter);
		mPhysicsRenderSystem->destroyFluid(mFluid);
		mFluid = 0;
	}

	mFluid = mPhysicsRenderSystem->createFluid(mFluidDescription, "SpheresShaded"/*"BaseWhiteNoLighting"*/, Critter::Enums::FluidType_Position); //FluidType_Velocity FluidType_Position FluidType_OgreParticle
	mEmitter = mFluid->createEmitter(mEmitterDescription);
}

void OgreWidget::updateFrameStats(void)
{
	static String currFps = "Current FPS: ";
	static String avgFps = "Average FPS: ";
	static String bestFps = "Best FPS: ";
	static String worstFps = "Worst FPS: ";
	static String tris = "Triangle Count: ";
	static String batches = "Batch Count: ";

	// update stats when necessary
	try {
		OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
		OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
		OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
		OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

		const RenderTarget::FrameStats& stats = mRenderWindow->getStatistics();
		guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
		guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
		guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
			+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
		guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
			+" "+StringConverter::toString(stats.worstFrameTime)+" ms");

		OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
		guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

		OverlayElement* guiBatches = OverlayManager::getSingleton().getOverlayElement("Core/NumBatches");
		guiBatches->setCaption(batches + StringConverter::toString(stats.batchCount));

		OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
		//guiDbg->setCaption(mDebugText);
	}
	catch(...) { /* ignore */ }

	//int particleCount = *mFluid->getParticleData()->mNbParticles.get();
	int particleCount = mFluid->getNbParticles();
	((ParticleFluids*)parent())->mLabelParticleCount->setText(QString("Particle Count : %1").arg(particleCount));
}
