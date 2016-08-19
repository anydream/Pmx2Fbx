
#ifndef __PmxReader_h__
#define __PmxReader_h__

#include <map>
#include <set>
#include <unordered_map>
#include <DirectXMath.h>
#include "MathVector.h"
#include "Utils.h"

using namespace DirectX;

class PmxReader
{
public:
	struct DeformType
	{
		enum Type
		{
			BDEF1,
			BDEF2,
			BDEF4,
			SDEF,
			QDEF
		};
	};

	struct BoneWeight
	{
		int		Bone;
		float	Value;

		BoneWeight()
			: Bone(-1), Value(0)
		{}

		bool IsValid() const
		{
			return Bone != -1 && Value != 0;
		}
	};

	struct PmxVertex
	{
		Vector3				Position, Normal, C0, R0, R1, RW0, RW1;
		Vector2				UV;
		Vector4				UVA[4];
		DeformType::Type	Deform;
		bool				bSDEF;
		BoneWeight			Weight[4];
		float				EdgeScale;

		PmxVertex()
		{
			Deform = DeformType::BDEF1;
			bSDEF = false;
			EdgeScale = 1;
		}

		void ReadVertex(BinReader & reader, PmxReader * owner)
		{
			Position.Read(reader);
			Normal.Read(reader);
			UV.Read(reader);

			for (int i = 0; i < owner->UVACount && i < 4; ++i)
			{
				UVA[i].Read(reader);
			}

			Deform = (DeformType::Type)reader.Read<uint8_t>();
			bSDEF = false;

			switch (Deform)
			{
			case DeformType::BDEF1:
				Weight[0].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[0].Value = 1;
				break;

			case DeformType::BDEF2:
				Weight[0].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[1].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[0].Value = reader.Read<float>();
				Weight[1].Value = 1 - Weight[0].Value;
				break;

			case DeformType::BDEF4:
			case DeformType::QDEF:
				Weight[0].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[1].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[2].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[3].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[0].Value = reader.Read<float>();
				Weight[1].Value = reader.Read<float>();
				Weight[2].Value = reader.Read<float>();
				Weight[3].Value = reader.Read<float>();
				break;

			case DeformType::SDEF:
				Weight[0].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[1].Bone = ReadElement_Int32(reader, owner->BoneSize, true);
				Weight[0].Value = reader.Read<float>();
				Weight[1].Value = 1 - Weight[0].Value;
				C0.Read(reader);
				R0.Read(reader);
				R1.Read(reader);
				CalcSDEF_RW();
				bSDEF = true;
				break;
			}
			EdgeScale = reader.Read<float>();
		}

		void CalcSDEF_RW()
		{
			Vector3 vector = (R0.Mul(Weight[0].Value)).Add(R1.Mul(Weight[1].Value));
			RW0 = R0.Sub(vector);
			RW1 = R1.Sub(vector);
		}
	};

	struct MaterialFlags
	{
		enum Type
		{
			DrawBoth = 1,
			Edge = 0x10,
			LineDraw = 0x80,
			None = 0,
			PointDraw = 0x40,
			SelfShadow = 8,
			SelfShadowMap = 4,
			Shadow = 2,
			VertexColor = 0x20
		};
	};

	struct SphereModeType
	{
		enum Type
		{
			None,
			Mul,
			Add,
			SubTex
		};
	};

	struct PmxMaterial
	{
		std::string				Name, NameE, Tex, Sphere, Toon, Memo;
		Vector4					Diffuse, EdgeColor;
		Vector3					Specular, Ambient;
		float					Power, EdgeSize;
		MaterialFlags::Type		Flags;
		SphereModeType::Type	SphereMode;
		int						FaceCount;

		PmxMaterial()
		{
			Power = 0;
			EdgeSize = 1;
			FaceCount = 0;
			Flags = MaterialFlags::None;
			SphereMode = SphereModeType::None;
		}

		void ReadMaterial(BinReader & reader, PmxReader * owner)
		{
			owner->ReadString(Name);
			owner->ReadString(NameE);
			Diffuse.Read(reader);
			Specular.Read(reader);
			Power = reader.Read<float>();
			Ambient.Read(reader);
			Flags = (MaterialFlags::Type)reader.Read<uint8_t>();
			EdgeColor.Read(reader);
			EdgeSize = reader.Read<float>();
			Tex = owner->IndexToName[ReadElement_Int32(reader, owner->TexSize, true)];
			Sphere = owner->IndexToName[ReadElement_Int32(reader, owner->TexSize, true)];
			SphereMode = (SphereModeType::Type)reader.Read<uint8_t>();

			if (reader.Read<uint8_t>() == 0)
			{
				Toon = owner->IndexToName[ReadElement_Int32(reader, owner->TexSize, true)];
			}
			else
			{
				int n = reader.Read<uint8_t>();
				Toon = GetToonName(n);
			}
			owner->ReadString(Memo);
			//UpdateAttributeFromMemo();
			FaceCount = ReadElement_Int32(reader, 4, true);
		}

		static inline std::string ToString(int val)
		{
			char buf[2 * _MAX_INT_DIG];
			sprintf_s(buf, sizeof(buf), "%d", val);
			return std::string(buf);
		}

		static std::string GetToonName(int n)
		{
			if (n < 0)
			{
				return "toon0.bmp";
			}
			int num = n + 1;
			return ("toon" + ToString(num) + ".bmp");
		}
	};

	struct BoneFlags
	{
		enum Type
		{
			AfterPhysics = 0x1000,
			AppendLocal = 0x80,
			AppendRotation = 0x100,
			AppendTranslation = 0x200,
			Enable = 0x10,
			ExtParent = 0x2000,
			FixAxis = 0x400,
			IK = 0x20,
			LocalFrame = 0x800,
			None = 0,
			Rotation = 2,
			ToBone = 1,
			Translation = 4,
			Visible = 8
		};
	};

	struct IKLink
	{
		int		Bone;
		bool	IsLimit;
		Vector3	Low, High;

		IKLink()
		{
			Bone = -1;
			IsLimit = false;
		}

		void ReadLink(BinReader & reader, PmxReader * owner)
		{
			Bone = ReadElement_Int32(reader, owner->BoneSize, true);
			IsLimit = reader.Read<uint8_t>() != 0;
			if (IsLimit)
			{
				Low.Read(reader);
				High.Read(reader);
			}
		}
	};

	struct PmxIK
	{
		int					Target, LoopCount;
		float				Angle;
		std::vector<IKLink>	LinkList;

		PmxIK()
		{
			Target = -1;
			LoopCount = 0;
			Angle = 1;
		}

		void ReadIK(BinReader & reader, PmxReader * owner)
		{
			Target = ReadElement_Int32(reader, owner->BoneSize, true);
			LoopCount = ReadElement_Int32(reader, 4, true);
			Angle = reader.Read<float>();

			int num = ReadElement_Int32(reader, 4, true);
			LinkList.resize(num);

			for (auto & item : LinkList)
			{
				item.ReadLink(reader, owner);
			}
		}
	};

	struct PmxBone
	{
		std::string		Name, NameE;
		Vector3			Position, RelativePosition, To_Offset, Axis, LocalX, LocalY, LocalZ;
		int				Parent, Level, To_Bone, AppendParent, ExtKey;
		float			AppendRatio;
		BoneFlags::Type	Flags;
		PmxIK			IK;
		PmxBone			*RefParent;
		bool			bCalcHierarchy;
		XMMATRIX		WorldMatrix, LocalMatrix;

		PmxBone()
		{
			Parent = -1;
			To_Bone = -1;
			AppendParent = -1;
			AppendRatio = 1;
			Level = 0;
			ExtKey = 0;
			Flags = (BoneFlags::Type)(BoneFlags::Enable | BoneFlags::Visible | BoneFlags::Rotation);
			LocalX.X = 1;
			LocalY.Y = 1;
			LocalZ.Z = 1;
			RefParent = 0;
			bCalcHierarchy = false;
		}

		void ReadBone(BinReader & reader, PmxReader * owner)
		{
			owner->ReadString(Name);
			owner->ReadString(NameE);
			Position.Read(reader);
			Parent = ReadElement_Int32(reader, owner->BoneSize, true);
			Level = ReadElement_Int32(reader, 4, true);
			Flags = (BoneFlags::Type)ReadElement_Int32(reader, 2, false);
			if (GetFlag(BoneFlags::ToBone))
			{
				To_Bone = ReadElement_Int32(reader, owner->BoneSize, true);
			}
			else
			{
				To_Offset.Read(reader);
			}
			if (GetFlag(BoneFlags::AppendRotation) || GetFlag(BoneFlags::AppendTranslation))
			{
				AppendParent = ReadElement_Int32(reader, owner->BoneSize, true);
				AppendRatio = reader.Read<float>();
			}
			if (GetFlag(BoneFlags::FixAxis))
			{
				Axis.Read(reader);
			}
			if (GetFlag(BoneFlags::LocalFrame))
			{
				LocalX.Read(reader);
				LocalZ.Read(reader);
				NormalizeLocal();
			}
			if (GetFlag(BoneFlags::ExtParent))
			{
				ExtKey = ReadElement_Int32(reader, 4, true);
			}
			if (GetFlag(BoneFlags::IK))
			{
				IK.ReadIK(reader, owner);
			}
		}

		bool GetFlag(BoneFlags::Type f)
		{
			return ((f & Flags) == f);
		}

		void NormalizeLocal()
		{
			LocalZ.Normalize();
			LocalX.Normalize();
			LocalY = Vector3::Cross(LocalZ, LocalX);
			LocalZ = Vector3::Cross(LocalX, LocalY);
			LocalY.Normalize();
			LocalZ.Normalize();
		}

		void CalcHierarchy()
		{
			if (!bCalcHierarchy)
			{
				bCalcHierarchy = true;

				LocalMatrix = XMMatrixSet(
					LocalX.X, LocalX.Y, LocalX.Z, 0,
					LocalY.X, LocalY.Y, LocalY.Z, 0,
					LocalZ.X, LocalZ.Y, LocalZ.Z, 0,
					0, 0, 0, 1);

				if (RefParent)
				{
					RefParent->CalcHierarchy();
					RelativePosition = Position.Sub(RefParent->Position);

					WorldMatrix = XMMatrixMultiply(LocalMatrix, RefParent->WorldMatrix);
				}
				else
				{
					RelativePosition = Position;
					WorldMatrix = LocalMatrix;
				}
				WorldMatrix.r[3] = XMVectorSet(Position.X, Position.Y, Position.Z, 1);

				if (RefParent)
				{
					XMMATRIX invWorld = XMMatrixInverse(0, RefParent->WorldMatrix);
					LocalMatrix = XMMatrixMultiply(WorldMatrix, invWorld);
				}
				else
				{
					LocalMatrix = WorldMatrix;
				}
			}
		}
	};

	struct PmxBaseMorph
	{
	};

	struct PmxGroupMorph : PmxBaseMorph
	{
		int		Index;
		float	Ratio;

		PmxGroupMorph()
		{
			Index = -1;
			Ratio = 1;
		}

		void Read(BinReader & reader, PmxReader * owner)
		{
			Index = ReadElement_Int32(reader, owner->MorphSize, true);
			Ratio = reader.Read<float>();
		}
	};

	struct PmxVertexMorph : PmxBaseMorph
	{
		int		Index;
		Vector3	Offset;

		PmxVertexMorph()
		{
			Index = -1;
		}

		void Read(BinReader & reader, PmxReader * owner)
		{
			Index = ReadElement_Int32(reader, owner->VertexSize, false);
			Offset.Read(reader);
		}
	};

	struct PmxBoneMorph : PmxBaseMorph
	{
		int		Index;
		Vector3	Translation;
		Vector4	Rotaion;

		PmxBoneMorph()
		{
			Index = -1;
		}

		void Read(BinReader & reader, PmxReader * owner)
		{
			Index = ReadElement_Int32(reader, owner->BoneSize, true);
			Translation.Read(reader);
			Rotaion.Read(reader);
		}
	};

	struct PmxUVMorph : PmxBaseMorph
	{
		int		Index;
		Vector4	Offset;

		PmxUVMorph()
		{
			Index = -1;
		}

		void Read(BinReader & reader, PmxReader * owner)
		{
			Index = ReadElement_Int32(reader, owner->VertexSize, false);
			Offset.Read(reader);
		}
	};

	struct MorphData
	{
		Vector4	Diffuse;
		Vector4	Specular;
		Vector3	Ambient;
		float	EdgeSize;
		Vector4	EdgeColor;
		Vector4	Tex;
		Vector4	Sphere;
		Vector4	Toon;

		MorphData()
		{
			EdgeSize = 1;
		}
	};

	struct OpType
	{
		enum Type
		{
			Mul,
			Add
		};
	};

	struct PmxMaterialMorph : PmxBaseMorph
	{
		int				Index;
		OpType::Type	Op;
		MorphData		Data;

		void Read(BinReader & reader, PmxReader * owner)
		{
			Index = ReadElement_Int32(reader, owner->MaterialSize, true);
			Op = (OpType::Type)reader.Read<uint8_t>();
			Data.Diffuse.Read(reader);
			Data.Specular.Read(reader);
			Data.Ambient.Read(reader);
			Data.EdgeColor.Read(reader);
			Data.EdgeSize = reader.Read<float>();
			Data.Tex.Read(reader);
			Data.Sphere.Read(reader);
			Data.Toon.Read(reader);
		}
	};

	struct PmxImpulseMorph : PmxBaseMorph
	{
		int		Index;
		bool	Local;
		Vector3	Torque;
		Vector3	Velocity;

		PmxImpulseMorph()
		{
			Index = -1;
			Local = false;
		}

		void Read(BinReader & reader, PmxReader * owner)
		{
			Index = ReadElement_Int32(reader, owner->BodySize, true);
			Local = reader.Read<uint8_t>() != 0;
			Velocity.Read(reader);
			Torque.Read(reader);
		}
	};

	struct OffsetKind
	{
		enum Type
		{
			Group,
			Vertex,
			Bone,
			UV,
			UVA1,
			UVA2,
			UVA3,
			UVA4,
			Material,
			Flip,
			Impulse
		};
	};

	struct PmxMorph
	{
		std::string					Name, NameE;
		int							Panel;
		OffsetKind::Type			Kind;
		std::vector<PmxBaseMorph*>	OffsetList;

		PmxMorph()
		{
			Panel = 4;
			Kind = OffsetKind::Vertex;
		}

		~PmxMorph()
		{
			for (auto & item : OffsetList)
			{
				delete item;
			}
		}

		void ReadMorph(BinReader & reader, PmxReader * owner)
		{
			owner->ReadString(Name);
			owner->ReadString(NameE);
			Panel = ReadElement_Int32(reader, 1, true);
			Kind = (OffsetKind::Type)ReadElement_Int32(reader, 1, true);

			int num = ReadElement_Int32(reader, 4, true);
			for (int i = 0; i < num; ++i)
			{
				switch (Kind)
				{
				case OffsetKind::Group:
				case OffsetKind::Flip:
				{
					PmxGroupMorph * pMorph = new PmxGroupMorph;
					pMorph->Read(reader, owner);
					OffsetList.push_back(pMorph);
					break;
				}
				case OffsetKind::Vertex:
				{
					PmxVertexMorph * pMorph = new PmxVertexMorph;
					pMorph->Read(reader, owner);
					OffsetList.push_back(pMorph);
					break;
				}
				case OffsetKind::Bone:
				{
					PmxBoneMorph * pMorph = new PmxBoneMorph;
					pMorph->Read(reader, owner);
					OffsetList.push_back(pMorph);
					break;
				}
				case OffsetKind::UV:
				case OffsetKind::UVA1:
				case OffsetKind::UVA2:
				case OffsetKind::UVA3:
				case OffsetKind::UVA4:
				{
					PmxUVMorph * pMorph = new PmxUVMorph;
					pMorph->Read(reader, owner);
					OffsetList.push_back(pMorph);
					break;
				}
				case OffsetKind::Material:
				{
					PmxMaterialMorph * pMorph = new PmxMaterialMorph;
					pMorph->Read(reader, owner);
					OffsetList.push_back(pMorph);
					break;
				}
				case OffsetKind::Impulse:
				{
					PmxImpulseMorph * pMorph = new PmxImpulseMorph;
					pMorph->Read(reader, owner);
					OffsetList.push_back(pMorph);
					break;
				}
				}
			}
		}
	};

	struct ElementType
	{
		enum Type
		{
			Bone,
			Morph
		};
	};

	struct NodeElement
	{
		ElementType::Type	EleType;
		int					Index;

		void Read(BinReader & reader, PmxReader * owner)
		{
			EleType = (ElementType::Type)reader.Read<uint8_t>();
			switch (EleType)
			{
			case ElementType::Bone:
				Index = ReadElement_Int32(reader, owner->BoneSize, true);
				return;

			case ElementType::Morph:
				Index = ReadElement_Int32(reader, owner->MorphSize, true);
				return;
			}
		}
	};

	struct PmxNode
	{
		std::string					Name, NameE;
		bool						SystemNode;
		std::vector<NodeElement>	ElementList;

		void ReadNode(BinReader & reader, PmxReader * owner)
		{
			owner->ReadString(Name);
			owner->ReadString(NameE);
			SystemNode = reader.Read<uint8_t>() != 0;

			int num = ReadElement_Int32(reader, 4, true);
			ElementList.resize(num);

			for (auto & item : ElementList)
			{
				item.Read(reader, owner);
			}
		}
	};

	struct PmxBodyPassGroup
	{
		static const int	Flags_Length = 16;
		bool				Flags[Flags_Length];

		void FromFlagBits(uint16_t bits)
		{
			uint16_t num = 1;
			for (int i = 0; i < Flags_Length; ++i)
			{
				Flags[i] = (bits & (num << i)) <= 0;
			}
		}
	};

	struct BoxKind
	{
		enum Type
		{
			Sphere,
			Box,
			Capsule
		};
	};

	struct ModeType
	{
		enum Type
		{
			Static,
			Dynamic,
			DynamicWithBone
		};
	};

	struct PmxBody
	{
		std::string			Name, NameE;
		int					Bone, Group;
		PmxBodyPassGroup	PassGroup;
		BoxKind::Type		BoxType;
		Vector3				BoxSize, Position, Rotation;
		float				Mass, PositionDamping, RotationDamping, Restitution, Friction;
		ModeType::Type		Mode;

		void ReadBody(BinReader & reader, PmxReader * owner)
		{
			owner->ReadString(Name);
			owner->ReadString(NameE);
			Bone = ReadElement_Int32(reader, owner->BoneSize, true);
			Group = ReadElement_Int32(reader, 1, true);
			uint16_t bits = (uint16_t)ReadElement_Int32(reader, 2, false);
			PassGroup.FromFlagBits(bits);
			BoxType = (BoxKind::Type)reader.Read<uint8_t>();
			BoxSize.Read(reader);
			Position.Read(reader);
			Rotation.Read(reader);
			Mass = reader.Read<float>();
			Vector4 vector;
			vector.Read(reader);
			PositionDamping = vector.X;
			RotationDamping = vector.Y;
			Restitution = vector.Z;
			Friction = vector.W;
			Mode = (ModeType::Type)reader.Read<uint8_t>();
		}
	};

	struct JointKind
	{
		enum Type
		{
			Sp6DOF,
			G6DOF,
			P2P,
			ConeTwist,
			Slider,
			Hinge
		};
	};

	struct PmxJoint
	{
		std::string			Name, NameE;
		JointKind::Type		Kind;
		int					BodyA, BodyB;
		Vector3				Position, Rotation;
		Vector3				Limit_MoveLow, Limit_MoveHigh, Limit_AngleLow, Limit_AngleHigh;
		Vector3				SpConst_Move, SpConst_Rotate;

		void ReadJoint(BinReader & reader, PmxReader * owner)
		{
			owner->ReadString(Name);
			owner->ReadString(NameE);
			Kind = (JointKind::Type)reader.Read<uint8_t>();
			BodyA = ReadElement_Int32(reader, owner->BodySize, true);
			BodyB = ReadElement_Int32(reader, owner->BodySize, true);
			Position.Read(reader);
			Rotation.Read(reader);
			Limit_MoveLow.Read(reader);
			Limit_MoveHigh.Read(reader);
			Limit_AngleLow.Read(reader);
			Limit_AngleHigh.Read(reader);
			SpConst_Move.Read(reader);
			SpConst_Rotate.Read(reader);
		}
	};

	struct ShapeKind
	{
		enum Type
		{
			TriMesh,
			Rope
		};
	};

	struct SoftBodyFlags
	{
		enum Type
		{
			GenerateBendingLinks = 1,
			GenerateClusters = 2,
			RandomizeConstraints = 4
		};
	};

	struct SoftBodyConfig
	{
		int AeroModel;
		float VCF;
		float DP;
		float DG;
		float LF;
		float PR;
		float VC;
		float DF;
		float MT;
		float CHR;
		float KHR;
		float SHR;
		float AHR;
		float SRHR_CL;
		float SKHR_CL;
		float SSHR_CL;
		float SR_SPLT_CL;
		float SK_SPLT_CL;
		float SS_SPLT_CL;
		int V_IT;
		int P_IT;
		int D_IT;
		int C_IT;

		SoftBodyConfig()
		{
			Clear();
		}

		void Clear()
		{
			AeroModel = 0;
			VCF = 1;
			DP = 0;
			DG = 0;
			LF = 0;
			PR = 0;
			VC = 0;
			DF = 0.2f;
			MT = 0;
			CHR = 1;
			KHR = 0.1f;
			SHR = 1;
			AHR = 0.7f;
			SRHR_CL = 0.1f;
			SKHR_CL = 1;
			SSHR_CL = 0.5f;
			SR_SPLT_CL = 0.5f;
			SK_SPLT_CL = 0.5f;
			SS_SPLT_CL = 0.5f;
			V_IT = 0;
			P_IT = 1;
			D_IT = 0;
			C_IT = 4;
		}
	};

	struct SoftBodyMaterialConfig
	{
		float LST;
		float AST;
		float VST;

		SoftBodyMaterialConfig()
		{
			Clear();
		}

		void Clear()
		{
			LST = 1;
			AST = 1;
			VST = 1;
		}
	};

	struct BodyAnchor
	{
		int		Body;
		bool	IsNear;
		int		Vertex;

		bool operator < (const BodyAnchor & other) const
		{
			if (Body == other.Body)
			{
				return Vertex < other.Vertex;
			}
			return Body < other.Body;
		}
	};

	struct VertexPin
	{
		int		Vertex;

		bool operator < (const VertexPin & other) const
		{
			return Vertex < other.Vertex;
		}
	};

	struct PmxSoftBody
	{
		std::string				Name, NameE;
		ShapeKind::Type			Shape;
		int						Material, Group;
		PmxBodyPassGroup		PassGroup;
		SoftBodyFlags::Type		Flags;
		int						BendingLinkDistance, ClusterCount;
		float					TotalMass, Margin;
		SoftBodyConfig			Config;
		SoftBodyMaterialConfig	MaterialConfig;
		std::vector<BodyAnchor>	BodyAnchorList;
		std::vector<VertexPin>	VertexPinList;

		void ReadSoftBody(BinReader & reader, PmxReader * owner)
		{
			owner->ReadString(Name);
			owner->ReadString(NameE);
			Shape = (ShapeKind::Type)ReadElement_Int32(reader, 1, true);
			Material = ReadElement_Int32(reader, owner->MaterialSize, true);
			Group = ReadElement_Int32(reader, 1, true);
			uint16_t bits = (uint16_t)ReadElement_Int32(reader, 2, false);
			PassGroup.FromFlagBits(bits);
			Flags = (SoftBodyFlags::Type)ReadElement_Int32(reader, 1, true);
			BendingLinkDistance = ReadElement_Int32(reader, 4, true);
			ClusterCount = ReadElement_Int32(reader, 4, true);
			TotalMass = reader.Read<float>();
			Margin = reader.Read<float>();
			Config.AeroModel = ReadElement_Int32(reader, 4, true);
			Config.VCF = reader.Read<float>();
			Config.DP = reader.Read<float>();
			Config.DG = reader.Read<float>();
			Config.LF = reader.Read<float>();
			Config.PR = reader.Read<float>();
			Config.VC = reader.Read<float>();
			Config.DF = reader.Read<float>();
			Config.MT = reader.Read<float>();
			Config.CHR = reader.Read<float>();
			Config.KHR = reader.Read<float>();
			Config.SHR = reader.Read<float>();
			Config.AHR = reader.Read<float>();
			Config.SRHR_CL = reader.Read<float>();
			Config.SKHR_CL = reader.Read<float>();
			Config.SSHR_CL = reader.Read<float>();
			Config.SR_SPLT_CL = reader.Read<float>();
			Config.SK_SPLT_CL = reader.Read<float>();
			Config.SS_SPLT_CL = reader.Read<float>();
			Config.V_IT = ReadElement_Int32(reader, 4, true);
			Config.P_IT = ReadElement_Int32(reader, 4, true);
			Config.D_IT = ReadElement_Int32(reader, 4, true);
			Config.C_IT = ReadElement_Int32(reader, 4, true);
			MaterialConfig.LST = reader.Read<float>();
			MaterialConfig.AST = reader.Read<float>();
			MaterialConfig.VST = reader.Read<float>();

			int sz = ReadElement_Int32(reader, 4, true);
			BodyAnchorList.resize(sz);
			for (int i = 0; i < sz; ++i)
			{
				BodyAnchor & item = BodyAnchorList[i];
				int bodyID = ReadElement_Int32(reader, owner->BodySize, true);
				int vID = ReadElement_Int32(reader, owner->VertexSize, true);
				int nNear = ReadElement_Int32(reader, 1, true);

				item.Body = bodyID;
				item.Vertex = vID;
				item.IsNear = nNear != 0;
			}

			sz = ReadElement_Int32(reader, 4, true);
			VertexPinList.resize(sz);
			for (int j = 0; j < sz; ++j)
			{
				VertexPin & item = VertexPinList[j];
				item.Vertex = ReadElement_Int32(reader, owner->VertexSize, true);
			}
			NormalizeBodyAnchorAndVertexPin();
		}

		void NormalizeBodyAnchorAndVertexPin()
		{
			std::set<BodyAnchor> optAnchor;

			for (const auto & item : BodyAnchorList)
			{
				if (optAnchor.find(item) == optAnchor.end())
					optAnchor.insert(item);
			}
			BodyAnchorList.clear();
			for (const auto & item : optAnchor)
			{
				BodyAnchorList.push_back(item);
			}

			std::set<VertexPin> optPin;
			for (const auto & item : VertexPinList)
			{
				if (optPin.find(item) == optPin.end())
					optPin.insert(item);
			}
			VertexPinList.clear();
			for (const auto & item : optPin)
			{
				VertexPinList.push_back(item);
			}
		}
	};

public:
	PmxReader(const void * pBuffer, size_t len)
		: m_Reader(pBuffer, len)
	{
		ReadHeader();
		ReadModelInfo();
		ReadVertexInfo();
		ReadFaceInfo();
		ReadTextureInfo();
		ReadMaterialInfo();
		ReadBoneInfo();
		ReadMorphInfo();
		ReadNodeInfo();
		ReadBodyInfo();
		ReadJointInfo();
		ReadSoftBodyInfo();
	}

private:
	void ReadHeader()
	{
		static std::string PmxKey_v1 = "Pmx ";
		static std::string PmxKey = "PMX ";

		std::string strMagic;
		m_Reader.ReadString(4, strMagic);
		if (strMagic == PmxKey_v1)
		{
			Ver = 1;
			m_Reader.Skip(4);
		}
		else
		{
			if (strMagic != PmxKey)
			{
				throw std::wstring(L"文件头错误");
			}

			Ver = m_Reader.Read<float>();
		}

		if (Ver > 2.1f)
		{
			throw std::wstring(L"不支持大于 2.1 版本的文件");
		}

		ReadElementFormat();
	}

	void ReadElementFormat()
	{
		int szRead = ReadElement_Int32(m_Reader, 1, true);
		BinReader::Data buffer;
		m_Reader.ReadData(szRead, buffer);

		int num2 = 0;
		if (Ver <= 1)
		{
			VertexSize = buffer[num2++];
			BoneSize = buffer[num2++];
			MorphSize = buffer[num2++];
			MaterialSize = buffer[num2++];
			BodySize = buffer[num2++];
		}
		else
		{
			StringEnc = (StringEncType::Type)buffer[num2++];
			UVACount = buffer[num2++];
			VertexSize = buffer[num2++];
			TexSize = buffer[num2++];
			MaterialSize = buffer[num2++];
			BoneSize = buffer[num2++];
			MorphSize = buffer[num2++];
			BodySize = buffer[num2++];
		}
	}

	void ReadString(std::string & str)
	{
		if (Ver <= 1)
		{
			ReadString_v1(str);
			return;
		}

		if (StringEnc == StringEncType::UTF8)
		{
			ReadString_v1(str);
			return;
		}
		return ReadString_v2(str);
	}

	void ReadString_v1(std::string & str)
	{
		int count = m_Reader.Read<int>();
		if (count > 0)
		{
			m_Reader.ReadString(count, str);
		}
	}

	void ReadString_v2(std::string & str)
	{
		int count = m_Reader.Read<int>();
		if (count > 0)
		{
			BinReader::Data dat;
			m_Reader.ReadData(count, dat);

			std::wstring wstr((wchar_t*)dat.data(), count / 2);
			Platform_Utf16To8(wstr, str);
		}
	}

	void ReadModelInfo()
	{
		ReadString(ModelName);
		ReadString(ModelNameE);
		ReadString(Comment);
		ReadString(CommentE);
	}

	void ReadVertexInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);
		VertexList.resize(num);

		for (auto & item : VertexList)
		{
			item.ReadVertex(m_Reader, this);
		}
	}

	void ReadFaceInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);
		FaceList.reserve(num);

		for (int i = 0; i < num; ++i)
		{
			int fid = ReadElement_Int32(m_Reader, VertexSize, false);
			FaceList.push_back(fid);
		}
	}

	void ReadTextureInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);

		std::string str;
		for (int i = 0; i < num; ++i)
		{
			ReadString(str);
			NameToIndex[str] = i;
			IndexToName[i] = str;
		}
	}

	void ReadMaterialInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);

		MaterialList.resize(num);

		for (auto & item : MaterialList)
		{
			item.ReadMaterial(m_Reader, this);
		}
	}

	void ReadBoneInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);

		BoneList.resize(num);

		for (auto & item : BoneList)
		{
			item.ReadBone(m_Reader, this);
		}

		int i = 0;
		for (auto & item : BoneList)
		{
			if (item.Parent < 0)
			{
				RootBoneList.push_back(i);
			}
			else
			{
				item.RefParent = &BoneList[item.Parent];
			}
			++i;
		}

		for (auto & item : BoneList)
		{
			item.CalcHierarchy();
		}
	}

	void ReadMorphInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);

		MorphList.resize(num);

		for (auto & item : MorphList)
		{
			item.ReadMorph(m_Reader, this);
		}
	}

	void ReadNodeInfo()
	{
		static std::string s_Exp;
		static bool s_tmp = Platform_Utf16To8(L"表情", s_Exp).empty();

		int num = ReadElement_Int32(m_Reader, 4, true);

		NodeList.resize(num);

		for (int i = 0; i < num; ++i)
		{
			PmxNode & item = NodeList[i];
			item.ReadNode(m_Reader, this);

			if (item.SystemNode)
			{
				if (item.Name == "Root")
				{
					RootNode = i;
				}
				else if (item.Name == s_Exp)
				{
					ExpNode = i;
				}
			}
		}
	}

	void ReadBodyInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);

		BodyList.resize(num);

		for (auto & item : BodyList)
		{
			item.ReadBody(m_Reader, this);
		}
	}

	void ReadJointInfo()
	{
		int num = ReadElement_Int32(m_Reader, 4, true);

		JointList.resize(num);

		for (auto & item : JointList)
		{
			item.ReadJoint(m_Reader, this);
		}
	}

	void ReadSoftBodyInfo()
	{
		if (Ver >= 2.1f)
		{
			int num = ReadElement_Int32(m_Reader, 4, true);

			SoftBodyList.resize(num);

			for (auto & item : SoftBodyList)
			{
				item.ReadSoftBody(m_Reader, this);
			}
		}
	}

private:
	static int ReadElement_Int32(BinReader & reader, int bufSize, bool bSigned = true)
	{
		int num = 0;

		BinReader::Data buffer;
		reader.ReadData(bufSize, buffer);

		switch (bufSize)
		{
		case 1:
			if (!bSigned)
			{
				return buffer[0];
			}
			return (char)buffer[0];

		case 2:
			if (!bSigned)
			{
				return *(uint16_t*)buffer.data();
			}
			return *(int16_t*)buffer.data();

		case 3:
			return num;

		case 4:
			return *(int32_t*)buffer.data();
		}
		return num;
	}

public:
	std::string								ModelName, ModelNameE, Comment, CommentE;
	int										RootNode, ExpNode;

	std::vector<PmxVertex>					VertexList;
	std::vector<int>						FaceList;

	std::unordered_map<std::string, int>	NameToIndex;
	std::map<int, std::string>				IndexToName;

	std::vector<PmxMaterial>				MaterialList;
	std::vector<PmxBone>					BoneList;
	std::vector<int>						RootBoneList;
	std::vector<PmxMorph>					MorphList;
	std::vector<PmxNode>					NodeList;
	std::vector<PmxBody>					BodyList;
	std::vector<PmxJoint>					JointList;
	std::vector<PmxSoftBody>				SoftBodyList;

private:
	BinReader								m_Reader;

	struct StringEncType
	{
		enum Type
		{
			UTF16,
			UTF8
		};
	};

	float									Ver;
	int										VertexSize;
	int										BoneSize;
	int										MorphSize;
	int										MaterialSize;
	int										BodySize;
	int										TexSize;
	int										UVACount;
	StringEncType::Type						StringEnc;
};

#endif