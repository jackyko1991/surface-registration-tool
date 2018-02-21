#ifndef SURFACE_REGISTRATION_H
#define SURFACE_REGISTRATION_H

#include <QObject>
#include <vtkPolyData.h>


class SurfaceRegistration : public QObject
{
	Q_OBJECT

public:
	explicit SurfaceRegistration(QObject* parent = 0);
	~SurfaceRegistration();

	void SetSource();
	void Update();

public slots:

signals:

private:

};

#endif // !SURFACE_REGISTRATION
