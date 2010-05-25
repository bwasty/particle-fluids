#ifndef PARTICLEFLUIDS_H
#define PARTICLEFLUIDS_H

#include <QtGui/QMainWindow>
#include "ui_particlefluids.h"

class OgreWidget;

class QtProperty;
class QVariant;
class QDebugStream;

class ParticleFluids : public QMainWindow
{
	Q_OBJECT

public:
	ParticleFluids(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ParticleFluids();

	void setupPhysXGUI();

public slots:
	void propertyValueChanged(QtProperty* property, const QVariant & value);

private:
	Ui::ParticleFluidsClass ui;
	OgreWidget* mOgreWidget;
	QDebugStream* mCout;
	QDebugStream* mCerr;

public:
	QLabel* mLabelParticleCount;
	QLabel* mLabelCamPos;
};

#endif // PARTICLEFLUIDS_H
