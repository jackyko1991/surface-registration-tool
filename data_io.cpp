#include "data_io.h"

DataIO::DataIO(QObject* parent)
{
	m_sourceSurface = vtkPolyData::New();
	m_targetSurface = vtkPolyData::New();
	m_outputSurface = vtkPolyData::New();
}

DataIO::~DataIO()
{
	m_sourceSurface->Delete();
	m_targetSurface->Delete();
	m_outputSurface->Delete();
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

vtkPolyData * DataIO::SetOutputSurface()
{
	return m_outputSurface;
}

bool DataIO::Read()
{
	// Return 0 for successful read and 1 for fail read

	// check file existence
	if (!(m_sourceFile.isFile()&&m_sourceFile.exists()))
	{
		emit sourceFileReadStatus(1);
		return 1;
	}

	if (!(m_targetFile.isFile() && m_targetFile.exists()))
	{
		emit targetFileReadStatus(1);
		return 1;
	}

	if (m_sourceFile.suffix() == "vtp" || m_sourceFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_sourceFile.absolutePath().toStdString().c_str());
		reader->Update();
		m_sourceSurface->DeepCopy(reader->GetOutput());
	}
	else if (m_sourceFile.suffix() == "stl" || m_sourceFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_sourceFile.absolutePath().toStdString().c_str());
		reader->Update();
		m_sourceSurface->DeepCopy(reader->GetOutput());
	}
	emit sourceFileReadStatus(0);

	if (m_targetFile.suffix() == "vtp" || m_targetFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_sourceFile.absolutePath().toStdString().c_str());
		reader->Update();
		m_targetSurface->DeepCopy(reader->GetOutput());
	}
	else if (m_targetFile.suffix() == "stl" || m_targetFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_sourceFile.absolutePath().toStdString().c_str());
		reader->Update();
		m_targetSurface->DeepCopy(reader->GetOutput());
	}
	emit targetFileReadStatus(0);

	return 0;
}