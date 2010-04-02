// from http://projectify.blogspot.com/2009/06/qt-ogre-vs2008-express.html
#ifndef QTOGRE_OGREWIDGET_H_
#define QTOGRE_OGREWIDGET_H_

#include <OgreCommon.h>
#include <OgrePrerequisites.h>

#include <QSettings>
#include <QWidget>
#include <QTimer>

namespace QtOgre
{
	class InputEventHandler;

	class OgreWidget : public QWidget
	{
		Q_OBJECT

	public:
		OgreWidget(QWidget* pParentWidget=0, Qt::WindowFlags f=0);
		~OgreWidget();

		Ogre::RenderWindow* getOgreRenderWindow() const;

		//QPaintEngine* paintEngine() const;



	public:
		Ogre::RenderWindow* renderWindow()
		{
			return m_pOgreRenderWindow;
		}
		Ogre::RenderWindow* m_pOgreRenderWindow;

	protected:
		void paintEvent(QPaintEvent* evt);
		void resizeEvent(QResizeEvent* evt);

	private:
		void initialiseOgre();
		void moveAndResize();

		QWidget* m_pParentWidget;

		Ogre::RenderSystem* m_ActiveRenderSystem;
		Ogre::Root* m_Root;

		QTimer* m_UpdateTimer;
	};
}

#endif /*QTOGRE_OGREWIDGET_H_*/ 