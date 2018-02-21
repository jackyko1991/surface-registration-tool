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

#include <iostream>

#include <qDebug>
#include <chrono>
#include <thread>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QMainWindow *parent=nullptr);

protected:

private slots :
	void browseSource();
	void browseTarget();
	void browseOutput();
	void initialTransformSet();
	void initialTransformValueChange();
	void execute();
signals:
	void executeCompleted();
private:
	Ui::MainWindow ui;
	void initialTransformMatrixEnable(bool enable);
	int executeRun();
	
	QFutureWatcher<int>* m_watcher;
private slots:
	void executeComplete();
};

#endif // !MAINWINDOW_H
