#include "stdafx.h"
#include "particlefluids.h"
#include "OgreWidget.h"
#include "qtvariantproperty.h"
#include <QtGroupBoxPropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtTreePropertyBrowser>
#include <QtButtonPropertyBrowser>
#include <NxOgre.h>

#include "qdebugstream.h"

ParticleFluids::ParticleFluids(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	QPlainTextEdit* consoleOutput = new QPlainTextEdit();
	consoleOutput->setReadOnly(true);
	ui.dockWidgetConsole->setWidget(consoleOutput);

	mCout = new QDebugStream(std::cout, consoleOutput);
	mCerr = new QDebugStream(std::cerr, consoleOutput);

	mOgreWidget = new OgreWidget(this);
	setCentralWidget(mOgreWidget);

	// let the OgreWidget redraw as often as possible
	QTimer* timer = new QTimer();
	timer->setInterval(0);
	connect(timer, SIGNAL(timeout()), mOgreWidget, SLOT(update()));
	timer->start();

	setupPhysXGUI();

	// TODO!: show camera position in status bar
	mLabelParticleCount = new QLabel("Particle Count: 0");
	statusBar()->addPermanentWidget(mLabelParticleCount);
}

ParticleFluids::~ParticleFluids()
{
	delete mCout;
	delete mCerr;
}

void ParticleFluids::setupPhysXGUI() {
	QtVariantPropertyManager* variantManager = new QtVariantPropertyManager(this);
	QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
	
	//QtGroupBoxPropertyBrowser* propertyEditor = new QtGroupBoxPropertyBrowser(ui.dockWidgetPhysX);
	//QtTreePropertyBrowser* propertyEditor = new QtTreePropertyBrowser(ui.dockWidgetPhysX);
	QtButtonPropertyBrowser* propertyEditor = new QtButtonPropertyBrowser(ui.dockWidgetPhysX);
    
	propertyEditor->setFactoryForManager(variantManager, variantFactory);

    ui.dockWidgetPhysX->setWidget(propertyEditor);

	propertyEditor->show();

	QtGroupPropertyManager* groupManager = new QtGroupPropertyManager(this);

	//QtProperty *group = groupManager->addProperty("General"); // Sim speed, sim paused (speed = 0?)
	//QtBrowserItem* generalGroupItem = propertyEditor->addProperty(group);

	QtVariantProperty *property = variantManager->addProperty(QVariant::Double, "Simulation Speed");
	property->setValue(1);
	property->setAttribute("minimum", 0);
	property->setToolTip("the time difference for the PhysX simulation step is multiplied by this factor");
	propertyEditor->addProperty(property);

	QtProperty *group = groupManager->addProperty("Fluid");
	QtBrowserItem* fluidGroupItem = propertyEditor->addProperty(group);
	
	property = variantManager->addProperty(QVariant::Int, "MaxParticles");
	property->setAttribute("singleStep", 200);
	property->setAttribute("minimum", 1);
	property->setAttribute("maximum", 65535);
	property->setValue(mOgreWidget->mFluidDescription.mMaxParticles);
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "KernelRadiusMultiplier");
	property->setValue(mOgreWidget->mFluidDescription.mKernelRadiusMultiplier);
	//property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 1);
	property->setAttribute("singleStep", 0.1);
	property->setToolTip("along with restParticlesPerMeter, controls the radius of influence for each particle. \n\
radius = KernelRadiusMultiplier / RestParticlesPerMeter. Should be set around 2.0 and definitely below 2.5 for optimal performance and simulation quality.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "RestParticlesPerMeter");
	property->setValue(mOgreWidget->mFluidDescription.mRestParticlesPerMetre);
	//property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 0.01);
	property->setAttribute("singleStep", 0.1);
	property->setToolTip("The particle resolution given as particles per linear meter measured when the fluid is in its rest state (relaxed).\n\
Even if the particle system is simulated without particle interactions, this parameter defines the emission density of the emitters.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "RestDensity");
	property->setValue(mOgreWidget->mFluidDescription.mRestDensity);
	property->setAttribute("minimum", 0.01);
	property->setAttribute("singleStep", 10);
	property->setToolTip("Target density for the fluid (water is about 1000). mass = restDensity/(restParticlesPerMeter^3).\n\
The particle mass has an impact on the repulsion effect on emitters and actors.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "Viscosity");
	property->setValue(mOgreWidget->mFluidDescription.mViscosity);
	//property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 0.01);
	property->setToolTip("Must be positive. Higher values will result in a honey-like behavior.\n Viscosity is an effect which depends on the relative velocity of neighboring particles; it reduces the magnitude of the relative velocity");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "Stiffness");
	property->setValue(mOgreWidget->mFluidDescription.mStiffness);
	//property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 0.01);
	property->setToolTip("Must be positive. The stiffness of the particle interaction related to the pressure.\n\
This factor linearly scales the force which acts on particles which are closer to each other than the rest spacing.\n\
Setting this parameter appropriately is crucial for the simulation. The right value depends on many factors such as viscosity,\n\
damping, and kernelRadiusMultiplier. Values which are too high will result in an unstable simulation, \n\
whereas too low values will make the fluid appear \"springy\" (the fluid acts more compressible).");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "Damping");
	property->setValue(mOgreWidget->mFluidDescription.mDamping);
	//property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 0);
	property->setToolTip("must be nonnegative. Reduces the velocity of the particles over time.\nSetting the damping to 0 will leave the particles unaffected.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "SurfaceTension");
	property->setValue(mOgreWidget->mFluidDescription.mSurfaceTension);
	//property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 0);
	property->setToolTip("Must be nonnegative. Defines an attractive force between particles. \nHigher values will result in smoother surfaces.");
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "MotionLimitMultiplier");
	property->setValue(mOgreWidget->mFluidDescription.mMotionLimitMultiplier);
	//property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 1);
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
	property->setToolTip("NX_F_MIXED_MODE - alternates between SPH and simple mode (providing more performance than SPH alone, while maintaining some dense characteristics).");
	group->addSubProperty(property);


	group = groupManager->addProperty("Emitter");
	propertyEditor->addProperty(group);

	//mPose

	property = variantManager->addProperty(QVariant::Double, "ParticleLifetime");
	property->setValue(mOgreWidget->mEmitterDescription.mParticleLifetime);
	property->setAttribute("decimals", 4);
	property->setAttribute("minimum", 0);
	property->setToolTip("in seconds. if 0: particle will live until collides with drain");
	group->addSubProperty(property);

	//mRate
	property = variantManager->addProperty(QVariant::Double, "Rate");
	property->setValue(mOgreWidget->mEmitterDescription.mRate);
	property->setAttribute("singleStep", 10);
	property->setAttribute("minimum", 0);
	property->setToolTip("The rate specifies how many particles are emitted per second.\nThe rate is only considered in the simulation if the type is set to NX_FE_CONSTANT_FLOW_RATE.");
	group->addSubProperty(property);

	// mType - FlowRate, Pressure
	property = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), "Type");
	enums = QStringList();
	enums << "ConstantFlowRate" << "ConstantPressure";
	property->setAttribute("enumNames", enums);
	// TODO: set flag value properly from emitter description?
	group->addSubProperty(property);

	property = variantManager->addProperty(QVariant::Double, "FluidSpeed");
	property->setValue(mOgreWidget->mEmitterDescription.mFluidSpeed);
	property->setAttribute("minimum", 0);
	property->setToolTip("The velocity magnitude of the emitted fluid particles. Default 1.0?");
	group->addSubProperty(property);

	// shape
	// dimensionX, dimensionY 
	// randomAngle 
	// randomPos
	// if frameShape used: flags NX_FEF_FORCE_ON_BODY, NX_FEF_ADD_BODY_VELOCITY

    propertyEditor->setExpanded(fluidGroupItem, true);
	//propertyEditor->setExpanded(generalGroupItem, true);

	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)), 
		this, SLOT(propertyValueChanged(QtProperty *, const QVariant &)));
}

void ParticleFluids::propertyValueChanged(QtProperty* property, const QVariant & value) {
	ui.statusBar->showMessage(QString("%1 %2").arg(property->propertyName()).arg(value.toString()));

	bool shouldRecreate = false;

	QString pName = property->propertyName();
	if (pName == "Simulation Speed") {
		mOgreWidget->mSimulationSpeed = value.toFloat();
	}
	else if (pName == "MaxParticles") {
		mOgreWidget->mFluidDescription.mMaxParticles = value.toUInt();
		shouldRecreate = true;
	}
	else if (pName == "KernelRadiusMultiplier") {
		mOgreWidget->mFluidDescription.mKernelRadiusMultiplier = value.toFloat();
		shouldRecreate = true;
	}
	else if (pName == "RestParticlesPerMeter") {
		mOgreWidget->mFluidDescription.mRestParticlesPerMetre = value.toFloat();
		shouldRecreate = true;
	}
	else if (pName == "RestDensity") {
		mOgreWidget->mFluidDescription.mRestDensity = value.toFloat();
		shouldRecreate = true;
	}
	else if (pName == "Viscosity") {
		mOgreWidget->mFluidDescription.mViscosity = value.toFloat();
		//mOgreWidget->mFluid->setViscosity(mOgreWidget->mFluidDescription.mViscosity); // doesn't change anything in current fluid
		shouldRecreate = true;
	}
	else if (pName == "Stiffness") {
		mOgreWidget->mFluidDescription.mStiffness = value.toFloat();
		mOgreWidget->mFluid->setStiffness(mOgreWidget->mFluidDescription.mStiffness);
	}
	else if (pName == "Damping") {
		mOgreWidget->mFluidDescription.mDamping = value.toFloat();
		mOgreWidget->mFluid->setDamping(mOgreWidget->mFluidDescription.mDamping);
	}
	else if (pName == "SurfaceTension") {
		mOgreWidget->mFluidDescription.mSurfaceTension = value.toFloat();
		mOgreWidget->mFluid->setSurfaceTension(mOgreWidget->mFluidDescription.mSurfaceTension);
	}
	else if (pName == "MotionLimitMultiplier") {
		mOgreWidget->mFluidDescription.mMotionLimitMultiplier = value.toFloat();
		shouldRecreate = true;
	}
	else if (pName == "SimulationMethod") {
		int e = value.toInt();
		int converted;
		if (e == 0)
			converted = NxOgre::Enums::FluidSimulationMethod_SPH;
		else if (e == 1)
			converted = NxOgre::Enums::FluidSimulationMethod_MixedMode;
		else if (e == 2)
			converted = NxOgre::Enums::FluidSimulationMethod_NoParticleInteraction;

		mOgreWidget->mFluidDescription.mSimulationMethod = converted;
		mOgreWidget->mFluid->setSimulationMethod(converted);		
	}
	// Flags
	else if (pName == "Hardware") {
		mOgreWidget->mFluidDescription.mFlags ^= NxOgre::Enums::FluidFlags_Hardware;
		//mOgreWidget->mFluid->setFlag(NxOgre::Enums::FluidFlags_Hardware, value.toBool()); // doesn't change anything in current fluid
		shouldRecreate = true;
	}
	else if (pName == "CollisionTwoWay") { // TODO!: test CollisionTwoWay flag
		mOgreWidget->mFluidDescription.mFlags ^= NxOgre::Enums::FluidFlags_CollisionTwoWay;
		mOgreWidget->mFluid->setFlag(NxOgre::Enums::FluidFlags_CollisionTwoWay, value.toBool());
	}
	else if (pName == "DisableGravity") {
		mOgreWidget->mFluidDescription.mFlags ^= NxOgre::Enums::FluidFlags_DisableGravity;
		mOgreWidget->mFluid->setFlag(NxOgre::Enums::FluidFlags_DisableGravity, value.toBool());
	}
	else if (pName == "ParticleLifetime") {
		mOgreWidget->mEmitterDescription.mParticleLifetime = value.toFloat();
		mOgreWidget->mEmitter->setParticleLifetime(value.toFloat());
	}
	// Now Emitter parameters
	else if (pName == "Rate") {
		mOgreWidget->mEmitterDescription.mRate = value.toFloat();
		mOgreWidget->mEmitter->setRate(value.toFloat());
	}
	else if (pName == "Type") {
		if (value.toInt() == 0)
			mOgreWidget->mEmitterDescription.mType = NxOgre::Enums::FluidEmitterType_FlowRate;
		else if (value.toInt() == 1)
			mOgreWidget->mEmitterDescription.mType = NxOgre::Enums::FluidEmitterType_Pressure;

		shouldRecreate = true;
	}
	else if (pName == "FluidSpeed") {
		mOgreWidget->mEmitterDescription.mFluidSpeed = value.toFloat();
		mOgreWidget->mEmitter->setFluidSpeed(value.toFloat());
	}

	if (shouldRecreate)
		mOgreWidget->createFluid();
}