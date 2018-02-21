#ifndef DATA_IO_H
#define DATA_IO_H

#include <QObject>
#include <QString>

class DataIO : public QObject
{
	Q_OBJECT

public:
	explicit DataIO(QObject* parent = 0);
	~DataIO();

	void SetSourcePath(QString);
	void SetTargetPath(QString);
	void SetOutputDir(QString);
	void WriteTransformedSTL(bool);
	void WriteTransformedVTP(bool);
	void WriteTransformMatrix(bool);
	void Read();
	void Write();

public slots:

signals:

private:
	QString m_sourcePath;
	QString m_targetPath;
	QString m_outputDir;
	bool m_writeTransformedSTL = false;
	bool m_writeTransformedVTP = false;
	bool m_writeTransformedMatrix = false;
	bool checkExistence(QString);

};

#endif // !DATA_IO_H
