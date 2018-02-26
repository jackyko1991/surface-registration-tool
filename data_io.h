#ifndef DATA_IO_H
#define DATA_IO_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkMatrix4x4.h>
#include "observe_error.h"
#include <vtkCenterOfMass.h>
#include "vtkDistancePolyDataFilter.h"

class DataIO : public QObject
{
	Q_OBJECT

public:
	explicit DataIO(QObject* parent = 0);
	~DataIO();
	static void ComputeSurfaceDistance(vtkPolyData* source, vtkPolyData* target);

	void SetSourcePath(QString);
	void SetTargetPath(QString);
	void SetOutputDir(QString);
	vtkPolyData* GetSourceSurface();
	vtkPolyData* GetTargetSurface();
	vtkPolyData* GetOutputSurface();
	double* GetSourceCentroid();
	double* GetTargetCentroid();
	void SetOutputSurface(vtkPolyData*);
	void WriteTransformedSTL(bool);
	void WriteTransformedVTP(bool);
	void WriteTransformMatrix(bool);
	vtkMatrix4x4* GetInitialTransform();
	vtkMatrix4x4* GetRegistartionTransform();
	bool ReadSource();
	bool ReadTarget();
	void Write();
	std::string GetErrorMessage();

public slots:

signals:
	// 0 for success, 1 for fail
	void sourceFileReadStatus(bool); 
	void targetFileReadStatus(bool);

private:
	QFileInfo m_sourceFile;
	QFileInfo m_targetFile;
	QFileInfo m_outputDir;
	bool m_writeTransformedSTL = false;
	bool m_writeTransformedVTP = false;
	bool m_writeTransformedMatrix = false;
	vtkPolyData* m_sourceSurface;
	vtkPolyData* m_targetSurface;
	vtkPolyData* m_outputSurface;
	std::string m_errorMessage;
	vtkMatrix4x4* m_initialTransform;
	vtkMatrix4x4* m_registrationTransform;
	double* m_sourceCentroid;
	double* m_targetCentroid;
};

#endif // !DATA_IO_H
