#ifndef SURFACE_REGISTRATION_H
#define SURFACE_REGISTRATION_H

#include <QObject>
#include <vtkPolyData.h>
#include "data_io.h"


class SurfaceRegistration : public QObject
{
	Q_OBJECT

public:
	enum InitialTransformEnum { user_matrix, identity, centroid, PCA };
	explicit SurfaceRegistration(QObject* parent = 0);
	~SurfaceRegistration();

	void SetDataIO(DataIO*);
	void SetInitialTransformType(InitialTransformEnum);
	void Update();

public slots:

signals:

private:
	InitialTransformEnum m_initialTransformEnum;
	DataIO* m_dataIO;
};

#endif // !SURFACE_REGISTRATION
