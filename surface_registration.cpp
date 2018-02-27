#include "surface_registration.h"

SurfaceRegistration::SurfaceRegistration(QObject* parent)
{
	m_maxIterSteps = 100;
}

SurfaceRegistration::~SurfaceRegistration()
{

}

void SurfaceRegistration::SetDataIO(DataIO *dataIO)
{
	m_dataIO = dataIO;
}

void SurfaceRegistration::SetRegistrationMethod(RegistrationMethodEnum registrationMethodEnum)
{
	m_registrationMethodEnum = registrationMethodEnum;
}

void SurfaceRegistration::Update()
{
	// perform initial transform
	vtkSmartPointer<vtkTransform> initTransform = vtkSmartPointer<vtkTransform>::New();
	initTransform->Identity();
	initTransform->SetMatrix(m_dataIO->GetInitialTransform());

	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(initTransform);
	transformFilter->SetInputData(m_dataIO->GetSourceSurface());
	transformFilter->Update();
	emit InitialTransformComplete();

	switch (m_registrationMethodEnum)
	{
	case ICP:
		this->ICPRegistration(transformFilter->GetOutput());
		break;
	case PCAICP:
		this->PCAICPRegistration(transformFilter->GetOutput());
		break;
	}
}

void SurfaceRegistration::ICPRegistration(vtkPolyData* source)
{
	vtkSmartPointer<vtkIterativeClosestPointTransform> transform = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
	transform->SetSource(source);
	transform->SetTarget(m_dataIO->GetTargetSurface());
	transform->SetMaximumNumberOfIterations(m_maxIterSteps);
	transform->GetLandmarkTransform()->SetModeToRigidBody();
	transform->Update();

	m_dataIO->GetRegistartionTransform()->DeepCopy(transform->GetMatrix());

	emit RegistrationComplete();
}

void SurfaceRegistration::SetMaximumIterationSteps(int maxIterSteps)
{
	m_maxIterSteps = maxIterSteps;
}

void SurfaceRegistration::PCAICPRegistration(vtkPolyData* source)
{
	vtkSmartPointer<vtkPCA_ICP_Transform> transform = vtkSmartPointer<vtkPCA_ICP_Transform>::New();
	transform->SetSource(source);
	transform->SetTarget(m_dataIO->GetTargetSurface());
	transform->SetMaximumNumberOfIterations(m_maxIterSteps);
	transform->GetLandmarkTransform()->SetModeToRigidBody();
	transform->Update();

	m_dataIO->GetRegistartionTransform()->DeepCopy(transform->GetMatrix());

	emit RegistrationComplete();

}