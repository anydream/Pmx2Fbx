
#ifndef __FbxHelper_h__
#define __FbxHelper_h__

#include <map>
#include <fbxsdk.h>
#include "MathVector.h"

class FbxHelper
{
public:
	class Shape
	{
		friend class FbxHelper;
	public:
		Shape(FbxHelper * owner, const char * strName);

		void InitPositionSize(int count);
		void SetPositionAt(const Vector3 & pos, int idx);
		void AddNormal(const Vector3 & normal);
		void AddUV(const Vector2 & uv);

		void BeginFace(int matID);
		void AddIndex(int vertID);
		void EndFace();

		void AddMaterial(FbxSurfaceMaterial * pMaterial);

		void SetBindPose(FbxNode * pNode, const FbxAMatrix & mat);

	private:
		FbxHelper					*m_pOwner;
		FbxNode						*m_pNode;
		FbxMesh						*m_pMesh;
		FbxGeometryElementNormal	*m_pNormalElement;
		FbxGeometryElementUV		*m_pUVElement;

		FbxGeometryElementMaterial	*m_pMaterialElement;

		FbxSkin						*m_pSkin;

		std::map<FbxNode*, FbxAMatrix>	m_BindPoses;
	};

	struct BoneInfo
	{
		FbxNode		*m_pNode;
		FbxSkeleton	*m_pSkeleton;
		FbxCluster	*m_pCluster;
	};

public:
	FbxHelper();
	~FbxHelper();

	void SetInfo();
	bool SaveScene(const char * pFileName, bool pEmbedMedia = false, int pFileFormat = -1);

	void AddNodeToRoot(FbxNode * pNode);

	Shape * BeginShape(const char * strName);
	void EndShape(Shape * pShape);

	FbxSurfacePhong * NewPhong(const char * strMatName);
	FbxFileTexture * NewTexture(const char * strName, const char * strFileName);
	void NewBoneNode(const char * strName, const Shape * pShape, BoneInfo & boneInfo);

public:
	void StoreBindPose(FbxNode * pNode, std::map<FbxNode*, FbxAMatrix> & poseMap);

private:
	FbxManager	*m_pSdkMgr;
	FbxScene	*m_pScene;
};

#endif