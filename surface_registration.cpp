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

void SurfaceRegistration::SetInitialTransformType(InitialTransformEnum initialTransformEnum)
{
	m_initialTransformEnum = initialTransformEnum;
}

void SurfaceRegistration::Update()
{
	switch (m_initialTransformEnum)
	{
	case user_matrix:
		this->UserMatrixRegistration();
		break;
	case PCA:
		std::cout << "PCA" << std::endl;
		break;
	}
}

void SurfaceRegistration::UserMatrixRegistration()
{
	std::cout << "user matrix" << std::endl;
	m_dataIO->GetInitialTransform()->Print(std::cout);
	vtkSmartPointer<vtkTransform> initTransform = vtkSmartPointer<vtkTransform>::New();
	initTransform->Identity();
	initTransform->SetMatrix(m_dataIO->GetInitialTransform());

	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter->SetTransform(initTransform);
	transformFilter->SetInputData(m_dataIO->GetSourceSurface());
	transformFilter->Update();
	emit InitialTransformComplete();

	vtkSmartPointer<vtkIterativeClosestPointTransform> icpTransform = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
	icpTransform->SetSource(transformFilter->GetOutput());
	icpTransform->SetTarget(m_dataIO->GetTargetSurface());
	icpTransform->SetMaximumNumberOfIterations(m_maxIterSteps);
	icpTransform->Update();

	std::cout << "registration matrix" << std::endl;
	icpTransform->GetMatrix()->Print(std::cout);

	m_dataIO->GetRegistartionTransform()->DeepCopy(icpTransform->GetMatrix());

	emit IcpTransformComplete();
}

void SurfaceRegistration::SetMaximumIterationSteps(int maxIterSteps)
{
	m_maxIterSteps = maxIterSteps;
}