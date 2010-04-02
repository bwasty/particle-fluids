#include "stdafx.h"
#include "particlefluids.h"
#include "OgreWidget.h"

ParticleFluids::ParticleFluids(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	QtOgre::OgreWidget* ow = new QtOgre::OgreWidget(this);
	ui.centralWidget = ow;
}

ParticleFluids::~ParticleFluids()
{

}
