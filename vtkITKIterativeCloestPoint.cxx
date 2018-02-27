// me 
#include "vtkITKIterativeCloestPoint.h"

// vtk
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkMatrix4x4.h>

vtkStandardNewMacro(vtkITKIterativeCloestPoint);
vtkCxxSetObjectMacro(vtkITKIterativeCloestPoint, Source, vtkPoints);
vtkCxxSetObjectMacro(vtkITKIterativeCloestPoint, Target, vtkPoints);
void vtkITKIterativeCloestPoint::PrintSelf(ostream & os, vtkIndent indent)
{
	Superclass::PrintSelf(os, indent);
}

void vtkITKIterativeCloestPoint::Inverse()
{
	std::swap(this->Source, this->Target);
	this->Modified();
}

vtkAbstractTransform * vtkITKIterativeCloestPoint::MakeTransform()
{
	return vtkITKIterativeCloestPoint::New();
}

vtkITKIterativeCloestPoint::vtkITKIterativeCloestPoint():
	NumberOfIterations(100),
	Mode(0),
	RMS(0),
	Source(nullptr),
	Target(nullptr)
{
	
}

vtkITKIterativeCloestPoint::~vtkITKIterativeCloestPoint()
{
}

void vtkITKIterativeCloestPoint::InternalDeepCopy(vtkAbstractTransform * transform)
{
	vtkITKIterativeCloestPoint* t = reinterpret_cast<vtkITKIterativeCloestPoint*>(transform);
	this->SetSource(t->GetSource());
	this->SetTarget(t->GetTarget());

	this->Modified();
}

void vtkITKIterativeCloestPoint::InternalUpdate()
{
  if (this->Source == NULL || this->Target == NULL)
  {
    this->Matrix->Identity();
    return;
  }
  this->ITK_Calculation();
}

// itk
#include <itkPointSet.h>
#include <itkEuler3DTransform.h>
#include <itkAffineTransform.h>
#include <itkEuclideanDistancePointMetric.h>
#include <itkIdentityTransform.h>
#include <itkEuclideanDistancePointSetToPointSetMetricv4.h>
#include <itkPointSetToPointSetRegistrationMethod.h>
#include <itkLevenbergMarquardtOptimizer.h>
// itk_vnl
#include <vnl_vector.h>
#include <vnl_matrix.h>
#include <vnl_rotation_matrix.h>
#include <vnl_inverse.h>
#include <vnl_symmetric_eigensystem.h>
#include <vnl_cross.h>

using namespace std;
typedef double CoordinateType;
const unsigned int DIMENSION = 3;
typedef itk::PointSet<CoordinateType, DIMENSION> PointSet;
typedef itk::EuclideanDistancePointSetToPointSetMetricv4<PointSet, PointSet> EuclideanDistancePointMetricv4;
typedef itk::EuclideanDistancePointMetric<PointSet, PointSet> EuclideanDistancePointMetric;
//template <typename TTransform>
//using TransformMeshFilter = itk::TransformMeshFilter<PointSet, PointSet, TTransform>;
typedef itk::PointSetToPointSetRegistrationMethod<PointSet, PointSet> PointSetToPointSetRegistration;
typedef itk::Euler3DTransform<PointSetToPointSetRegistration::TransformType::ParametersValueType> Euler3DTransform;
typedef itk::AffineTransform<PointSetToPointSetRegistration::TransformType::ParametersValueType, DIMENSION> AffineTransform;
typedef itk::IdentityTransform<EuclideanDistancePointMetric::TransformType::ParametersValueType, DIMENSION> IdentityTransform;
typedef itk::LevenbergMarquardtOptimizer LevenbergMarquardtOptimizer;

template <typename Transform>
//typedef Euler3DTransform Transform;
//typedef AffineTransform Transform;
double ITK_ICP_implementation(
	PointSet* movingPoints,
	PointSet* fixedPoints, 
	vnl_matrix_fixed< CoordinateType, DIMENSION + 1, DIMENSION + 1>& initial_matrix,
	unsigned int iteractions, 
	vtkMatrix4x4* output 
	)
{
	Transform::OffsetType offset;
	//offset.SetVnlVector(double_initialization_transform_matrix.get_column(3));
	offset.GetVnlVector().update(initial_matrix.get_column(3).extract(DIMENSION));
	Transform::MatrixType matrix;
	//matrix.GetVnlMatrix().update(double_initialization_transform_matrix.extract(DIMENSION, DIMENSION));
	matrix.GetVnlMatrix().update(initial_matrix.extract(DIMENSION, DIMENSION));
	Transform::Pointer transform = Transform::New();
	transform->SetOffset(offset);
	transform->SetMatrix(matrix);
	LevenbergMarquardtOptimizer::ScalesType scales;
	//(transform->GetNumberOfParameters());
	scales.SetSize(transform->GetNumberOfParameters());
	scales.Fill(0.01);


	LevenbergMarquardtOptimizer::Pointer levenbergMarquardtOptimizer =
		LevenbergMarquardtOptimizer::New();
	levenbergMarquardtOptimizer->SetUseCostFunctionGradient(false);
	levenbergMarquardtOptimizer->SetScales(scales);
	levenbergMarquardtOptimizer->SetNumberOfIterations(iteractions);
	levenbergMarquardtOptimizer->SetValueTolerance(1e-5);
	levenbergMarquardtOptimizer->SetGradientTolerance(1e-5);
	levenbergMarquardtOptimizer->SetEpsilonFunction(1e-6);

	PointSetToPointSetRegistration::Pointer pointSetToPointSetRegistration =
		PointSetToPointSetRegistration::New();
	pointSetToPointSetRegistration->SetMetric(EuclideanDistancePointMetric::New());
	pointSetToPointSetRegistration->SetOptimizer(levenbergMarquardtOptimizer);
	pointSetToPointSetRegistration->SetMovingPointSet(movingPoints);
	pointSetToPointSetRegistration->SetFixedPointSet(fixedPoints);
	pointSetToPointSetRegistration->SetTransform(transform);
	pointSetToPointSetRegistration->SetInitialTransformParameters(transform->GetParameters());
	pointSetToPointSetRegistration->Update();

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (j < 3) {
				output->SetElement(i, j, transform->GetMatrix()[i][j]);
			}
			else {
				output->SetElement(i, j, transform->GetOffset()[i]);
			}
		}
	}
	return levenbergMarquardtOptimizer->GetValue().mean();
}


void vtkITKIterativeCloestPoint::ITK_Calculation()
{

	vector<vtkPoints*> points;
	points.resize(2);
	points[0] = this->Source;
	points[1] = this->Target;
	vector<vnl_matrix<CoordinateType>> points_matrix(2, vnl_matrix<CoordinateType>());
	vector<vnl_vector_fixed<CoordinateType, DIMENSION>> centers(2, vnl_vector_fixed<CoordinateType, DIMENSION>());
	vector<vnl_vector_fixed<CoordinateType, DIMENSION>> eigenvalues(2, vnl_vector_fixed<CoordinateType, DIMENSION>());
	vector<vnl_matrix_fixed<CoordinateType, DIMENSION, DIMENSION>> eigenvectors(2, vnl_matrix_fixed<CoordinateType, DIMENSION, DIMENSION>());
	/**
	 * eigenvalue, eigenvector.
	 * eigenvector(principal axis), mean(center).
	 */
	for (int i = 0; i < 2; ++i) {
		points_matrix[i].set_size(DIMENSION, points[i]->GetNumberOfPoints());

		for (vtkIdType id = 0; id < points[i]->GetNumberOfPoints(); ++id) {
			double* point = points[i]->GetPoint(id);
			points_matrix[i][0][id] = point[0];
			points_matrix[i][1][id] = point[1];
			points_matrix[i][2][id] = point[2];
		}
		vnl_vector<CoordinateType> pointX = points_matrix[i].get_row(0);
		vnl_vector<CoordinateType> pointY = points_matrix[i].get_row(1);
		vnl_vector<CoordinateType> pointZ = points_matrix[i].get_row(2);

		centers[i][0] = pointX.mean();
		centers[i][1] = pointY.mean();
		centers[i][2] = pointZ.mean();

		pointX -= pointX.mean();
		pointY -= pointY.mean();
		pointZ -= pointZ.mean();
		vnl_matrix<CoordinateType> mean_matrix(points_matrix[i].rows(), points_matrix[i].columns());
		mean_matrix.set_row(0, pointX).set_row(1, pointY).set_row(2, pointZ);
		vnl_matrix_fixed<CoordinateType, DIMENSION, DIMENSION> covariance = mean_matrix * (mean_matrix.transpose());
		covariance /= points[i]->GetNumberOfPoints();

		vnl_symmetric_eigensystem<CoordinateType> eigenSystem(covariance);
		eigenvalues[i] = eigenSystem.D.get_diagonal();
		eigenvectors[i] = eigenSystem.V;
	}
	/**
	 * Push point to itk::PointSet
	 */
	vnl_matrix_fixed< CoordinateType, DIMENSION + 1, DIMENSION + 1> initialization_transform_matrix;
	double initialization_rms = itk::NumericTraits<double>::max();
	PointSet::Pointer initializationPointSet;
	vector<PointSet::Pointer> pointSets;
	pointSets.resize(2);
	for (int i = 0; i < 2; ++i) {
		pointSets[i] = PointSet::New();
		PointSet::PointsContainerPointer points = PointSet::PointsContainer::New();
		points->resize(points_matrix[1].columns());
		for (PointSet::PointsContainerIterator cit = points->Begin();
			cit != points->End(); ++cit) {
			//cit->Value().GetVnlVector().update(source_to_target_matrix.get_column(cit.Index()));
			cit->Value().GetVnlVector()[0] = points_matrix[i][0][cit->Index()];
			cit->Value().GetVnlVector()[1] = points_matrix[i][1][cit->Index()];
			cit->Value().GetVnlVector()[2] = points_matrix[i][2][cit->Index()];
		}
		pointSets[i]->SetPoints(points);
	}
	/**
	 * pair up 2 principal axes and centers to get initial rotation and translation 
	 */
	for (int positive_nagative_determinant = 0; positive_nagative_determinant < 4; ++positive_nagative_determinant) {
		vector<CoordinateType> opposite(2, 1.f);
		opposite[0] = (positive_nagative_determinant / 2) ? 1.f : -1.f;
		opposite[1] = (positive_nagative_determinant % 2) ? 1.f : -1.f;
		vector<vnl_vector_fixed<CoordinateType, DIMENSION>> source_axes(2, vnl_vector_fixed<CoordinateType, DIMENSION>());
		source_axes[0] = eigenvectors[0].get_column(2);
		source_axes[1] = eigenvectors[0].get_column(1);
		vector<vnl_vector_fixed<CoordinateType, DIMENSION>> target_axes(2, vnl_vector_fixed<CoordinateType, DIMENSION>());
		target_axes[0] = eigenvectors[1].get_column(2) * opposite[0];
		target_axes[1] = eigenvectors[1].get_column(1) * opposite[1];

		CoordinateType rotation_angle0 = angle(source_axes[0], target_axes[0]);
		vnl_vector_fixed<CoordinateType, DIMENSION> rotation_axis0 = vnl_cross_3d(source_axes[0], target_axes[0]).normalize();
		rotation_axis0 *= rotation_angle0;
		vnl_matrix_fixed<CoordinateType, DIMENSION, DIMENSION> rotation_matrix0;
		vnl_rotation_matrix(rotation_axis0, rotation_matrix0);


		source_axes[1] = rotation_matrix0 * source_axes[1];

		CoordinateType rotation_angle1 = angle(source_axes[1], target_axes[1]);
		vnl_vector_fixed<CoordinateType, DIMENSION> rotation_axis1 = vnl_cross_3d(source_axes[1], target_axes[1]).normalize();
		rotation_axis1 *= rotation_angle1;

		vnl_matrix_fixed<CoordinateType, DIMENSION, DIMENSION> rotation_matrix1;
		vnl_rotation_matrix(rotation_axis1, rotation_matrix1);

		vnl_matrix_fixed<CoordinateType, DIMENSION, DIMENSION> rotation_matrix = rotation_matrix1 * rotation_matrix0;

		vnl_matrix_fixed<CoordinateType, DIMENSION + 1, DIMENSION + 1> _centralization_matrix;
		_centralization_matrix.set_identity();
		for (int i = 0; i < DIMENSION; ++i) {
			_centralization_matrix[i][DIMENSION] = -centers[0][i];

		}

		vnl_matrix_fixed<CoordinateType, DIMENSION + 1, DIMENSION + 1> _centralization_matrix_inverse = vnl_inverse(_centralization_matrix);
		vnl_matrix_fixed<CoordinateType, DIMENSION + 1, DIMENSION + 1> _rotation_matrix;
		_rotation_matrix.set_identity();
		_rotation_matrix.update(rotation_matrix);


		vnl_matrix_fixed<CoordinateType, DIMENSION + 1, DIMENSION + 1> _translation_matrix;
		vnl_vector_fixed<CoordinateType, DIMENSION> _translation_vector = centers[1] - centers[0];
		_translation_matrix.set_identity();
		for (int i = 0; i < DIMENSION; ++i) {
			_translation_matrix[i][DIMENSION] = _translation_vector[i];
		}
		/**
		 * initial transform matrix by rotation with the first and second principal axes and 
		 * translation of the centers
		 */
		vnl_matrix_fixed<CoordinateType, DIMENSION + 1, DIMENSION + 1> transform_matrix =
			_translation_matrix *
			_centralization_matrix_inverse *
			_rotation_matrix *
			_centralization_matrix;


		vector<vnl_matrix<CoordinateType>> _mesh_matrix(2, vnl_matrix<CoordinateType>());
		for (int i = 0; i < 1; ++i) {
			_mesh_matrix[i].set_size(points_matrix[i].rows() + 1, points_matrix[i].columns());
			_mesh_matrix[i].fill(1.0f);
			_mesh_matrix[i].update(points_matrix[i]);
		}
		
		/**
		 * doing transformation using initial transform from pca.
		 * and push it back to itk::PointSet
		 */
		vnl_matrix<CoordinateType> source_to_target_matrix = (transform_matrix * _mesh_matrix[0]).extract(_mesh_matrix[0].rows() - 1, _mesh_matrix[0].columns());

		PointSet::Pointer sourceToTargetPointSet = PointSet::New();
		PointSet::PointsContainerPointer points = PointSet::PointsContainer::New();
		points->resize(source_to_target_matrix.columns());
		for (PointSet::PointsContainerIterator cit = points->Begin();
			cit != points->End(); ++cit) {
			//cit->Value().GetVnlVector().update(source_to_target_matrix.get_column(cit.Index()));
			cit->Value().GetVnlVector()[0] = source_to_target_matrix[0][cit->Index()];
			cit->Value().GetVnlVector()[1] = source_to_target_matrix[1][cit->Index()];
			cit->Value().GetVnlVector()[2] = source_to_target_matrix[2][cit->Index()];
		}
		sourceToTargetPointSet->SetPoints(points);
		
		EuclideanDistancePointMetricv4::Pointer euclideanDistancev4 = EuclideanDistancePointMetricv4::New();
		euclideanDistancev4->SetMovingPointSet(sourceToTargetPointSet);
		euclideanDistancev4->SetFixedPointSet(pointSets[1]);
		double rms = euclideanDistancev4->GetValue();
		if (rms < initialization_rms) {
			initialization_rms = rms;
			initialization_transform_matrix = transform_matrix;
			initializationPointSet = sourceToTargetPointSet;
		}
	}


	//vnl_matrix_fixed<Euler3DTransform::ParametersValueType, DIMENSION + 1, DIMENSION + 1> double_initialization_transform_matrix;
	//copy(initialization_transform_matrix.begin(), initialization_transform_matrix.end(), double_initialization_transform_matrix.begin());
	if (this->Mode == RIGID) {
		this->RMS = ITK_ICP_implementation<Euler3DTransform>(pointSets[0], pointSets[1], initialization_transform_matrix, this->NumberOfIterations, this->Matrix);
	}
	else if (this->Mode == AFFINE)
	{
		this->RMS = ITK_ICP_implementation<AffineTransform>(pointSets[0], pointSets[1], initialization_transform_matrix, this->NumberOfIterations, this->Matrix);
	}

}
