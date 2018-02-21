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

class DataIO : public QObject
{
	Q_OBJECT

public:
	explicit DataIO(QObject* parent = 0);
	~DataIO();

	void SetSourcePath(QString);
	void SetTargetPath(QString);
	void SetOutputDir(QString);
	vtkPolyData* GetSourceSurface();
	vtkPolyData* GetTargetSurface();
	vtkPolyData* SetOutputSurface();
	void WriteTransformedSTL(bool);
	void WriteTransformedVTP(bool);
	void WriteTransformMatrix(bool);
	bool Read();
	void Write();

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

};

#endif // !DATA_IO_H
