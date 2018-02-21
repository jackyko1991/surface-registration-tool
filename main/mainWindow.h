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

#include "data_io.h"

#include <qDebug>
#include <chrono>
#include <thread>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QMainWindow *parent=nullptr);
	void setDataIO(DataIO*);

protected:

private slots :
	void browseSource();
	void browseTarget();
	void browseOutput();
	void initialTransformSet();
	void initialTransformValueChange();
	void execute();

signals:
	void loadDataComplete();
private:
	Ui::MainWindow ui;
	void initialTransformMatrixEnable(bool enable);
	void executeRun();
	
	QFutureWatcher<void>* m_watcher;
	DataIO* m_dataIO;

private slots:
	void executeComplete();
	void sourceFileReadStatusPrint(bool);
	void targetFileReadStatusPrint(bool);
};

#endif // !MAINWINDOW_H
