#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_surface_registration_tool.h"
#include <QMainWindow>
#include <QDir>
#include <QFileDialog>
#include <QTableView>
#include <QStandardItemModel>
#include "delegate.h"
#include <QDoubleSpinBox>
#include <QFuture>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QFutureWatcher>
#include <QVTKWidget.h>

#include <iostream>
#include "data_io.h"
#include "surface_registration.h"

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

#include <qDebug>
#include <chrono>
#include <thread>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QMainWindow *parent=nullptr);
	~MainWindow();
	void setDataIO(DataIO*);

protected:

private slots :
	void browseSource();
	void browseTarget();
	void browseOutput();
	void initialTransformSet();
	void initialTransformValueChange();
	void execute();
	void executeComplete();
	void sourceFileReadStatusPrint(bool);
	void targetFileReadStatusPrint(bool);

signals:
	void loadDataComplete();
private:
	Ui::MainWindow ui;
	void executeRun();
	
	QFutureWatcher<void>* m_watcher;
	DataIO* m_dataIO;
	vtkRenderer* m_renderer;
};

#endif // !MAINWINDOW_H
