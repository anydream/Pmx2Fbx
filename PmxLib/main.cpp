#include <vld.h>
#include <stdio.h>
#include <windows.h>
#include "PmxReader.h"
#include "FbxHelper.h"

static void SavePmxToFbx(PmxReader & pmx, const char * strFileName)
{
	FbxHelper fbx;

	FbxHelper::Shape * shp = fbx.BeginShape(pmx.ModelName.c_str());
	{
		// 写入顶点
		shp->InitPositionSize(pmx.VertexList.size());

		int count = 0;
		for (auto & item : pmx.VertexList)
		{
			shp->SetPositionAt(item.Position, count);
			shp->AddNormal(item.Normal);
			shp->AddUV(item.UV);
			++count;
		}

		// 遍历材质组
		int currFace = 0;
		for (size_t matID = 0, szMat = pmx.MaterialList.size(); matID < szMat; ++matID)
		{
			// 写入材质
			auto & currMat = pmx.MaterialList[matID];
			FbxSurfacePhong * pFbxMat = fbx.NewPhong(currMat.Name.c_str());
			pFbxMat->Diffuse.Set(FbxDouble3(currMat.Diffuse.X, currMat.Diffuse.Y, currMat.Diffuse.Z));
			pFbxMat->Ambient.Set(FbxDouble3(currMat.Ambient.X, currMat.Ambient.Y, currMat.Ambient.Z));
			pFbxMat->Specular.Set(FbxDouble3(currMat.Specular.X, currMat.Specular.Y, currMat.Specular.Z));
			pFbxMat->SpecularFactor = currMat.Power;
			// 设置纹理
			FbxFileTexture * pFbxTex = fbx.NewTexture("Diffuse", currMat.Tex.c_str());
			pFbxMat->Diffuse.ConnectSrcObject(pFbxTex);
			// 设置透明通道
			FbxFileTexture * pFbxOpacityTex = fbx.NewTexture("Opacity", currMat.Tex.c_str());
			pFbxOpacityTex->SetAlphaSource(FbxTexture::eBlack);
			pFbxMat->TransparentColor.ConnectSrcObject(pFbxOpacityTex);
			shp->AddMaterial(pFbxMat);

			// 写入当前材质对应的面
			count = 0;
			for (int endFace = currFace + currMat.FaceCount; currFace < endFace; ++currFace)
			{
				if (count == 0)
				{
					shp->BeginFace(matID);
				}

				shp->AddIndex(pmx.FaceList[currFace]);

				if (count == 2)
				{
					shp->EndFace();
					count = 0;
				}
				else
					++count;
			}
		}

		// 写入骨架信息
		std::vector<FbxHelper::BoneInfo> fbxBoneList;
		fbxBoneList.resize(pmx.BoneList.size());
		count = 0;
		for (auto & currBone : pmx.BoneList)
		{
			fbx.NewBoneNode(currBone.Name.c_str(), shp, fbxBoneList[count]);
			++count;
		}
		size_t boneID = 0;
		for (auto & currBone : pmx.BoneList)
		{
			if (currBone.Parent < 0)
			{
				fbxBoneList[boneID].m_pSkeleton->SetSkeletonType(FbxSkeleton::eRoot);
				fbx.AddNodeToRoot(fbxBoneList[boneID].m_pNode);
			}
			else
			{
				fbxBoneList[boneID].m_pSkeleton->SetSkeletonType(FbxSkeleton::eLimbNode);
				fbxBoneList[currBone.Parent].m_pNode->AddChild(fbxBoneList[boneID].m_pNode);
			}
			++boneID;
		}
		// 写入骨骼初始位置
		count = 0;
		for (auto & item : fbxBoneList)
		{
			auto & currBone = pmx.BoneList[count];
			++count;

			FbxAMatrix matLocal;
			matLocal.SetRow(0, FbxVector4(currBone.LocalMatrix._11, currBone.LocalMatrix._12, currBone.LocalMatrix._13, currBone.LocalMatrix._14));
			matLocal.SetRow(1, FbxVector4(currBone.LocalMatrix._21, currBone.LocalMatrix._22, currBone.LocalMatrix._23, currBone.LocalMatrix._24));
			matLocal.SetRow(2, FbxVector4(currBone.LocalMatrix._31, currBone.LocalMatrix._32, currBone.LocalMatrix._33, currBone.LocalMatrix._34));
			matLocal.SetRow(3, FbxVector4(currBone.LocalMatrix._41, currBone.LocalMatrix._42, currBone.LocalMatrix._43, currBone.LocalMatrix._44));

			item.m_pNode->LclTranslation.Set(FbxDouble3(matLocal.GetT()));
			item.m_pNode->LclRotation.Set(FbxDouble3(matLocal.GetR()));

#if 0
			auto & matWorld = item.m_pNode->EvaluateGlobalTransform();
#else
			FbxAMatrix matWorld;
			matWorld.SetRow(0, FbxVector4(currBone.WorldMatrix._11, currBone.WorldMatrix._12, currBone.WorldMatrix._13, currBone.WorldMatrix._14));
			matWorld.SetRow(1, FbxVector4(currBone.WorldMatrix._21, currBone.WorldMatrix._22, currBone.WorldMatrix._23, currBone.WorldMatrix._24));
			matWorld.SetRow(2, FbxVector4(currBone.WorldMatrix._31, currBone.WorldMatrix._32, currBone.WorldMatrix._33, currBone.WorldMatrix._34));
			matWorld.SetRow(3, FbxVector4(currBone.WorldMatrix._41, currBone.WorldMatrix._42, currBone.WorldMatrix._43, currBone.WorldMatrix._44));
#endif
			item.m_pCluster->SetTransformLinkMatrix(matWorld);
		}

		// 写入权重
		count = 0;
		for (auto & item : pmx.VertexList)
		{
			for (int i = 0; i < 4; ++i)
			{
				auto & currWeight = item.Weight[i];
				if (currWeight.IsValid())
				{
					fbxBoneList[currWeight.Bone].m_pCluster->AddControlPointIndex(count, currWeight.Value);
				}
			}
			++count;
		}
	}
	fbx.EndShape(shp);

	fbx.SaveScene(strFileName);
}

//////////////////////////////////////////////////////////////////////////
static void * ReadFile(const wchar_t * strFileName, size_t & szFileLen)
{
	void * pResult = 0;
	FILE * fp = 0;
	_wfopen_s(&fp, strFileName, L"rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		szFileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		pResult = malloc(szFileLen);
		fread(pResult, szFileLen, 1, fp);

		fclose(fp);
	}
	return pResult;
}

int wmain(int argc, const wchar_t ** argv)
{
	if (argc == 2)
	{
		const wchar_t * strInput = argv[1];

		std::wstring strInputPath;
		for (int i = wcslen(strInput) - 1; i >= 0; --i)
		{
			if (strInput[i] == L'\\')
			{
				strInputPath.assign(strInput, strInput + i);
				SetCurrentDirectoryW(strInputPath.c_str());
				break;
			}
		}

		size_t flen = 0;
		void * pBuf = ReadFile(strInput, flen);
		if (pBuf)
		{
			PmxReader reader(pBuf, flen);
			free(pBuf);

			std::string strFileName;
			Platform_Utf16To8(strInput, strFileName);
			SavePmxToFbx(reader, (strFileName + ".fbx").c_str());
		}
	}
	return 0;
}