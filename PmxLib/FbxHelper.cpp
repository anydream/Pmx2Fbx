
#include <string>
#include "FbxHelper.h"
#include "Utils.h"
#include <windows.h>
#include <shellapi.h>

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(m_pSdkMgr->GetIOSettings()))
#endif

#define	UVSet1	"UVSet1"

bool g_bLatin = true;

static void ConvNameToLatin(const char * strName, std::string & strOut)
{
	int curr = 0;
	uint8_t ch;
	char strBuf[8];
	while ((ch = strName[curr]) != 0)
	{
		if (ch > 0x7f)
		{
			sprintf_s(strBuf, "%02x", ch);
			strOut.append(strBuf);
		}
		else
			strOut.push_back(ch);
		++curr;
	}
}

//////////////////////////////////////////////////////////////////////////
FbxHelper::FbxHelper()
{
	m_pSdkMgr = FbxManager::Create();
	if (!m_pSdkMgr)
	{
		throw std::wstring(L"Error: Unable to create FBX Manager!\n");
	}
	FbxIOSettings * ios = FbxIOSettings::Create(m_pSdkMgr, IOSROOT);
	m_pSdkMgr->SetIOSettings(ios);

	m_pScene = FbxScene::Create(m_pSdkMgr, "Scene");
}

FbxHelper::~FbxHelper()
{
	m_pSdkMgr->Destroy();
}

void FbxHelper::SetInfo()
{
	FbxDocumentInfo * pSceneInfo = FbxDocumentInfo::Create(m_pSdkMgr, "Info");
	m_pScene->SetSceneInfo(pSceneInfo);
}

bool FbxHelper::SaveScene(const char * pFileName, bool pEmbedMedia, int pFileFormat)
{
	int lMajor, lMinor, lRevision;
	bool lStatus = true;

	// Create an exporter.
	FbxExporter * lExporter = FbxExporter::Create(m_pSdkMgr, "");

	if (pFileFormat < 0 || pFileFormat >= m_pSdkMgr->GetIOPluginRegistry()->GetWriterFormatCount())
	{
		// Write in fall back format in less no ASCII format found
		pFileFormat = m_pSdkMgr->GetIOPluginRegistry()->GetNativeWriterFormat();

		// Try to export in ASCII if possible
		int lFormatIndex, lFormatCount = m_pSdkMgr->GetIOPluginRegistry()->GetWriterFormatCount();

		for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (m_pSdkMgr->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = m_pSdkMgr->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char * strBinary = "binary";
				if (lDesc.Find(strBinary) >= 0)
				{
					pFileFormat = lFormatIndex;
					break;
				}
			}
		}
	}

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
	// shows how to change these states.
	IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
	IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
	IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, pEmbedMedia);
	IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
	IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
	IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
	IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	// Initialize the exporter by providing a filename.
	if (lExporter->Initialize(pFileName, pFileFormat, m_pSdkMgr->GetIOSettings()) == false)
	{
		FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		return false;
	}

	FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
	FBXSDK_printf("FBX file format version %d.%d.%d\n\n", lMajor, lMinor, lRevision);

	// Export the scene.
	lStatus = lExporter->Export(m_pScene);

	// Destroy the exporter.
	lExporter->Destroy();
	return lStatus;
}

void FbxHelper::AddNodeToRoot(FbxNode * pNode)
{
	m_pScene->GetRootNode()->AddChild(pNode);
}

FbxSurfacePhong * FbxHelper::NewPhong(const char * strName)
{
	if (g_bLatin)
	{
		std::string strLatin;
		ConvNameToLatin(strName, strLatin);
		return FbxSurfacePhong::Create(m_pScene, strLatin.c_str());
	}
	return FbxSurfacePhong::Create(m_pScene, strName);
}

static bool CopyFileTo(const char * strFrom, const char * strTo)
{
	std::wstring strWfrom, strWto;
	Platform_Utf8To16(strFrom, strWfrom);
	Platform_Utf8To16(strTo, strWto);
	strWfrom.push_back(0);
	strWto.push_back(0);

	SHFILEOPSTRUCT fop;
	fop.hwnd = 0;
	fop.wFunc = FO_COPY;
	fop.pFrom = strWfrom.c_str();
	fop.pTo = strWto.c_str();
	fop.fFlags = FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION;
	fop.fAnyOperationsAborted = FALSE;
	fop.hNameMappings = 0;
	fop.lpszProgressTitle = 0;
	return SHFileOperation(&fop) == 0;
}

FbxFileTexture * FbxHelper::NewTexture(const char * strName, const char * strFileName)
{
	std::string strLatin, strLatinFileName;
	if (g_bLatin)
	{
		ConvNameToLatin(strName, strLatin);
		strName = strLatin.c_str();

		ConvNameToLatin(strFileName, strLatinFileName);
		if (strFileName != strLatinFileName)
		{
			if (!CopyFileTo(strFileName, strLatinFileName.c_str()))
			{
				std::string strAcpInUtf8;
				Platform_SJIS2ACP_UTF8(strFileName, strAcpInUtf8);
				CopyFileTo(strAcpInUtf8.c_str(), strLatinFileName.c_str());
			}
			strFileName = strLatinFileName.c_str();
		}
	}

	FbxFileTexture * ptr = FbxFileTexture::Create(m_pScene, strName);
	ptr->SetFileName(strFileName);
	ptr->SetTextureUse(FbxTexture::eStandard);
	ptr->SetMappingType(FbxTexture::eUV);
	ptr->SetMaterialUse(FbxFileTexture::eModelMaterial);
	ptr->SetSwapUV(false);
	ptr->SetTranslation(0.0, 0.0);
	ptr->SetScale(1.0, 1.0);
	ptr->SetRotation(0.0, 0.0);
	ptr->UVSet.Set(UVSet1);
	return ptr;
}

void FbxHelper::NewBoneNode(const char * strName, const Shape * pShape, BoneInfo & boneInfo)
{
	std::string strLatin;
	if (g_bLatin)
	{
		ConvNameToLatin(strName, strLatin);
		strName = strLatin.c_str();
	}

	boneInfo.m_pSkeleton = FbxSkeleton::Create(m_pScene, strName);

	std::string strNodeName("Bone ");
	strNodeName += strName;
	boneInfo.m_pNode = FbxNode::Create(m_pScene, strNodeName.c_str());
	boneInfo.m_pNode->SetNodeAttribute(boneInfo.m_pSkeleton);

	boneInfo.m_pCluster = FbxCluster::Create(m_pScene, "");
	boneInfo.m_pCluster->SetLink(boneInfo.m_pNode);
	boneInfo.m_pCluster->SetLinkMode(FbxCluster::eTotalOne);

	boneInfo.m_pCluster->SetTransformMatrix(pShape->m_pNode->EvaluateGlobalTransform());

	pShape->m_pSkin->AddCluster(boneInfo.m_pCluster);
}

static void AddNodeRecursively(FbxArray<FbxNode*>& pNodeArray, FbxNode* pNode)
{
	if (pNode)
	{
		AddNodeRecursively(pNodeArray, pNode->GetParent());

		if (pNodeArray.Find(pNode) == -1)
		{
			// Node not in the list, add it
			pNodeArray.Add(pNode);
		}
	}
}

void FbxHelper::StoreBindPose(FbxNode * pNode, std::map<FbxNode*, FbxAMatrix> & poseMap)
{
	// In the bind pose, we must store all the link's global matrix at the time of the bind.
	// Plus, we must store all the parent(s) global matrix of a link, even if they are not
	// themselves deforming any model.

	// In this example, since there is only one model deformed, we don't need walk through 
	// the scene
	//

	// Now list the all the link involve in the patch deformation
	FbxArray<FbxNode*> lClusteredFbxNodes;
	int                       i, j;

	if (pNode && pNode->GetNodeAttribute())
	{
		int lSkinCount = 0;
		int lClusterCount = 0;
		switch (pNode->GetNodeAttribute()->GetAttributeType())
		{
		default:
			break;
		case FbxNodeAttribute::eMesh:
		case FbxNodeAttribute::eNurbs:
		case FbxNodeAttribute::ePatch:

			lSkinCount = ((FbxGeometry*)pNode->GetNodeAttribute())->GetDeformerCount(FbxDeformer::eSkin);
			//Go through all the skins and count them
			//then go through each skin and get their cluster count
			for (i = 0; i < lSkinCount; ++i)
			{
				FbxSkin *lSkin = (FbxSkin*)((FbxGeometry*)pNode->GetNodeAttribute())->GetDeformer(i, FbxDeformer::eSkin);
				lClusterCount += lSkin->GetClusterCount();
			}
			break;
		}
		//if we found some clusters we must add the node
		if (lClusterCount)
		{
			//Again, go through all the skins get each cluster link and add them
			for (i = 0; i < lSkinCount; ++i)
			{
				FbxSkin *lSkin = (FbxSkin*)((FbxGeometry*)pNode->GetNodeAttribute())->GetDeformer(i, FbxDeformer::eSkin);
				lClusterCount = lSkin->GetClusterCount();
				for (j = 0; j < lClusterCount; ++j)
				{
					FbxNode* lClusterNode = lSkin->GetCluster(j)->GetLink();
					AddNodeRecursively(lClusteredFbxNodes, lClusterNode);
				}

			}

			// Add the patch to the pose
			lClusteredFbxNodes.Add(pNode);
		}
	}

	// Now create a bind pose with the link list
	if (lClusteredFbxNodes.GetCount())
	{
		// A pose must be named. Arbitrarily use the name of the patch node.
		FbxPose* lPose = FbxPose::Create(m_pScene, pNode->GetName());

		// default pose type is rest pose, so we need to set the type as bind pose
		lPose->SetIsBindPose(true);

		for (i = 0; i < lClusteredFbxNodes.GetCount(); i++)
		{
			FbxNode * lKFbxNode = lClusteredFbxNodes.GetAt(i);

			if (poseMap.find(lKFbxNode) == poseMap.end())
			{
				poseMap[lKFbxNode] = lKFbxNode->EvaluateGlobalTransform();
			}
		}

		for (auto & item : poseMap)
		{
			lPose->Add(item.first, item.second);
		}

		// Add the pose to the scene
		m_pScene->AddPose(lPose);
	}
}

//////////////////////////////////////////////////////////////////////////
FbxHelper::Shape * FbxHelper::BeginShape(const char * strName)
{
	if (g_bLatin)
	{
		std::string strLatin;
		ConvNameToLatin(strName, strLatin);
		return new Shape(this, strLatin.c_str());
	}
	return new Shape(this, strName);
}

void FbxHelper::EndShape(Shape * pShape)
{
	StoreBindPose(pShape->m_pNode, pShape->m_BindPoses);
	delete pShape;
}

FbxHelper::Shape::Shape(FbxHelper * owner, const char * strName)
	: m_pOwner(owner)
{
	m_pNode = FbxNode::Create(m_pOwner->m_pScene, strName);
	m_pMesh = FbxMesh::Create(m_pOwner->m_pScene, strName);
	m_pNode->SetNodeAttribute(m_pMesh);
	owner->AddNodeToRoot(m_pNode);

	m_pNormalElement = m_pMesh->CreateElementNormal();
	m_pNormalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
	m_pNormalElement->SetReferenceMode(FbxGeometryElement::eDirect);

	m_pUVElement = m_pMesh->CreateElementUV(UVSet1);
	m_pUVElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
	m_pUVElement->SetReferenceMode(FbxGeometryElement::eDirect);

	m_pMaterialElement = m_pMesh->CreateElementMaterial();
	m_pMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
	m_pMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

	m_pSkin = FbxSkin::Create(m_pOwner->m_pScene, "");
	m_pMesh->AddDeformer(m_pSkin);
}

void FbxHelper::Shape::InitPositionSize(int count)
{
	m_pMesh->InitControlPoints(count);
}

void FbxHelper::Shape::SetPositionAt(const Vector3 & pos, int idx)
{
	m_pMesh->SetControlPointAt(FbxVector4(pos.X, pos.Y, pos.Z), idx);
}

void FbxHelper::Shape::AddNormal(const Vector3 & normal)
{
	m_pNormalElement->GetDirectArray().Add(FbxVector4(normal.X, normal.Y, normal.Z));
}

void FbxHelper::Shape::AddUV(const Vector2 & uv)
{
	m_pUVElement->GetDirectArray().Add(FbxVector2(uv.X, -uv.Y));
}

void FbxHelper::Shape::BeginFace(int matID)
{
	m_pMesh->BeginPolygon(matID);
}

void FbxHelper::Shape::AddIndex(int vertID)
{
	m_pMesh->AddPolygon(vertID);
}

void FbxHelper::Shape::EndFace()
{
	m_pMesh->EndPolygon();
}

void FbxHelper::Shape::AddMaterial(FbxSurfaceMaterial * pMaterial)
{
	m_pNode->AddMaterial(pMaterial);
}

void FbxHelper::Shape::SetBindPose(FbxNode * pNode, const FbxAMatrix & mat)
{
	m_BindPoses[pNode] = mat;
}