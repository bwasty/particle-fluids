#include "stdafx.h"
#include "particlefluids.h"
#include "OgreWidget.h"

ParticleFluids::ParticleFluids(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	OgreWidget* ow = new OgreWidget(this);
	setCentralWidget(ow);

	QTimer* timer = new QTimer();
	timer->setInterval(0);
	connect(timer, SIGNAL(timeout()), ow, SLOT(update()));
	timer->start();

	// TODO: show camera position in status bar
}

ParticleFluids::~ParticleFluids()
{

}
