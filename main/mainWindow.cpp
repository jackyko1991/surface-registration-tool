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
	m_sourceActor->GetProperty()->SetColor(1, 1, 1);

	m_targetActor->SetMapper(m_targetMapper);
	m_targetActor->GetProperty()->SetColor(1, 1, 1);

	m_renderer->AddActor(m_sourceActor);
	m_renderer->AddActor(m_targetActor);

	// mapper setting
	m_sourceMapper->SetScalarRange(5,5);

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
	connect(ui.resetPushButton, SIGNAL(clicked()), this, SLOT(resetCamera()));
	connect(ui.identityPushButton, SIGNAL(clicked()), this, SLOT(identityInitialTransform()));
	connect(ui.centroidPushButton, SIGNAL(clicked()), this, SLOT(centroidInitialTransform()));
	connect(ui.executePushButton, SIGNAL(clicked()), this, SLOT(execute()));
	connect(ui.sourceOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(sourceOpacityChange()));
	connect(ui.targetOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(targetOpacityChange()));
	connect(ui.clearPushButton, SIGNAL(clicked()), this, SLOT(clearLog()));
	connect(ui.inversePushButton, SIGNAL(clicked()), this, SLOT(inverseMatrix()));
}

void MainWindow::browseSource()
{
	QString sourceFile = QFileDialog::getOpenFileName(this,
		tr("Browse Source"), ui.sourcePlainTextEdit->toPlainText(), tr("Surface Files (*.stl *.vtp *.vtk)"));

	if (!sourceFile.isNull())
	{
		ui.sourcePlainTextEdit->setPlainText(sourceFile);
		m_dataIO->SetSourcePath(ui.sourcePlainTextEdit->toPlainText());

		// read status print out
		ui.textBrowser->append("Reading source file: " + ui.sourcePlainTextEdit->toPlainText());

		// connect read file signal slots
		connect(m_dataIO, SIGNAL(sourceFileReadStatus(bool)), this, SLOT(sourceFileReadStatusPrint(bool)));

		// lock ui
		enableUI(false);

		// Instantiate the watcher to unlock
		m_ioWatcher = new QFutureWatcher<bool>;
		connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readFileComplete()));

		// use QtConcurrent to run the read file on a new thread;
		QFuture<bool> future = QtConcurrent::run(m_dataIO, &DataIO::ReadSource);
		m_ioWatcher->setFuture(future);
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
		tr("Browse Target"), ui.targetPlainTextEdit->toPlainText(), tr("Surface Files (*.stl *.vtp *.vtk)"));

	if (!targetFile.isNull())
	{
		ui.targetPlainTextEdit->setPlainText(targetFile);
		m_dataIO->SetTargetPath(ui.targetPlainTextEdit->toPlainText());

		// read status print out
		ui.textBrowser->append("Reading target file: " + ui.targetPlainTextEdit->toPlainText());

		// connect read file signal slots
		connect(m_dataIO, SIGNAL(targetFileReadStatus(bool)), this, SLOT(targetFileReadStatusPrint(bool)));

		// lock ui
		enableUI(false);

		// Instantiate the watcher to unlock
		m_ioWatcher = new QFutureWatcher<bool>;
		connect(m_ioWatcher, SIGNAL(finished()), this, SLOT(readFileComplete()));

		// use QtConcurrent to run the read file on a new thread;
		QFuture<bool> future = QtConcurrent::run(m_dataIO, &DataIO::ReadTarget);
		m_ioWatcher->setFuture(future);
	}
}

void MainWindow::initialTransformValueChange()
{
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
	// check both source and target exists
	if (!(m_dataIO->GetSourceSurface()->GetNumberOfCells() > 0 &&
		m_dataIO->GetSourceSurface()->GetNumberOfPoints() > 0 &&
		m_dataIO->GetTargetSurface()->GetNumberOfCells() > 0 &&
		m_dataIO->GetTargetSurface()->GetNumberOfPoints() > 0))
	{
		return;
	}

	// lock ui
	enableUI(false);

	// reset progress bar
	ui.progressBar->setValue(0);

	// log start registration
	ui.textBrowser->append("Registration Starts...");
	ui.textBrowser->append("Registration Method: " + ui.methodComboBox->currentText());

	ui.textBrowser->append("Initial Transform:");
	ui.textBrowser->append(
		QString::number(m_dataIO->GetInitialTransform()->GetElement(0, 0)) + ", " + 
		QString::number(m_dataIO->GetInitialTransform()->GetElement(0, 1)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(0, 2)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(0, 3)));
	ui.textBrowser->append(
		QString::number(m_dataIO->GetInitialTransform()->GetElement(1, 0)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(1, 1)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(1, 2)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(1, 3)));
	ui.textBrowser->append(
		QString::number(m_dataIO->GetInitialTransform()->GetElement(2, 0)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(2, 1)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(2, 2)) + ", " +
		QString::number(m_dataIO->GetInitialTransform()->GetElement(2, 3)));

	// Instantiate the watcher to unlock
	m_executeWatcher = new QFutureWatcher<void>;
	connect(m_executeWatcher, SIGNAL(finished()), this, SLOT(executeComplete()));

	// use QtConcurrent to run the registration on a new thread;
	QFuture<void> future = QtConcurrent::run(this,&MainWindow::executeRun);
	m_executeWatcher->setFuture(future);
}

void MainWindow::executeRun()
{
	// registration
	SurfaceRegistration surfaceReg;
	surfaceReg.SetDataIO(m_dataIO);
	surfaceReg.SetMaximumIterationSteps(ui.maxIcpStepsSpinBox->value());
	surfaceReg.SetRegistrationMethod(SurfaceRegistration::RegistrationMethodEnum(ui.methodComboBox->currentIndex()));
	surfaceReg.Update();
}

void MainWindow::renderSource()
{
	if (!(m_dataIO->GetSourceSurface()->GetNumberOfCells() > 0 ||
		m_dataIO->GetSourceSurface()->GetNumberOfPoints() > 0))
	{
		return;
	}

	auto transformMat = m_dataIO->GetInitialTransform();
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->SetMatrix(transformMat);
	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(transform);
	transformFilter->SetInputData(m_dataIO->GetSourceSurface());
	transformFilter->Update();

	m_sourceMapper->SetInputData(transformFilter->GetOutput());

	ui.qvtkWidget->update();
}

void MainWindow::renderTarget()
{
	if (!(m_dataIO->GetTargetSurface()->GetNumberOfCells() > 0 ||
		m_dataIO->GetTargetSurface()->GetNumberOfPoints() > 0))
	{
		return;
	}

	m_targetMapper->SetInputData(m_dataIO->GetTargetSurface());

	ui.qvtkWidget->update();
}

void MainWindow::UpdateMatrixFromTransformWidgets()
{
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply();

	transform->Translate(ui.translateXDoubleSpinBox->value(), ui.translateYDoubleSpinBox->value(), ui.translateZDoubleSpinBox->value());
	transform->Translate(
		-(m_dataIO->GetSourceCentroid()[0] + ui.translateXDoubleSpinBox->value()), 
		-(m_dataIO->GetSourceCentroid()[1] + ui.translateYDoubleSpinBox->value()), 
		-(m_dataIO->GetSourceCentroid()[2] + ui.translateZDoubleSpinBox->value()));
	transform->RotateX(ui.rotateXDoubleSpinBox->value());
	transform->RotateY(ui.rotateYDoubleSpinBox->value());
	transform->RotateZ(ui.rotateZDoubleSpinBox->value());

	transform->Translate(
		m_dataIO->GetSourceCentroid()[0] + ui.translateXDoubleSpinBox->value(), 
		m_dataIO->GetSourceCentroid()[1] + ui.translateYDoubleSpinBox->value(),
		m_dataIO->GetSourceCentroid()[2] + ui.translateZDoubleSpinBox->value());

	// update ui matrix
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			QModelIndex index = ui.initialTransformTableView->model()->index(row, column, QModelIndex());
			ui.initialTransformTableView->model()->setData(index, QVariant(transform->GetMatrix()->GetElement(row,column)));
		}
	}

	// update database initial transform matrix
	m_dataIO->GetInitialTransform()->DeepCopy(transform->GetMatrix());
}

void MainWindow::sourceFileReadStatusPrint(bool status)
{
	if (status)
	{
		ui.textBrowser->append("Source file read fail");
		ui.textBrowser->append(m_dataIO->GetErrorMessage().c_str());
	}
	else
	{
		ui.textBrowser->append("Source file read success");
	}

	// disconnect related signal slots
	disconnect(m_dataIO, SIGNAL(sourceFileReadStatus(bool)), this, SLOT(sourceFileReadStatusPrint(bool)));
}

void MainWindow::targetFileReadStatusPrint(bool status)
{
	if (status)
	{
		ui.textBrowser->append("Target file read fail");
		ui.textBrowser->append(m_dataIO->GetErrorMessage().c_str());
	}
	else
	{
		ui.textBrowser->append("Target file read success");
	}

	// disconnect related signal slots
	disconnect(m_dataIO, SIGNAL(targetFileReadStatus(bool)), this, SLOT(targetFileReadStatusPrint(bool)));
}

void MainWindow::rotateXSliderValueChange(int value)
{
	ui.rotateXDoubleSpinBox->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::rotateYSliderValueChange(int value)
{
	ui.rotateYDoubleSpinBox->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::rotateZSliderValueChange(int value)
{
	ui.rotateZDoubleSpinBox->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::translateXSliderValueChange(int value)
{
	ui.translateXDoubleSpinBox->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::translateYSliderValueChange(int value)
{
	ui.translateYDoubleSpinBox->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::translateZSliderValueChange(int value)
{
	ui.translateZDoubleSpinBox->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::executeComplete()
{
	// log the transform
	ui.textBrowser->append("");
	ui.textBrowser->append("Registration Transform:");
	ui.textBrowser->append(
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(0, 0)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(0, 1)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(0, 2)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(0, 3)));
	ui.textBrowser->append(
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(1, 0)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(1, 1)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(1, 2)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(1, 3)));
	ui.textBrowser->append(
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(2, 0)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(2, 1)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(2, 2)) + ", " +
		QString::number(m_dataIO->GetRegistartionTransform()->GetElement(2, 3)));
	ui.textBrowser->append("===================================");

	// update matrix table
	vtkSmartPointer<vtkMatrix4x4> finalTransform = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkMatrix4x4::Multiply4x4(m_dataIO->GetRegistartionTransform(), m_dataIO->GetInitialTransform(), finalTransform);

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			QModelIndex index = ui.initialTransformTableView->model()->index(row, column, QModelIndex());
			ui.initialTransformTableView->model()->setData(index, QVariant(finalTransform->GetElement(row, column)));
		}
	}

	//// calcualte distance
	//

	//DataIO::ComputeSurfaceDistance(transformFilter->GetOutput(), m_dataIO->GetTargetSurface());
	//m_sourceMapper->SetColorModeToMapScalars();


	// unlock ui
	enableUI(true);

	delete m_executeWatcher;
}

void MainWindow::rotateXSpinBoxValueChange(double value)
{
	ui.rotateXSlider->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::rotateYSpinBoxValueChange(double value)
{
	ui.rotateYSlider->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::rotateZSpinBoxValueChange(double value)
{
	ui.rotateZSlider->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::translateXSpinBoxValueChange(double value)
{
	ui.translateXSlider->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::translateYSpinBoxValueChange(double value)
{
	ui.translateYSlider->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::translateZSpinBoxValueChange(double value)
{
	ui.translateZSlider->setValue(value);
	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::resetCamera()
{
	m_renderer->ResetCamera();
	ui.qvtkWidget->update();
}

void MainWindow::identityInitialTransform()
{
	ui.rotateXDoubleSpinBox->setValue(0);
	ui.rotateYDoubleSpinBox->setValue(0);
	ui.rotateZDoubleSpinBox->setValue(0);
	ui.translateXDoubleSpinBox->setValue(0);
	ui.translateYDoubleSpinBox->setValue(0);
	ui.translateZDoubleSpinBox->setValue(0);

	this->UpdateMatrixFromTransformWidgets();
}

void MainWindow::centroidInitialTransform()
{
	ui.translateXDoubleSpinBox->setValue(m_dataIO->GetTargetCentroid()[0]- m_dataIO->GetSourceCentroid()[0]);
	ui.translateYDoubleSpinBox->setValue(m_dataIO->GetTargetCentroid()[1] -m_dataIO->GetSourceCentroid()[1]);
	ui.translateZDoubleSpinBox->setValue(m_dataIO->GetTargetCentroid()[2] -m_dataIO->GetSourceCentroid()[2]);
}

void MainWindow::enableUserMatrix(bool enable)
{
	ui.initialTransformTableView->setEnabled(enable);
	ui.identityPushButton->setEnabled(enable);
	ui.centroidPushButton->setEnabled(enable);
	ui.inversePushButton->setEnabled(enable);
	ui.rotateXSlider->setEnabled(enable);
	ui.rotateYSlider->setEnabled(enable);
	ui.rotateZSlider->setEnabled(enable);
	ui.translateXSlider->setEnabled(enable);
	ui.translateYSlider->setEnabled(enable);
	ui.translateZSlider->setEnabled(enable);
	ui.rotateXDoubleSpinBox->setEnabled(enable);
	ui.rotateYDoubleSpinBox->setEnabled(enable);
	ui.rotateZDoubleSpinBox->setEnabled(enable);
	ui.translateXDoubleSpinBox->setEnabled(enable);
	ui.translateYDoubleSpinBox->setEnabled(enable);
	ui.translateZDoubleSpinBox->setEnabled(enable);
}

void MainWindow::enableUI(bool enable)
{
	ui.sourcePlainTextEdit->setEnabled(enable);
	ui.sourcePushButton->setEnabled(enable);
	ui.targetPlainTextEdit->setEnabled(enable);
	ui.targetPushButton->setEnabled(enable);
	ui.methodComboBox->setEnabled(enable);
	ui.initialTransformTableView->setEnabled(enable);
	ui.executePushButton->setEnabled(enable);
	ui.identityPushButton->setEnabled(enable);
	ui.centroidPushButton->setEnabled(enable);
	ui.inversePushButton->setEnabled(enable);
	ui.rotateXSlider->setEnabled(enable);
	ui.rotateYSlider->setEnabled(enable);
	ui.rotateZSlider->setEnabled(enable);
	ui.translateXSlider->setEnabled(enable);
	ui.translateYSlider->setEnabled(enable);
	ui.translateZSlider->setEnabled(enable);
	ui.rotateXDoubleSpinBox->setEnabled(enable);
	ui.rotateYDoubleSpinBox->setEnabled(enable);
	ui.rotateZDoubleSpinBox->setEnabled(enable);
	ui.translateXDoubleSpinBox->setEnabled(enable);
	ui.translateYDoubleSpinBox->setEnabled(enable);
	ui.translateZDoubleSpinBox->setEnabled(enable);
	ui.distancePushButton->setEnabled(enable);
	ui.clearPushButton->setEnabled(enable);
	ui.saveTransformPushButton->setEnabled(enable);
	ui.saveSurfacePushButton->setEnabled(enable);
	ui.maxIcpStepsSpinBox->setEnabled(enable);
}

void MainWindow::sourceOpacityChange()
{
	m_sourceActor->GetProperty()->SetOpacity(ui.sourceOpacitySlider->value()*1.0 / 100.0);
	ui.qvtkWidget->update();
}

void MainWindow::targetOpacityChange()
{
	m_targetActor->GetProperty()->SetOpacity(ui.targetOpacitySlider->value()*1.0 / 100.0);
	ui.qvtkWidget->update();
}

void MainWindow::clearLog()
{
	ui.textBrowser->clear();
}

void MainWindow::inverseMatrix()
{
	disconnect(ui.initialTransformTableView->model(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(initialTransformValueChange()));

	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 4; column++)
		{
			QModelIndex index = ui.initialTransformTableView->model()->index(row, column, QModelIndex());
			ui.initialTransformTableView->model()->setData(index, QVariant(m_dataIO->GetInitialTransform()->GetElement(row, column)));
		}
	}

	connect(ui.initialTransformTableView->model(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(initialTransformValueChange()));

	this->renderSource();
}

void MainWindow::readFileComplete()
{
	if (!m_ioWatcher->future().result())
	{
		this->renderSource();
		this->renderTarget();
		m_renderer->ResetCamera();
	}
	ui.textBrowser->append("===================================");

	// unlock ui
	enableUI(true);

	delete m_ioWatcher;
}