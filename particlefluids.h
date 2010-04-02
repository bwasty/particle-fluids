#ifndef PARTICLEFLUIDS_H
#define PARTICLEFLUIDS_H

#include <QtGui/QMainWindow>
#include "ui_particlefluids.h"

class ParticleFluids : public QMainWindow
{
	Q_OBJECT

public:
	ParticleFluids(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ParticleFluids();

private:
	Ui::ParticleFluidsClass ui;
};

#endif // PARTICLEFLUIDS_H
