#include "stdafx.h"
#include "particlefluids.h"
#include "OgreWidget.h"

ParticleFluids::ParticleFluids(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	OgreWidget* ow = new OgreWidget(this);
	setCentralWidget(ow);
}

ParticleFluids::~ParticleFluids()
{

}
