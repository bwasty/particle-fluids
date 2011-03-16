#include "stdafx.h"
#include "particlefluids.h"
#include <QtGui/QApplication>

//#include <OgreException.h>

int main(int argc, char *argv[])
{
    //try {

	QApplication a(argc, argv);
	ParticleFluids w;
	w.show();
	return a.exec();

    //} 
    //catch (Ogre::InvalidStateException e) {
    //    std::cout << e.getDescription();
    //}
}
