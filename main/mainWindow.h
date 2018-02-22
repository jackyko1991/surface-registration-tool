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
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkProperty.h"

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
	void renderSource();
	
	QFutureWatcher<void>* m_watcher;
	DataIO* m_dataIO;
	vtkRenderer* m_renderer;
	vtkPolyDataMapper* m_sourceMapper;
	vtkPolyDataMapper* m_targetMapper;
	vtkActor* m_sourceActor;
	vtkActor* m_targetActor;
};

#endif // !MAINWINDOW_H
