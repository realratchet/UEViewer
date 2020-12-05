#include "Core.h"
#include "UnCore.h"
#include "UnObject.h"

#if UNREAL4

/*-----------------------------------------------------------------------------
	UE4.26+ unversioned property table
-----------------------------------------------------------------------------*/

// Make a bitmask from provided bit values.
template<int Arg>
constexpr uint32 MakeBitmask()
{
	static_assert(Arg >= 0 && Arg <= 31, "Bad 'Arg'");
	return 1 << Arg;
}

template<int Arg1, int Arg2, int... Args>
constexpr uint32 MakeBitmask()
{
	return MakeBitmask<Arg1>() | MakeBitmask<Arg2, Args...>();
}

// Subtract Offset from each input constant and make a bitmask from it.
// Bitmask from a single constant makes nothing. Bitmask of <N,N+1> will result 1.
template<int Offset>
constexpr int MakeBitmaskWithOffset()
{
    return 0;
}

template<int Offset, int Arg2, int... Args>
constexpr int MakeBitmaskWithOffset()
{
	return MakeBitmask<Arg2 - (Offset + 1), Args - (Offset + 1)...>();
}

struct PropInfo
{
	// Name of the property, should exist in class's property table (BEGIN_PROP_TABLE...)
	// If starts with '#', the property is ignored, and the name is actually a type name.
	const char* Name;
	int			Index;
	// When PropMask is not zero, more than one property is specified with this entry.
	// PropMask of value '1' means 2 properties: { Index, Index + 1 }
	uint32		PropMask;
};

#define BEGIN(type)					{ type,  0,        0 },	// store class name as field name, index is not used
#define MAP(name,index)				{ #name, index,    0 },	// field specification
#define END							{ NULL,  0,        0 },	// end of class - mark with NULL name

// Multi-property "drop" instructions
#define DROP_INT8(index, ...)		{ "#int8", index, MakeBitmaskWithOffset<index,__VA_ARGS__>() },
#define DROP_INT64(index, ...)		{ "#int64", index, MakeBitmaskWithOffset<index,__VA_ARGS__>() },
#define DROP_VECTOR3(index, ...)	{ "#vec3", index,  MakeBitmaskWithOffset<index,__VA_ARGS__>() },
#define DROP_VECTOR4(index, ...)	{ "#vec4", index,  MakeBitmaskWithOffset<index,__VA_ARGS__>() },	// FQuat, FGuid etc

#define DROP_OBJ_ARRAY(index)		{ "#arr_int32", index, 0 }, // TArray<UObject*>

static const PropInfo PropData[] =
{
BEGIN("UStaticMesh4")
	DROP_INT64(0)						// FPerPlatformInt MinLOD - serialized as 2x int32, didn't find why
	MAP(StaticMaterials, 2)
	MAP(LightmapUVDensity, 3)
	DROP_INT8(9, 10, 11, 12, 13, 14, 15, 16)
	MAP(Sockets, 17)
	DROP_VECTOR3(18)					// FVector PositiveBoundsExtension
	DROP_VECTOR3(19)					// FVector NegativeBoundsExtension
	MAP(ExtendedBounds, 20)
	DROP_OBJ_ARRAY(22)					// TArray<UAssetUserData*> AssetUserData
END

BEGIN("USkeletalMesh4")
	MAP(Skeleton, 0)
	DROP_VECTOR3(3)						// PositiveBoundsExtension
	DROP_VECTOR3(4)						// PositiveBoundsExtension
	MAP(Materials, 5)
	MAP(LODInfo, 7)
	DROP_INT64(8)						// FPerPlatformInt MinLod
	DROP_INT64(9)						// FPerPlatformBool DisableBelowMinLodStripping
	DROP_INT8(10, 11, 12, 13, 14, 16)
	MAP(bHasVertexColors, 15)
	MAP(MorphTargets, 21)
	DROP_OBJ_ARRAY(23)					// TArray<UClothingAssetBase*> MeshClothingAssets
	MAP(SamplingInfo, 24)
	DROP_OBJ_ARRAY(25)					// TArray<UAssetUserData*> AssetUserData
	MAP(Sockets, 26)
	MAP(SkinWeightProfiles, 27)
END

BEGIN("UTexture2D")
	DROP_INT8(2)						// uint8 bTemporarilyDisableStreaming:1
	DROP_INT8(3)						// TEnumAsByte<enum TextureAddress> AddressX
	DROP_INT8(4)						// TEnumAsByte<enum TextureAddress> AddressY
	DROP_INT64(5)						// FIntPoint ImportedSize
END

BEGIN("UTexture3")
	DROP_VECTOR4(0)						// FGuid LightingGuid
	// UTexture
	DROP_INT8(2, 3, 5, 8, 9, 10, 11, 12, 13)
END

BEGIN("UMaterialInstance")
	MAP(Parent, 9)
	DROP_INT8(10, 11)
	MAP(ScalarParameterValues, 12)
	MAP(VectorParameterValues, 13)
	MAP(TextureParameterValues, 14)
	MAP(BasePropertyOverrides, 17)
	MAP(StaticParameters, 18)
	DROP_OBJ_ARRAY(20)					//todo: TArray<UObject*> CachedReferencedTextures
END

BEGIN("UMaterialInterface")
	MAP(LightmassSettings, 1)
	MAP(TextureStreamingData, 2)
	DROP_OBJ_ARRAY(3)					// TArray<UAssetUserData*> AssetUserData
END

BEGIN("UStreamableRenderAsset")
	DROP_INT64(0)						// double ForceMipLevelsToBeResidentTimestamp
	DROP_INT8(4, 5, 6, 7, 8, 9)
END

BEGIN("FSkeletalMeshLODInfo")
	DROP_INT64(0)						// FPerPlatformFloat ScreenSize
	MAP(LODHysteresis, 1)
	MAP(LODMaterialMap, 2)
	MAP(BuildSettings, 3)
	MAP(ReductionSettings, 4)
	MAP(BonesToRemove, 5)
	MAP(BonesToPrioritize, 6)
	MAP(SourceImportFilename, 10)
	DROP_INT8(11, 12, 13, 14, 15)
END

BEGIN("FSkeletalMeshBuildSettings")
	DROP_INT8(0, 1, 2, 3, 4, 5, 6, 7)
END

BEGIN("FSkeletalMeshOptimizationSettings")
	DROP_INT8(0, 6, 7, 8, 9, 10, 11, 12, 16, 18, 19)
END

BEGIN("FSkinWeightProfileInfo")
	MAP(Name, 0)
	DROP_INT64(1)						// FPerPlatformBool DefaultProfile
	DROP_INT64(2)						// FPerPlatformInt DefaultProfileFromLODIndex
END

BEGIN("FMaterialInstanceBasePropertyOverrides")
	MAP(bOverride_OpacityMaskClipValue, 0)
	MAP(bOverride_BlendMode, 1)
	DROP_INT8(2, 3, 4, 7, 8, 10)
	MAP(bOverride_TwoSided, 5)
	MAP(TwoSided, 6)
	MAP(BlendMode, 9)
	MAP(OpacityMaskClipValue, 11)
END

BEGIN("FLightmassMaterialInterfaceSettings")
	DROP_INT8(3, 4, 5, 6, 7)
END
};

#undef MAP
#undef BEGIN
#undef END

struct ParentInfo
{
	const char* ThisName;
	const char* ParentName; // this could be a UE4 class name which is NOT defined here
	int NumProps;
};

static const ParentInfo ParentData[] =
{
	// Parent classes should be defined after children, so the whole table could be iterated with a single pass
	{ "UTextureCube4", "UTexture3", 0 },
	{ "UTexture2D", "UTexture3", 6 },
	{ "UTexture3", "UStreamableRenderAsset", 15 },
	{ "USkeletalMesh4", "UStreamableRenderAsset", 28 },
	{ "UStaticMesh4", "UStreamableRenderAsset", 25 },
	{ "UMaterialInstanceConstant", "UMaterialInstance", 1 }, // just 1 UObject* property
	{ "UMaterialInstance", "UMaterialInterface", 21 },
	{ "FStaticSwitchParameter", "FStaticParameterBase", 1 },
	{ "FStaticComponentMaskParameter", "FStaticParameterBase", 4 },
	{ "FStaticTerrainLayerWeightParameter", "FStaticParameterBase", 2 },
	{ "FStaticMaterialLayersParameter", "FStaticParameterBase", 1 },
};

const char* CTypeInfo::FindUnversionedProp(int PropIndex, int& OutArrayIndex) const
{
	guard(CTypeInfo::FindUnversionedProp);

	OutArrayIndex = 0;

	const char* CurrentClassName = Name;
	const CTypeInfo* CurrentType = this;

	//todo: can optimize with checking if class name starts with 'U', but: CTypeInfo::Name skips 'U', plus there could be FSomething (structs)
	for (const ParentInfo& Parent : ParentData)
	{
		// Fast reject
		if (PropIndex < Parent.NumProps)
			continue;
		// Compare types
		if (CurrentType)
		{
			if (!CurrentType->IsA(Parent.ThisName + 1))
				continue;
		}
		else
		{
			if (strcmp(CurrentClassName, Parent.ThisName) != 0)
				continue;
		}
		// Types are matching, redirect to parent
		CurrentClassName = Parent.ParentName;
		CurrentType = FindStructType(Parent.ParentName);
		PropIndex -= Parent.NumProps;
	}

	// Find a field
	const PropInfo* p;
	const PropInfo* end;

	p = PropData;
	end = PropData + ARRAY_COUNT(PropData);

	bool bClassFound = false;
	while (p < end)
	{
		// Note: StrucType could correspond to a few classes from the list about
		// because of inheritance, so don't "break" a loop when we've scanned some class, check
		// other classes too
		bool IsOurClass;
		if (CurrentType)
			IsOurClass = CurrentType->IsA(p->Name + 1);
		else
			IsOurClass = stricmp(p->Name, CurrentClassName) == 0;

		while (++p < end && p->Name)
		{
			if (!IsOurClass) continue;
			if (p->PropMask)
			{
				uint32 IndexWithOffset = PropIndex - p->Index;
				if (IndexWithOffset > 32)
				{
					// 'uint' here, so negative values will also go here
					continue;
				}
				if ((IndexWithOffset == 0) ||						// we're implicitly storing first property
					((1 << (IndexWithOffset - 1)) & p->PropMask))	// and explicitly up to 32 properties more
				{
					return p->Name;
				}
			}
			else if (p->Index == PropIndex)
			{
				//todo: not supporting arrays here, arrays relies on class' property table matching layout
				return p->Name;
			}
		}
		if (IsOurClass)
		{
			// the class has been verified, and we didn't find a property
			bClassFound = true;
			break;
		}
		// skip END marker
		p++;
	}

	if (bClassFound)
	{
		// We have a declaration of the class, so don't fall back to PROP declaration
		//todo: review later, actually can "return NULL" from inside the loop body
		return NULL;
	}

	// The property not found. Try using CTypeInfo properties, assuming their layout matches UE
	int CurrentPropIndex = 0;
	assert(CurrentType);
	for (int Index = 0; Index < CurrentType->NumProps; Index++)
	{
		const CPropInfo& Prop = CurrentType->Props[Index];
		if (Prop.Count >= 2)
		{
			// Static array, should count each item as a separate property
			if (CurrentPropIndex + Prop.Count > PropIndex)
			{
				// The property is located inside this array
				OutArrayIndex = PropIndex - CurrentPropIndex;
				return Prop.Name;
			}
			CurrentPropIndex += Prop.Count;
		}
		else
		{
			// The same code, but works as Count == 1 for any values
			if (CurrentPropIndex == PropIndex)
			{
				return Prop.Name;
			}
			CurrentPropIndex++;
		}
	}

	return NULL;
	unguard;
}

#endif // UNREAL4
