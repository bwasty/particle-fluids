#include "stdafx.h"
#include "particlefluids.h"
#include "OgreWidget.h"
#include "qtvariantproperty.h"
#include <QtGroupBoxPropertyBrowser>
#include <QtGroupPropertyManager>
#include <NxOgre.h>

ParticleFluids::ParticleFluids(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	mOgreWidget = new OgreWidget(this);
	setCentralWidget(mOgreWidget);

	// let the OgreWidget redraw as often as possible
	QTimer* timer = new QTimer();
	timer->setInterval(0);
	connect(timer, SIGNAL(timeout()), mOgreWidget, SLOT(update()));
	timer->start();

	setupPhysXGUI();

	// TODO: show camera position in status bar

}



ParticleFluids::~ParticleFluids()
{

}

void ParticleFluids::setupPhysXGUI() {
	QtVariantPropertyManager* variantManager = new QtVariantPropertyManager(this);
	QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
	
	QtGroupBoxPropertyBrowser* propertyEditor = new QtGroupBoxPropertyBrowser(ui.dockWidgetPhysX);
    propertyEditor->setFactoryForManager(variantManager, variantFactory);

    ui.dockWidgetPhysX->setWidget(propertyEditor);

	propertyEditor->show();

	// TODO!!: get values from default fluid & emitter descriptions, catch signal valueChanged OR add "change"-Button
	QtGroupPropertyManager* groupManager = new QtGroupPropertyManager(this);
	QtProperty *group = groupManager->addProperty("Fluid");
	propertyEditor->addProperty(group);
	
	QtVariantProperty *property = variantManager->addProperty(QVariant::Int, "MaxParticles");
	property->setAttribute("singleStep", 200);
	property->setValue(mOgreWidget->mFluidDescription.mMaxParticles); // TODO!!!: make sure OgreWidget is initialized
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "KernelRadiusMultiplier");
	property->setValue(mOgreWidget->mFluidDescription.mKernelRadiusMultiplier);
	property->setAttribute("decimals", 4);
	group->addSubProperty(property);

	group = groupManager->addProperty("Emitter");
	propertyEditor->addProperty(group);

	group = groupManager->addProperty("General"); // Sim speed
	propertyEditor->addProperty(group);

	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), this, SLOT(propertyValueChanged(QtProperty *, const QVariant &)));
}

void ParticleFluids::propertyValueChanged(QtProperty* property, const QVariant & value) {
	//static int t = 0;
	//t++;
	//ui.statusBar->showMessage(QString("%1").arg(t));
	//TODO!!: check property name, change according variable
	//ui.statusBar->showMessage(property->propertyName());
	QString pName = property->propertyName();
	if (pName == "") {
	}
	else if (pName == "") {
	}

	// TODO!!!: make sure OgreWidget is initialized
	//mOgreWidget->createFluid();
}