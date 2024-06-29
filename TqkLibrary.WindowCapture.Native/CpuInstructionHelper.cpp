#include "CpuInstructionHelper.hpp"

static CIH_Info CIH_FeatureInformation{};
static CIH_Info CIH_ExtendedFeatureBit{};
static CIH_Info CIH_ExtendedExtendedProcessorInfoAndFeatureBit{};




void CpuInstructionHelper_Init()
{
	CIH_Info info;
	cpuid(info.infos, 0);
	int nIds = info.infos[0];

	cpuid(info.infos, 0x80000000);
	unsigned nExIds = info.infos[0];

	//  Detect Features
	if (nIds >= 0x00000001) {
		cpuid(CIH_FeatureInformation.infos, 0x00000001);
	}
	if (nIds >= 0x00000007) {//Extended Features
		cpuid(CIH_ExtendedFeatureBit.infos, 0x00000007);
	}
	if (nExIds >= 0x80000001) {
		cpuid(CIH_ExtendedExtendedProcessorInfoAndFeatureBit.infos, 0x80000001);
	}
}
BOOL CheckFlag(CIH_Info& info, CIH_Info& check)
{
	BOOL result = FALSE;
	for (int i = 0; i < 4; i++)
	{
		if (check.infos[i])
		{
			if (info.infos[i] & check.infos[i])
			{
				result = TRUE;
			}
			else
			{
				result = FALSE;
				break;
			}
		}
	}
	return result;
}


BOOL CpuInstructionHelper_IsSupport_FeatureInformation(CIH_Info check)
{
	return CheckFlag(CIH_FeatureInformation, check);
}
BOOL CpuInstructionHelper_IsSupport_ExtendedFeatureBit(CIH_Info check)
{
	return CheckFlag(CIH_ExtendedFeatureBit, check);
}
BOOL CpuInstructionHelper_IsSupport_ExtendedExtendedProcessorInfoAndFeatureBit(CIH_Info check)
{
	return CheckFlag(CIH_ExtendedExtendedProcessorInfoAndFeatureBit, check);
}