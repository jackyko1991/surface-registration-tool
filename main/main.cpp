#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "mainWindow.h"
#include "data_io.h"
#include <iostream>

int main(int argc, char * argv[])
{
	//Q_INIT_RESOURCE(application);

	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("Sucabot MedTech");
	QCoreApplication::setApplicationName("Surface Registration Tool");
	QCoreApplication::setApplicationVersion("v1.0b");

	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::applicationName());
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("source", "Path to source surface");
	parser.addPositionalArgument("target", "Path to target surface");
	parser.addPositionalArgument("output", "Directory for output");
	
	parser.addOption(QCommandLineOption("transform", "Save registration transform matrix (default = true)", "bool"));
	parser.addOption(QCommandLineOption("stl", "Save registerted surface as STL (default = true)", "bool"));
	parser.addOption(QCommandLineOption("vtp", "Save registerted surface as STL (default = false)", "bool"));

	parser.process(app);

	// data_io
	DataIO* dataIO = new DataIO;

	if (!parser.positionalArguments().isEmpty())
	{
		std::cout << "Commandline mode under development" << std::endl;
		return 0;
	}
	else
	{
		// Start mainwindow
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

		MainWindow mainWin;
		mainWin.setDataIO(dataIO);

		mainWin.show();
		return app.exec();
	}

	delete dataIO;
}