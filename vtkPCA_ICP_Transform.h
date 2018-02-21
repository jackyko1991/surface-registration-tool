#ifndef __VTK_PCA_ICP_TRANSFORM_H__
#define __VTK_PCA_ICP_TRANSFORM_H__

#include "vtkIterativeClosestPointTransform.h"

class vtkMatrix4x4;
class vtkPolyData;

class vtkPCA_ICP_Transform : public vtkIterativeClosestPointTransform
{
public:
	static vtkPCA_ICP_Transform* New();
	vtkTypeMacro(vtkPCA_ICP_Transform, vtkIterativeClosestPointTransform);
	virtual void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	vtkGetMacro(RMS, double);
	vtkGetObjectMacro(TransformPolyData, vtkPolyData);

	vtkGetVector3Macro(SourceCenter, double);
	vtkGetVector3Macro(TargetCenter, double);
	vtkGetVector3Macro(SourceDirection0, double);
	vtkGetVector3Macro(TargetDirection0, double);
	vtkGetVector3Macro(SourceDirection1, double);
	vtkGetVector3Macro(TargetDirection1, double);




protected:
	vtkPCA_ICP_Transform();
	virtual ~vtkPCA_ICP_Transform() VTK_OVERRIDE;
	virtual void InternalUpdate() VTK_OVERRIDE;

	double RMS;
	vtkPolyData* TransformPolyData;

	double SourceCenter[3];
	double TargetCenter[3];
	double SourceDirection0[3];
	double SourceDirection1[3];
	double TargetDirection0[3];
	double TargetDirection1[3];

	
};

#endif // !__VTK_PCA_ICP_TRANSFORM_H__
