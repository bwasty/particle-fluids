// from http://projectify.blogspot.com/2009/06/qt-ogre-vs2008-express.html
#include "stdafx.h"
#include "OgreWidget.h"

#pragma warning( push )
#pragma warning( disable : 4100 )

#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreStringConverter.h>
#include <OgreRenderSystem.h>

#pragma warning( pop ) 

#include <windows.h>


namespace QtOgre
{
	OgreWidget::OgreWidget(QWidget* pParentWidget, Qt::WindowFlags f)
	:QWidget(pParentWidget, f | Qt::MSWindowsOwnDC)
	,m_pOgreRenderWindow(0)
	,m_pParentWidget(pParentWidget)
	{		
		QWidget *q_parent = dynamic_cast <QWidget *> (parent());
		
		//It is possible you might need one of these on other platforms
		//setAttribute(Qt::WA_PaintOnScreen);
		//setAttribute(Qt::WA_NoSystemBackground);
		//setAttribute(Qt::WA_OpaquePaintEvent);
		//setAutoFillBackground( false );

		//Create the ogre root singleton
		m_Root = new Ogre::Root();
		//Initialise the ogre system, select the render plugins (if OpenGL is present, OpenGL is selected)
		initialiseOgre();

		//Set window parameters
		Ogre::NameValuePairList ogreWindowParams;
		//Full Screen Anti Aliasing
		ogreWindowParams["FSAA"] = "8";
		//Other code needed for Linux
		ogreWindowParams["parentWindowHandle"] = Ogre::StringConverter::toString((unsigned long)q_parent->winId());

		//Finally create our window.
		m_pOgreRenderWindow = Ogre::Root::getSingletonPtr()->createRenderWindow("OgreWindow", width(), height(), false, &ogreWindowParams);

		WId window_id;
		//Other code need for linux
		m_pOgreRenderWindow->getCustomAttribute ("HWND", &window_id);

		// Take over the ogre created window.
		QWidget::create (window_id);

		moveAndResize();

		//Connect a timer to the update method
		m_UpdateTimer = new QTimer;
		QObject::connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
		//Reducing the timer should give a higher framerate
		m_UpdateTimer->start(40);
	}

	OgreWidget::~OgreWidget()
	{
	}

	Ogre::RenderWindow* OgreWidget::getOgreRenderWindow() const
	{
		return m_pOgreRenderWindow;
	}

	/* Adding this was the solution to a flickering issue for someone using Qt and Ogre
	QPaintEngine *OgreWidget:: paintEngine() const
	{
		return 0;
	}*/

	void OgreWidget::paintEvent(QPaintEvent* /*evt*/)
	{
		Ogre::Root::getSingleton()._fireFrameStarted();
		m_pOgreRenderWindow->update();
		Ogre::Root::getSingleton()._fireFrameRenderingQueued();
		Ogre::Root::getSingleton()._fireFrameEnded();
	}

	void OgreWidget::resizeEvent(QResizeEvent* /*evt*/)
	{
		moveAndResize();
	}

	void OgreWidget::moveAndResize()
	{
		m_pOgreRenderWindow->reposition (x(),y());
		m_pOgreRenderWindow->resize(width(), height());
		m_pOgreRenderWindow->windowMovedOrResized();

		for(int ct = 0; ct < m_pOgreRenderWindow->getNumViewports(); ++ct)
		{
			Ogre::Viewport* pViewport = m_pOgreRenderWindow->getViewport(ct);
			Ogre::Camera* pCamera = pViewport->getCamera();
			pCamera->setAspectRatio(static_cast<Ogre::Real>(pViewport->getActualWidth()) / static_cast<Ogre::Real>(pViewport->getActualHeight()));
		}
	}

	void OgreWidget::initialiseOgre(void)
	{			
		//This will choose the OpenGL rendersystem as default
		//and will try to use Direct3D if loading OpenGL failed
		Ogre::RenderSystem* OpenGLRenderSystem = 0;
		Ogre::RenderSystem* Direct3D9RenderSystem = 0;

		try
		{
			#if defined(_DEBUG)			
				m_Root->loadPlugin("RenderSystem_GL_d");
			#else
				m_Root->loadPlugin("RenderSystem_GL");		
			#endif
		}
		catch(...)
		{
			qWarning("Failed to load OpenGL plugin");
		}
		try
		{
			#if defined(_DEBUG)			
				m_Root->loadPlugin("RenderSystem_Direct3D9_d");
			#else
				m_Root->loadPlugin("RenderSystem_Direct3D9");		
			#endif
		}
		catch(...)
		{
			qWarning("Failed to load Direct3D9 plugin");
		}

		Ogre::RenderSystemList list = Ogre::Root::getSingletonPtr()->getAvailableRenderers();
		Ogre::RenderSystemList::iterator i = list.begin();

		while (i != list.end())
		{
			if ((*i)->getName() == "OpenGL Rendering Subsystem")
			{
				OpenGLRenderSystem = *i;
			}
			if ((*i)->getName() == "Direct3D9 Rendering Subsystem")
			{
				Direct3D9RenderSystem = *i;
			}
			i++;
		}

		if(!(OpenGLRenderSystem || Direct3D9RenderSystem))
		{
			qCritical("No rendering subsystems found");
			exit(0);
		}

		if(OpenGLRenderSystem != 0)
		{
			m_ActiveRenderSystem = OpenGLRenderSystem;
		}
		else if(Direct3D9RenderSystem != 0)
		{
			m_ActiveRenderSystem = Direct3D9RenderSystem;
		}
		
		Ogre::Root::getSingletonPtr()->setRenderSystem(m_ActiveRenderSystem);

		Ogre::Root::getSingletonPtr()->initialise(false);
	}
}