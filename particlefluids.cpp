#include "stdafx.h"
#include "particlefluids.h"
#include "OgreWidget.h"
#include "qtvariantproperty.h"
#include <QtGroupBoxPropertyBrowser>

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

	// TODO!!: Property Browser
	QtVariantPropertyManager* variantManager = new QtVariantPropertyManager(this);
	QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
	
	QtGroupBoxPropertyBrowser* propertyEditor = new QtGroupBoxPropertyBrowser(ui.dockWidgetPhysX);
    propertyEditor->setFactoryForManager(variantManager, variantFactory);

    ui.dockWidgetPhysX->setWidget(propertyEditor);

	propertyEditor->show();

	QtVariantProperty *maxParticles = variantManager->addProperty(QVariant::Int, "Max Particles");
	maxParticles->setValue(40000);

	propertyEditor->addProperty(maxParticles);
}

ParticleFluids::~ParticleFluids()
{

}
