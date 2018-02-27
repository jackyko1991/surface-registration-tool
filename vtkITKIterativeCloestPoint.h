#ifndef __VTK_ITK_ITERATIVE_CLOEST_POINT_H__
#define __VTK_ITK_ITERATIVE_CLOEST_POINT_H__

#include <vtkLinearTransform.h>

class vtkITKIterativeCloestPoint : public vtkLinearTransform
{
public:
	static vtkITKIterativeCloestPoint* New();
	vtkTypeMacro(vtkITKIterativeCloestPoint, vtkLinearTransform);
	virtual void PrintSelf(ostream& os, vtkIndent indent) override;


	void SetSource(vtkPoints *points);
	vtkGetObjectMacro(Source, vtkPoints);

	void SetTarget(vtkPoints *points);
	vtkGetObjectMacro(Target, vtkPoints);

	vtkSetMacro(NumberOfIterations, unsigned int);
	vtkGetMacro(NumberOfIterations, unsigned int);

	vtkGetMacro(RMS, double);

	enum TRANSFORM_MODE
	{
		RIGID = 0,
		AFFINE,
	};

	virtual void Inverse() override;
	virtual vtkAbstractTransform* MakeTransform();

protected:
	vtkITKIterativeCloestPoint();
	virtual ~vtkITKIterativeCloestPoint() override;

	unsigned int NumberOfIterations;
	unsigned int Mode;
	double RMS;

	vtkPoints* Source;
	vtkPoints* Target;

	virtual void InternalDeepCopy(vtkAbstractTransform* transform) override;

	virtual void InternalUpdate() override;

	void ITK_Calculation();
	
	////template <typename Transform>
	//void ITK_ICP_implementation(
	//	vnl_matrix_fixed< CoordinateType, DIMENSION + 1, DIMENSION + 1> initial_matrix );

private:
	vtkITKIterativeCloestPoint(const vtkITKIterativeCloestPoint&) = delete;
	void operator=(const vtkITKIterativeCloestPoint&) = delete;
	
};

#endif // !__VTK_ITK_ITERATIVE_CLOEST_POINT_H__
