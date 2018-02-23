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

void DataIO::SetOutputDir(QString outputDir)
{
	m_outputDir.setFile(outputDir);
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

//bool DataIO::Read()
//{
//	if (!(m_targetFile.isFile() && m_targetFile.exists()))
//	{
//		m_errorMessage = "Target surface not exist";
//		emit targetFileReadStatus(1);
//		return 1;
//	}
//
//	
//
//	
//	
//	if (m_targetFile.suffix() == "vtp" || m_targetFile.suffix() == "VTP")
//	{
//		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
//		reader->SetFileName(m_sourceFile.absoluteFilePath().toStdString().c_str());
//		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
//		reader->Update();
//
//		if (errorObserver->GetError())
//		{
//			m_errorMessage = errorObserver->GetErrorMessage();
//			emit targetFileReadStatus(1);
//			return 1;
//		}
//		else
//		{
//			m_sourceSurface->DeepCopy(reader->GetOutput());
//			emit targetFileReadStatus(0);
//		}
//	}
//	else if (m_targetFile.suffix() == "stl" || m_targetFile.suffix() == "STL")
//	{
//		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
//		reader->SetFileName(m_sourceFile.absoluteFilePath().toStdString().c_str());
//		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
//		reader->Update();
//		if (errorObserver->GetError())
//		{
//			m_errorMessage = errorObserver->GetErrorMessage();
//			emit targetFileReadStatus(1);
//			return 1;
//		}
//		else
//		{
//			m_sourceSurface->DeepCopy(reader->GetOutput());
//			emit targetFileReadStatus(0);
//		}
//	}
//	else
//	{
//		m_errorMessage = "Invalid data type";
//		emit targetFileReadStatus(1);
//		return 1;
//	}
//	
//
//	return 0;
//}

vtkMatrix4x4 * DataIO::GetInitialTransform()
{
	return m_initialTransform;
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
