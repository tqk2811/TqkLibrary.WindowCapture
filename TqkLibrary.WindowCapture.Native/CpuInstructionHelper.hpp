#ifndef _H_CpuInstructionHelper_H_
#define _H_CpuInstructionHelper_H_

#ifdef _WIN32

//  Windows
#include <windows.h>
#define cpuid(info, x) __cpuidex(info, x, 0)



#else

typedef unsigned int        UINT32;
typedef int					INT32;
typedef unsigned char       BYTE;
typedef int					BOOL;

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

//  GCC Intrinsics
#include <cpuid.h>
void cpuid(int info[4], int InfoType) {
	__cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}

#endif

typedef union CIH_Info
{
	struct
	{
		UINT32 EAX;
		UINT32 EBX;
		UINT32 ECX;
		UINT32 EDX;
	};
	INT32 infos[4];
	BYTE datas[16];
} CIH_Info;
static CIH_Info& operator| (const CIH_Info left, const CIH_Info right)
{
	CIH_Info result{};
	for (int i = 0; i < 4; i++)
	{
		result.infos[i] = left.infos[i] | right.infos[i];
	}
	return result;
}


//https://en.wikipedia.org/wiki/CPUID#EAX=1:_Processor_Info_and_Feature_Bits
// 
//CPUID EAX=1: Feature Information in EDX and ECX
#define CIH_FEATURES_MMX			CIH_Info{       0,       0,        0, 1 << 23 }
#define CIH_FEATURES_SSE			CIH_Info{       0,       0,        0, 1 << 25 }
#define CIH_FEATURES_SSE2			CIH_Info{       0,       0,        0, 1 << 26 }
#define CIH_FEATURES_SSE3			CIH_Info{       0,       0,  1 <<  2,       0 }
#define CIH_FEATURES_SSSE3			CIH_Info{       0,       0,  1 <<  9,       0 }
#define CIH_FEATURES_SSE41			CIH_Info{       0,       0,  1 << 19,       0 }
#define CIH_FEATURES_SSE42			CIH_Info{       0,       0,  1 << 20,       0 }
#define CIH_FEATURES_AES			CIH_Info{       0,       0,  1 << 25,       0 }
#define CIH_FEATURES_AVX			CIH_Info{       0,       0,  1 << 28,       0 }
#define CIH_FEATURES_FMA3			CIH_Info{       0,       0,  1 << 12,       0 }
#define CIH_FEATURES_RDRAND			CIH_Info{       0,       0,  1 << 30,       0 }



//CPUID EAX=7,ECX=0: Extended feature bits in EBX, ECX and EDX
#define CIH_EX_FEATURES_AVX2		CIH_Info{       0, 1 <<  5,       0,       0 }

#define CIH_EX_FEATURES_BMI1		CIH_Info{       0, 1 <<  3,       0,       0 }
#define CIH_EX_FEATURES_BMI2		CIH_Info{       0, 1 <<  8,       0,       0 }
#define CIH_EX_FEATURES_ADX			CIH_Info{       0, 1 << 19,       0,       0 }
#define CIH_EX_FEATURES_SHA			CIH_Info{       0, 1 << 29,       0,       0 }
#define CIH_EX_FEATURES_PREFETCHWT1	CIH_Info{       0,       0, 1 <<  0,       0 }

#define CIH_EX_FEATURES_AVX512F		CIH_Info{       0, 1 << 16,       0,       0 }
#define CIH_EX_FEATURES_AVX512CD  	CIH_Info{       0, 1 << 28,       0,       0 }
#define CIH_EX_FEATURES_AVX512PF  	CIH_Info{       0, 1 << 26,       0,       0 }
#define CIH_EX_FEATURES_AVX512ER  	CIH_Info{       0, 1 << 27,       0,       0 }
#define CIH_EX_FEATURES_AVX512VL  	CIH_Info{       0, 1 << 31,       0,       0 }
#define CIH_EX_FEATURES_AVX512BW  	CIH_Info{       0, 1 << 30,       0,       0 }
#define CIH_EX_FEATURES_AVX512DQ  	CIH_Info{       0, 1 << 17,       0,       0 }
#define CIH_EX_FEATURES_AVX512IFMA	CIH_Info{       0, 1 << 21,       0,       0 }
#define CIH_EX_FEATURES_AVX512VBMI	CIH_Info{       0,       0, 1 <<  1,       0 }



//EAX=80000001h: Extended Processor Info and Feature Bits
//CPUID EAX=80000001h: Feature bits in EDX and ECX
#define CIH_EX_PROCESSOR_INFO_AND_FEATURES_BIT_x64			CIH_Info{       0,       0,       0, 1 << 29 }
#define CIH_EX_PROCESSOR_INFO_AND_FEATURES_BIT_ABM			CIH_Info{       0,       0, 1 <<  5,       0 }
#define CIH_EX_PROCESSOR_INFO_AND_FEATURES_BIT_SSE4a		CIH_Info{       0,       0, 1 <<  6,       0 }
#define CIH_EX_PROCESSOR_INFO_AND_FEATURES_BIT_FMA4			CIH_Info{       0,       0, 1 << 16,       0 }
#define CIH_EX_PROCESSOR_INFO_AND_FEATURES_BIT_XOP			CIH_Info{       0,       0, 1 << 11,       0 }


void CpuInstructionHelper_Init();
BOOL CpuInstructionHelper_IsSupport_FeatureInformation(CIH_Info FEATURES);
BOOL CpuInstructionHelper_IsSupport_ExtendedFeatureBit(CIH_Info EX_FEATURES);
BOOL CpuInstructionHelper_IsSupport_ExtendedExtendedProcessorInfoAndFeatureBit(CIH_Info EX_PROCESSOR_INFO_AND_FEATURES_BIT);
#endif // !_H_CpuInstructionHelper_H_
