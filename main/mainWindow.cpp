#include "mainWindow.h"

MainWindow::MainWindow(QMainWindow *parent)
{
	ui.setupUi(this);

	// qvtk widget start
	m_renderer = vtkRenderer::New();

	ui.qvtkWidget->GetRenderWindow()->AddRenderer(m_renderer);

	m_sourceMapper = vtkPolyDataMapper::New();
	m_targetMapper = vtkPolyDataMapper::New();
	m_sourceActor = vtkActor::New();
	m_targetActor = vtkActor::New();

	m_sourceActor->SetMapper(m_sourceMapper);
	m_sourceActor->GetProperty()->SetColor(1, 0, 0);
	//m_sourceActor->GetProperty()->SetOpacity(0.7);

	m_targetActor->SetMapper(m_targetMapper);
	m_targetActor->GetProperty()->SetColor(0, 1, 0);
	//m_targetActor->GetProperty()->SetOpacity(0.7);

	m_renderer->AddActor(m_sourceActor);
	m_renderer->AddActor(m_targetActor);

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
		ui.initialTransformTableView->horizontalHeader()->resizeSection(i, 82);
		ui.initialTransformTableView->verticalHeader()->resizeSection(i, 25);
	}

	// Connect button signal slots
	connect(ui.sourcePushButton, SIGNAL(clicked()), this, SLOT(browseSource()));
	connect(ui.targetPushButton, SIGNAL(clicked()), this, SLOT(browseTarget()));
	connect(ui.outputPushButton, SIGNAL(clicked()), this, SLOT(browseOutput()));
	connect(ui.initialTransformComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(initialTransformSet()));
	connect(ui.initialTransformTableView->model(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(initialTransformValueChange()));
	connect(ui.rotateXSlider, SIGNAL(valueChanged(int)), this, SLOT(rotateXSliderValueChange(int)));
	connect(ui.rotateYSlider, SIGNAL(valueChanged(int)), this, SLOT(rotateYSliderValueChange(int)));
	connect(ui.rotateZSlider, SIGNAL(valueChanged(int)), this, SLOT(rotateZSliderValueChange(int)));
	connect(ui.translateXSlider, SIGNAL(valueChanged(int)), this, SLOT(translateXSliderValueChange(int)));
	connect(ui.translateYSlider, SIGNAL(valueChanged(int)), this, SLOT(translateYSliderValueChange(int)));
	connect(ui.translateZSlider, SIGNAL(valueChanged(int)), this, SLOT(translateZSliderValueChange(int)));
	connect(ui.rotateXDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(rotateXSpinBoxValueChange(double)));
	connect(ui.rotateYDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(rotateYSpinBoxValueChange(double)));
	connect(ui.rotateZDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(rotateZSpinBoxValueChange(double)));
	connect(ui.translateXDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(translateXSpinBoxValueChange(double)));
	connect(ui.translateYDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(translateYSpinBoxValueChange(double)));
	connect(ui.translateZDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(translateZSpinBoxValueChange(double)));

	connect(ui.executePushButton, SIGNAL(clicked()), this, SLOT(execute()));
}

void MainWindow::browseSource()
{
	QString sourceFile = QFileDialog::getOpenFileName(this,
		tr("Browse Source"), ui.sourcePlainTextEdit->toPlainText(), tr("Surface Files (*.stl *.vtp)"));

	if (!sourceFile.isNull())
	{
		ui.sourcePlainTextEdit->setPlainText(sourceFile);
		m_dataIO->SetSourcePath(ui.sourcePlainTextEdit->toPlainText());

		// connect read file signal slots
		connect(m_dataIO, SIGNAL(sourceFileReadStatus(bool)), this, SLOT(sourceFileReadStatusPrint(bool)));

		bool readStatus = m_dataIO->ReadSource();

		if (!readStatus)
		{
			this->renderSource();
			m_renderer->ResetCamera();
		}
	}
}

MainWindow::~MainWindow()
{
	m_renderer->Delete();
	m_sourceMapper->Delete();
	m_targetMapper->Delete();
	m_sourceActor->Delete();
	m_targetActor->Delete();
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
		ui.initialTransformTableView->setEnabled(true);
	}
	else
	{
		// set spinbox to non editable
		ui.initialTransformTableView->setEnabled(false);
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
	if (ui.initialTransformComboBox->currentText() == "Identity")
		ui.initialTransformComboBox->setCurrentText("User Matrix");

	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 4; column++)
		{
			QModelIndex index = ui.initialTransformTableView->model()->index(row, column, QModelIndex());
			m_dataIO->GetInitialTransform()->SetElement(row, column, ui.initialTransformTableView->model()->data(index).value<double>());
		}
	}
	
	this->renderSource();
}

void MainWindow::execute()
{
	//// lock ui
	//ui.sourcePlainTextEdit->setEnabled(false);
	//ui.sourcePushButton->setEnabled(false);
	//ui.targetPlainTextEdit->setEnabled(false);
	//ui.targetPushButton->setEnabled(false);
	//ui.outputPlainTextEdit->setEnabled(false);
	//ui.outputPushButton->setEnabled(false);
	//ui.initialTransformComboBox->setEnabled(false);
	//ui.initialTransformTableView->setEnabled(false);
	//ui.tfmCheckBox->setEnabled(false);
	//ui.stlCheckBox->setEnabled(false);
	//ui.vtpCheckBox->setEnabled(false);
	//ui.executePushButton->setEnabled(false);

	//// clear log
	//ui.textBrowser->clear();

	//// reset progress bar
	//ui.progressBar->reset();

	//// connect execute signal slots
	//connect(m_dataIO, SIGNAL(sourceFileReadStatus(bool)), this, SLOT(sourceFileReadStatusPrint(bool)));
	//connect(m_dataIO, SIGNAL(targetFileReadStatus(bool)), this, SLOT(targetFileReadStatusPrint(bool)));

	//// Instantiate the watcher to unlock
	//m_watcher = new QFutureWatcher<void>;
	//connect(m_watcher, SIGNAL(finished()), this, SLOT(executeComplete()));

	//// use QtConcurrent to run the registration on a new thread;
	//QFuture<void> future = QtConcurrent::run(this,&MainWindow::executeRun);
	//m_watcher->setFuture(future);
}

void MainWindow::executeRun()
{
	//qDebug() << "Start load data...";

	//m_dataIO->SetSourcePath(ui.sourcePlainTextEdit->toPlainText());
	//m_dataIO->SetTargetPath(ui.targetPlainTextEdit->toPlainText());
	//bool readStatus = m_dataIO->Read();
	//if (readStatus)
	//{
	//	qDebug() << "read file fail";
	//	return;
	//}

	//qDebug() << "read file success";

	//// registration
	//SurfaceRegistration surfaceReg;
	//surfaceReg.SetInitialTransformType(SurfaceRegistration::InitialTransformEnum(ui.initialTransformComboBox->currentIndex()));
	//surfaceReg.Update();

}

void MainWindow::renderSource()
{
	if (!(m_dataIO->GetSourceSurface()->GetNumberOfCells() > 0 ||
		m_dataIO->GetSourceSurface()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_dataIO->GetInitialTransform()->Print(std::cout);
	auto transformMat = m_dataIO->GetInitialTransform();
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->SetMatrix(transformMat);
	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(transform);
	transformFilter->SetInputData(m_dataIO->GetSourceSurface());

	transformFilter->Update();
	m_sourceMapper->SetInputData(transformFilter->GetOutput());
	//std::cout <<
	//	transformFilter->GetOutput()->GetPoint(0)[0] << "," <<
	//	transformFilter->GetOutput()->GetPoint(0)[1] << "," <<
	//	transformFilter->GetOutput()->GetPoint(0)[2] << std::endl;

	ui.qvtkWidget->update();
}

void MainWindow::UpdateMatrixFromTransformWidgets()
{
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	std::cout
		<< m_dataIO->GetSourceCentroid()[0] << ","
		<< m_dataIO->GetSourceCentroid()[1] << ","
		<< m_dataIO->GetSourceCentroid()[2] << std::endl;
}

void MainWindow::sourceFileReadStatusPrint(bool status)
{
	if (status)
	{
		ui.textBrowser->append("Source surface read fail");
		ui.textBrowser->append(m_dataIO->GetErrorMessage().c_str());
	}
	else
	{
		ui.textBrowser->append("Source surface read success");
	}
}

void MainWindow::targetFileReadStatusPrint(bool status)
{
	if (status)
	{
		ui.textBrowser->append("Target surface read fail");
		ui.textBrowser->append(m_dataIO->GetErrorMessage().c_str());
	}
	else
	{
		ui.textBrowser->append("Target surface read success");
	}
}

void MainWindow::rotateXSliderValueChange(int value)
{
	ui.rotateXDoubleSpinBox->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::executeComplete()
{
	// disconnect related signal slots
	disconnect(m_dataIO, SIGNAL(sourceFileReadStatus(bool)), this, SLOT(sourceFileReadStatusPrint(bool)));
	disconnect(m_dataIO, SIGNAL(targetFileReadStatus(bool)), this, SLOT(targetFileReadStatusPrint(bool)));

	qDebug() << "Execute complete";

	// unlock ui
	ui.sourcePlainTextEdit->setEnabled(true);
	ui.sourcePushButton->setEnabled(true);
	ui.targetPlainTextEdit->setEnabled(true);
	ui.targetPushButton->setEnabled(true);
	ui.outputPlainTextEdit->setEnabled(true);
	ui.outputPushButton->setEnabled(true);
	ui.initialTransformComboBox->setEnabled(true);
	ui.initialTransformTableView->setEnabled(true);
	ui.tfmCheckBox->setEnabled(true);
	ui.stlCheckBox->setEnabled(true);
	ui.vtpCheckBox->setEnabled(true);
	ui.executePushButton->setEnabled(true);
	delete m_watcher;
}

void MainWindow::rotateXSpinBoxValueChange(double value)
{
	ui.rotateXSlider->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}
