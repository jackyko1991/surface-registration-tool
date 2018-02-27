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
#include "vtkPCA_ICP_Transform.h"
#include "vtkLandmarkTransform.h"
#include "vtkITKIterativeCloestPoint.h"

class SurfaceRegistration : public QObject
{
	Q_OBJECT

public:
	enum RegistrationMethodEnum { ICP, PCAICP, ITKICP };
	explicit SurfaceRegistration(QObject* parent = 0);
	~SurfaceRegistration();

	void SetDataIO(DataIO*);
	void SetRegistrationMethod(RegistrationMethodEnum);
	void SetMaximumIterationSteps(int);
	void Update();

public slots:

signals:
	void InitialTransformComplete();
	void RegistrationComplete();

private:
	RegistrationMethodEnum m_registrationMethodEnum;
	DataIO* m_dataIO;
	void ICPRegistration(vtkPolyData* source);
	void PCAICPRegistration(vtkPolyData* source);
	void ITKICPRegistration(vtkPolyData* source);
	int m_maxIterSteps;
};

#endif // !SURFACE_REGISTRATION
