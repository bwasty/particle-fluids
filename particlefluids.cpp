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
	property->setValue(mOgreWidget->mFluidDescription.mMaxParticles);
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "KernelRadiusMultiplier");
	property->setValue(mOgreWidget->mFluidDescription.mKernelRadiusMultiplier);
	property->setAttribute("decimals", 4);
	property->setToolTip("along with restParticlesPerMeter, controls the radius of influence for each particle. \n\
radius = KernelRadiusMultiplier / RestParticlesPerMeter. Should be set around 2.0 and definitely below 2.5 for optimal performance and simulation quality.");
	group->addSubProperty(property);

	//TODO!!!: add rest of fluid, emitter, general parameters
	property = variantManager->addProperty(QVariant::Double, "RestParticlesPerMeter");
	property->setValue(mOgreWidget->mFluidDescription.mRestParticlesPerMetre);
	property->setAttribute("decimals", 4);
	property->setToolTip("The particle resolution given as particles per linear meter measured when the fluid is in its rest state (relaxed).\n\
Even if the particle system is simulated without particle interactions, this parameter defines the emission density of the emitters.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "RestDensity");
	property->setValue(mOgreWidget->mFluidDescription.mRestDensity);
	property->setToolTip("Target density for the fluid (water is about 1000). mass = restDensity/(restParticlesPerMeter^3).\n\
The particle mass has an impact on the repulsion effect on emitters and actors.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "Viscosity");
	property->setValue(mOgreWidget->mFluidDescription.mViscosity);
	property->setAttribute("decimals", 4);
	property->setToolTip("Must be positive. Higher values will result in a honey-like behavior.\n Viscosity is an effect which depends on the relative velocity of neighboring particles; it reduces the magnitude of the relative velocity");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "Stiffness");
	property->setValue(mOgreWidget->mFluidDescription.mStiffness);
	property->setAttribute("decimals", 4);
	property->setToolTip("Must be positive. The stiffness of the particle interaction related to the pressure.\n\
This factor linearly scales the force which acts on particles which are closer to each other than the rest spacing.\n\
Setting this parameter appropriately is crucial for the simulation. The right value depends on many factors such as viscosity,\n\
damping, and kernelRadiusMultiplier. Values which are too high will result in an unstable simulation, \n\
whereas too low values will make the fluid appear \"springy\" (the fluid acts more compressible).");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "Damping");
	property->setValue(mOgreWidget->mFluidDescription.mDamping);
	property->setAttribute("decimals", 4);
	property->setToolTip("must be nonnegative. Reduces the velocity of the particles over time.\nSetting the damping to 0 will leave the particles unaffected.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "SurfaceTension");
	property->setValue(mOgreWidget->mFluidDescription.mSurfaceTension);
	property->setAttribute("decimals", 4);
	property->setToolTip("Must be nonnegative. Defines an attractive force between particles. \nHigher values will result in smoother surfaces.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "MotionLimitMultiplier");
	property->setValue(mOgreWidget->mFluidDescription.mMotionLimitMultiplier);
	property->setAttribute("decimals", 4);
	property->setToolTip("Maximal distance a particle is allowed to travel within one timestep. Default value is 3.6 (i.e., 3.0 * kernelRadiusMultiplier). \n\
The value must not be higher than the product of packetSizeMultiplier and kernelRadiusMultiplier.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "SimulationMethod");
	QStringList enums;
	enums << "SPH" << "Mixed Mode" << "No Particle Interactions";
	property->setAttribute("enumNames", enums);
	group->addSubProperty(property);

	//mFlags
	property = variantManager->addProperty(QtVariantPropertyManager::flagTypeId(), "Flags");
	QStringList flags;
	flags << "Hardware" << "CollisionTwoWay" << "DisableGravity";
	property->setAttribute("flagNames", flags);
	property->setValue(1); // TODO: set fluid flags properly?
	group->addSubProperty(property);


	group = groupManager->addProperty("Emitter");
	propertyEditor->addProperty(group);

	//mPose
	//mParticleLifetime
	//mRate
	//mType
	//...

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

	mOgreWidget->createFluid();
}