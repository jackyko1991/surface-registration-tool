#include "data_io.h"

DataIO::DataIO(QObject* parent)
{
	m_sourceSurface = vtkPolyData::New();
	m_targetSurface = vtkPolyData::New();
	m_outputSurface = vtkPolyData::New();
	m_initialTransform = vtkMatrix4x4::New();
	m_registrationTransform = vtkMatrix4x4::New();

	m_sourceCentroid = new double[3];
	m_targetCentroid = new double[3];

	for (int i = 0; i < 3; i++)
	{
		m_sourceCentroid[i] = 0;
		m_targetCentroid[i] = 0;
	}
}

DataIO::~DataIO()
{
	m_sourceSurface->Delete();
	m_targetSurface->Delete();
	m_outputSurface->Delete();
	m_initialTransform->Delete();
	m_registrationTransform->Delete();
	delete m_sourceCentroid;
	delete m_targetCentroid;
}

void DataIO::SetSourcePath(QString sourcePath)
{
	m_sourceFile.setFile(sourcePath);
}

void DataIO::SetTargetPath(QString targetPath)
{
	m_targetFile.setFile(targetPath);
}

void DataIO::SetInitialTransformSavePath(QString transformSavePath)
{
	m_initialTransformSaveFile.setFile(transformSavePath);
}

void DataIO::SetRegistrationTransformSavePath(QString transformSavePath)
{
	m_registrationTransformSaveFile.setFile(transformSavePath);
}

void DataIO::SetTransformedSurfaceSavePath(QString tranformedSurfaceSavePath)
{
	m_transformedSurfaceSaveFile.setFile(tranformedSurfaceSavePath);
}

vtkPolyData * DataIO::GetSourceSurface()
{
	return m_sourceSurface;
}

vtkPolyData * DataIO::GetTargetSurface()
{
	return m_targetSurface;
}

vtkPolyData * DataIO::GetOutputSurface()
{
	return m_outputSurface;
}

double* DataIO::GetSourceCentroid()
{
	return m_sourceCentroid;
}

double * DataIO::GetTargetCentroid()
{
	return m_targetCentroid;
}

bool DataIO::WriteTransformedSurface()
{
	std::cout << m_transformedSurfaceSaveFile.absoluteFilePath().toStdString() << std::endl;

	// Return 0 for successful read and 1 for fail read
	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	// initial transform
	vtkSmartPointer<vtkMatrix4x4> finalRegistrationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkMatrix4x4::Multiply4x4(m_initialTransform, m_registrationTransform, finalRegistrationMatrix);

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->PostMultiply();
	transform->SetMatrix(finalRegistrationMatrix);

	m_initialTransform->Print(std::cout);
	m_registrationTransform->Print(std::cout);

	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(transform);
	transformFilter->SetInputData(m_sourceSurface);
	transformFilter->AddObserver(vtkCommand::ErrorEvent, errorObserver);
	transformFilter->Update();

	if (errorObserver->GetError())
	{
		m_errorMessage = errorObserver->GetErrorMessage();
		emit transformedFileSaveStatus(1);
		return 1;
	}

	// actually save the file
	if (m_transformedSurfaceSaveFile.suffix() == "vtp" || m_transformedSurfaceSaveFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
		writer->SetFileName(m_transformedSurfaceSaveFile.absoluteFilePath().toStdString().c_str());
		writer->SetInputData(transformFilter->GetOutput());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit transformedFileSaveStatus(1);
			return 1;
		}
	}
	else if (m_transformedSurfaceSaveFile.suffix() == "stl" || m_transformedSurfaceSaveFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
		writer->SetFileName(m_transformedSurfaceSaveFile.absoluteFilePath().toStdString().c_str());
		writer->SetInputData(transformFilter->GetOutput());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit transformedFileSaveStatus(1);
			return 1;
		}
	}
	else if (m_transformedSurfaceSaveFile.suffix() == "vtk" || m_transformedSurfaceSaveFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
		writer->SetFileName(m_transformedSurfaceSaveFile.absoluteFilePath().toStdString().c_str());
		writer->SetInputData(transformFilter->GetOutput());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit transformedFileSaveStatus(1);
			return 1;
		}
	}
	else
	{
		m_errorMessage = "Invalid data type";
		emit transformedFileSaveStatus(1);
		return 1;
	}

	emit transformedFileSaveStatus(0);

	return 0;
}

bool DataIO::ReadTarget()
{
	if (!(m_targetFile.isFile() && m_targetFile.exists()))
	{
		m_errorMessage = "Target surface not exist";
		emit targetFileReadStatus(1);
		return 1;
	}

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_targetFile.suffix() == "vtp" || m_targetFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_targetFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit targetFileReadStatus(1);
			return 1;
		}
		else
		{
			m_targetSurface->DeepCopy(reader->GetOutput());
			emit targetFileReadStatus(0);
		}
	}
	else if (m_targetFile.suffix() == "stl" || m_targetFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_targetFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit targetFileReadStatus(1);
			return 1;
		}
		else
		{
			m_targetSurface->DeepCopy(reader->GetOutput());
			emit targetFileReadStatus(0);
		}
	}
	else if (m_targetFile.suffix() == "vtk" || m_targetFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_targetFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit targetFileReadStatus(1);
			return 1;
		}
		else
		{
			m_targetSurface->DeepCopy(reader->GetOutput());
			emit targetFileReadStatus(0);
		}
	}
	else
	{
		m_errorMessage = "Invalid data type";
		emit targetFileReadStatus(1);
		return 1;
	}
	
	// calculate source surface centroid
	vtkSmartPointer<vtkCenterOfMass> comFilter = vtkSmartPointer<vtkCenterOfMass>::New();
	comFilter->SetInputData(m_targetSurface);
	comFilter->Update();
	m_targetCentroid[0] = comFilter->GetCenter()[0];
	m_targetCentroid[1] = comFilter->GetCenter()[1];
	m_targetCentroid[2] = comFilter->GetCenter()[2];

	return 0;
}

vtkMatrix4x4 * DataIO::GetInitialTransform()
{
	return m_initialTransform;
}

vtkMatrix4x4 * DataIO::GetRegistartionTransform()
{
	return m_registrationTransform;
}

bool DataIO::ReadSource()
{
	// Return 0 for successful read and 1 for fail read
	// check file existence
	if (!(m_sourceFile.isFile() && m_sourceFile.exists()))
	{
		m_errorMessage = "Source surface not exist";
		emit sourceFileReadStatus(1);
		return 1;
	}

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_sourceFile.suffix() == "vtp" || m_sourceFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_sourceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit sourceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_sourceSurface->DeepCopy(reader->GetOutput());
			emit sourceFileReadStatus(0);
		}
	}
	else if (m_sourceFile.suffix() == "stl" || m_sourceFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_sourceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit sourceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_sourceSurface->DeepCopy(reader->GetOutput());
			emit sourceFileReadStatus(0);
		}
	}
	else if (m_sourceFile.suffix() == "vtk" || m_sourceFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_sourceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();

		if (errorObserver->GetError())
		{
			m_errorMessage = errorObserver->GetErrorMessage();
			emit sourceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_sourceSurface->DeepCopy(reader->GetOutput());
			emit sourceFileReadStatus(0);
		}
	}
	else
	{
		m_errorMessage = "Invalid data type";
		emit sourceFileReadStatus(1);
		return 1;
	}

	// calculate source surface centroid
	vtkSmartPointer<vtkCenterOfMass> comFilter = vtkSmartPointer<vtkCenterOfMass>::New();
	comFilter->SetInputData(m_sourceSurface);
	comFilter->Update();
	m_sourceCentroid[0] = comFilter->GetCenter()[0];
	m_sourceCentroid[1] = comFilter->GetCenter()[1];
	m_sourceCentroid[2] = comFilter->GetCenter()[2];

	return 0;
}

std::string DataIO::GetErrorMessage()
{
	return m_errorMessage;
}

void DataIO::ComputeSurfaceDistance(vtkPolyData * source, vtkPolyData * target)
{
	vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
	distanceFilter->SetInputData(0, source);
	distanceFilter->SetInputData(1, target);
	std::cout << "distance filter start" << std::endl;
	distanceFilter->Update();
	std::cout << "distance filter finish" << std::endl;
	source->DeepCopy(distanceFilter->GetOutput());
}

void DataIO::WriteInitialTransform()
{
	QFile data(m_initialTransformSaveFile.absoluteFilePath());
	if (data.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream output(&data);
		for (int row = 0; row < 4; row++)
		{
			for (int column = 0; column < 4; column++)
			{
				if (column == 3)
					output << m_initialTransform->GetElement(row, column) << "\r\n";
				else
					output << m_initialTransform->GetElement(row, column) << ",";
			}
		}
	}
}

void DataIO::WriteRegistrationTransform()
{
	QFile data(m_registrationTransformSaveFile.absoluteFilePath());
	if (data.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream output(&data);
		for (int row = 0; row < 4; row++)
		{
			for (int column = 0; column < 4; column++)
			{
				if (column == 3)
					output << m_registrationTransform->GetElement(row, column) << "\r\n";
				else
					output << m_registrationTransform->GetElement(row, column) << ",";
			}
		}
	}
}