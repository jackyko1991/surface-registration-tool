#ifndef SURFACE_REGISTRATION_H
#define SURFACE_REGISTRATION_H

#include <QObject>
#include <vtkPolyData.h>
#include "data_io.h"
#include "vtkTransform.h"
#include "vtkIterativeClosestPointTransform.h"
#include "vtkSmartPointer.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkMatrix4x4.h"

class SurfaceRegistration : public QObject
{
	Q_OBJECT

public:
	enum InitialTransformEnum { user_matrix, PCA };
	explicit SurfaceRegistration(QObject* parent = 0);
	~SurfaceRegistration();

	void SetDataIO(DataIO*);
	void SetInitialTransformType(InitialTransformEnum);
	void SetMaximumIterationSteps(int);
	void Update();

public slots:

signals:
	void InitialTransformComplete();
	void IcpTransformComplete();

private:
	InitialTransformEnum m_initialTransformEnum;
	DataIO* m_dataIO;
	void UserMatrixRegistration();
	void PCARegistration();
	int m_maxIterSteps;
};

#endif // !SURFACE_REGISTRATION
