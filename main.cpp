#include "stdafx.h"
#include "particlefluids.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ParticleFluids w;
	w.show();
	return a.exec();
}
