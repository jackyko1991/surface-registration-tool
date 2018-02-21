#include "mainWindow.h"

MainWindow::MainWindow(QMainWindow *parent)
{
	ui.setupUi(this);

	// initial transform matrix table
	QStandardItemModel* model = new QStandardItemModel(4, 4);
	ui.initialTransformTableView->setModel(model);

	SpinBoxDelegate* delegate = new SpinBoxDelegate();
	ui.initialTransformTableView->setItemDelegate(delegate);

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			QModelIndex index = model->index(row, column, QModelIndex());
			if (column == row)
				model->setData(index, QVariant(1.0));
			else
				model->setData(index, QVariant(0.0));
		}
	}

	for (int i = 0; i < 4; i++)
	{
		ui.initialTransformTableView->horizontalHeader()->resizeSection(i, 80);
		ui.initialTransformTableView->verticalHeader()->resizeSection(i, 25);
	}

	// Connect button signal slots
	connect(ui.sourcePushButton, SIGNAL(clicked()), this, SLOT(browseSource()));
	connect(ui.targetPushButton, SIGNAL(clicked()), this, SLOT(browseTarget()));
	connect(ui.outputPushButton, SIGNAL(clicked()), this, SLOT(browseOutput()));
	connect(ui.initialTransformComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(initialTransformSet()));
	connect(ui.initialTransformTableView->model(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(initialTransformValueChange()));
	connect(ui.executePushButton, SIGNAL(clicked()), this, SLOT(execute()));
}

void MainWindow::browseSource()
{
	QString sourceFile = QFileDialog::getOpenFileName(this,
		tr("Browse Source"), ui.sourcePlainTextEdit->toPlainText(), tr("Surface Files (*.stl *.vtp)"));

	if (!sourceFile.isNull())
		ui.sourcePlainTextEdit->setPlainText(sourceFile);
}

void MainWindow::setDataIO(DataIO* dataIO)
{
	m_dataIO = dataIO;
}

void MainWindow::browseTarget()
{
	QString targetFile = QFileDialog::getOpenFileName(this,
		tr("Browse Target"), ui.targetPlainTextEdit->toPlainText(), tr("Surface Files (*.stl *.vtp)"));

	if (!targetFile.isNull())
		ui.targetPlainTextEdit->setPlainText(targetFile);
}

void MainWindow::browseOutput()
{
	QString outputFolder = QFileDialog::getExistingDirectory(this,
		tr("Set Output Direcotry"), ui.outputPlainTextEdit->toPlainText());

	if (!outputFolder.isNull())
		ui.outputPlainTextEdit->setPlainText(outputFolder);
}

void MainWindow::initialTransformSet()
{
	if (ui.initialTransformComboBox->currentText() == "Identity" || 
		ui.initialTransformComboBox->currentText() == "User Matrix")
	{
		// set spinbox to editable
		this->initialTransformMatrixEnable(true);
	}
	else
	{
		// set spinbox to non editable
		this->initialTransformMatrixEnable(false);
	}

	if (ui.initialTransformComboBox->currentText() == "Identity")
	{
		for (int row = 0; row < 4; ++row) {
			for (int column = 0; column < 4; ++column) {
				QModelIndex index = ui.initialTransformTableView->model()->index(row, column, QModelIndex());
				if (column == row)
					ui.initialTransformTableView->model()->setData(index, QVariant(1.0));
				else
					ui.initialTransformTableView->model()->setData(index, QVariant(0.0));
			}
		}
	}
}

void MainWindow::initialTransformValueChange()
{
	ui.initialTransformComboBox->setCurrentText("User Matrix");
}

void MainWindow::execute()
{
	// lock the execute button
	ui.executePushButton->setEnabled(false);

	// connect execute signal slots
	connect(m_dataIO, SIGNAL(sourceFileReadStatus(bool)), this, SLOT(sourceFileReadStatusPrint(bool)));
	connect(m_dataIO, SIGNAL(targetFileReadStatus(bool)), this, SLOT(targetFileReadStatusPrint(bool)));

	// Instantiate the watcher to unlock
	m_watcher = new QFutureWatcher<void>;
	connect(m_watcher, SIGNAL(finished()), this, SLOT(executeComplete()));

	// use QtConcurrent to run the registration on a new thread;
	QFuture<void> future = QtConcurrent::run(this,&MainWindow::executeRun);
	m_watcher->setFuture(future);
}

void MainWindow::executeRun()
{
	qDebug() << "Start load data...";

	m_dataIO->SetSourcePath(ui.sourcePlainTextEdit->toPlainText());
	m_dataIO->SetTargetPath(ui.targetPlainTextEdit->toPlainText());
	bool readStatus = m_dataIO->Read();
	if (readStatus)
	{
		qDebug() << "read file fail";
		return;
	}
	else
	{
		qDebug() << "read file success";
		qDebug() << "Finish from thread" << QThread::currentThread();
	}
}

void MainWindow::sourceFileReadStatusPrint(bool status)
{
	if (status)
		ui.textBrowser->append("Source file read fail");
	else
		ui.textBrowser->append("Source file read success");
}

void MainWindow::targetFileReadStatusPrint(bool status)
{
	if (status)
		ui.textBrowser->append("Target file read fail");
	else
		ui.textBrowser->append("Target file read success");
}

void MainWindow::executeComplete()
{
	// disconnect related signal slots
	disconnect(m_dataIO, SIGNAL(sourceFileReadStatus(bool)), this, SLOT(sourceFileReadStatusPrint(bool)));
	disconnect(m_dataIO, SIGNAL(targetFileReadStatus(bool)), this, SLOT(targetFileReadStatusPrint(bool)));

	qDebug() << "Execute complete";
	ui.executePushButton->setEnabled(true);
	delete m_watcher;
}

void MainWindow::initialTransformMatrixEnable(bool enable)
{
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			// set spinbox to non editable
			QStandardItemModel* model = (QStandardItemModel*)ui.initialTransformTableView->model();
			model->item(row, column)->setEnabled(enable);
		}
	}
}

