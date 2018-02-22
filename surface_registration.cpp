#include "surface_registration.h"

SurfaceRegistration::SurfaceRegistration(QObject* parent)
{

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
		std::cout << "user matrix" << std::endl;
		break;
	case identity:
		std::cout << "identity" << std::endl;
		break;
	case centroid:
		std::cout << "centroid" << std::endl;
		break;
	case PCA:
		std::cout << "PCA" << std::endl;
		break;
	}
}
