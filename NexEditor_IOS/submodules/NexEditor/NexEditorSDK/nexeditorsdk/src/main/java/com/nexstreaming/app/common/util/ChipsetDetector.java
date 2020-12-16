package com.nexstreaming.app.common.util;

import android.os.Build;

public class ChipsetDetector {

	public static enum ChipsetType {
		APQ8064, APQ8064AB, APQ8064T, Exynos4412, Exynos5410, UNKNOWN
	}
	
	public static enum Maker {
		samsung,asus,HTC,LGE,Sony,OPPO,Xiaomi,PANTECH,TCT
	}
	
	private ChipsetDetector() {
	}
	
	private enum ModelInfo {
		
	//  (1) Enum name			(2) Model name			(3) Maker		(4) ChipsetType				(5) Comment
	//	----------------------- ----------------------- --------------- -----------------------		-----------------------
		/*
		GT_I9505(				"GT-I9505",				Maker.samsung,	ChipsetType.APQ8064),		//Galaxy S4
		HTC_One(				"HTC One",				Maker.HTC,		ChipsetType.APQ8064), 		//HTC one
		Nexus_7( 				"Nexus 7",				Maker.asus,		ChipsetType.APQ8064), 		//Nexus 7
		Nexus_4( 				"Nexus 4",				Maker.LGE,		ChipsetType.APQ8064), 		//Nexus 4
		SGH_M919( 				"SGH-M919",				Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		SAMSUNG_SGH_I337( 		"SAMSUNG-SGH-I337",		Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		LG_F240L( 				"LG-F240L",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus G pro
		SCH_I545( 				"SCH-I545",				Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		C6602( 					"C6602",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia Z
		LG_F240S(				"LG-F240S",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus G pro
		LG_F180L(				"LG-F180L",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		SPH_L720(				"SPH-L720",				Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		X909(					"X909",					Maker.OPPO,		ChipsetType.APQ8064T), 		//Find 5
		C6603(					"C6603",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia Z
		SGH_I337M(				"SGH-I337M",			Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		HTC_One_801e(			"HTC One 801e",			Maker.HTC,		ChipsetType.APQ8064T), 		//HTC one
		LG_F240K(				"LG-F240K",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus G pro
		SC_04E(					"SC-04E",				Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		MI_2S(					"MI 2S",				Maker.Xiaomi,	ChipsetType.APQ8064T), 		//Xiaomi phone 2S
		LG_F180K(				"LG-F180K",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		SO_04E(					"SO-04E",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia A
		LG_E975(				"LG-E975",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		X909T(					"X909T",				Maker.OPPO,		ChipsetType.APQ8064T), 		//Find 5
		MI_2(					"MI 2",					Maker.Xiaomi,	ChipsetType.APQ8064), 		//Xiaomi Mi Two
		IM_A870S(				"IM-A870S",				Maker.PANTECH,	ChipsetType.APQ8064T), 		//Vega Iron
		LG_E980(				"LG-E980",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus G pro
		SAMSUNG_SGH_I537(		"SAMSUNG-SGH-I537",		Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 Active
		GT_I9295(				"GT-I9295",				Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 Active
		LG_F180S(				"LG-F180S",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		HTCONE(					"HTCONE",				Maker.HTC,		ChipsetType.APQ8064), 		//HTC one
		SGH_M919N(				"SGH-M919N",			Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4
		SOL22(					"SOL22",				Maker.Sony,		ChipsetType.APQ8064),		//Xperia UL
		IM_A870L(				"IM-A870L",				Maker.PANTECH,	ChipsetType.APQ8064T), 		//Vega Iron
		LG_E988(				"LG-E988",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus G pro
		IM_A870K(				"IM-A870K",				Maker.PANTECH,	ChipsetType.APQ8064T), 		//Vega Iron
		SO_02E(					"SO-02E",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia Z
		C5502(					"C5502",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia ZR
		LG_F220K(				"LG-F220K",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus GK
		MI_2SC(					"MI 2SC",				Maker.Xiaomi,	ChipsetType.APQ8064T), 		//Xiaomi phone 2S
		N1T(					"N1T",					Maker.OPPO,		ChipsetType.APQ8064T), 		//N1T
		LG_LS970(				"LG-LS970",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		C6502(					"C6502",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia ZL
		HTC_One_max_(			"HTC_One_max",			Maker.HTC,		ChipsetType.APQ8064T), 		//HTC One Max
		SCH_R970C(				"SCH-R970C",			Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		LG_V500(				"LG-V500",				Maker.LGE,		ChipsetType.APQ8064T), 		//G Pad 8.3 WiFi
		HTL22(					"HTL22",				Maker.HTC,		ChipsetType.APQ8064T), 		//HTC J One
		SCH_R970(				"SCH-R970",				Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE
		C6606(					"C6606",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia Z
		HTC_One_max(			"HTC One max",			Maker.HTC,		ChipsetType.APQ8064T), 		//HTC One Max
		GT_I9505G(				"GT-I9505G",			Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4 LTE Google Play 
		HTC6500LVW(				"HTC6500LVW",			Maker.HTC,		ChipsetType.APQ8064T), 		//HTC One
		C6503(					"C6503",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia ZL
		LG_F180(				"LG-F180",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		PadFone_2(				"PadFone 2",			Maker.asus,		ChipsetType.APQ8064), 		//PadFone 2
		L_04E(					"L-04E",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus G pro
		LG_E970(				"LG-E970",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G pro
		ONE_TOUCH_8020D(		"ONE TOUCH 8020D",		Maker.TCT,		ChipsetType.APQ8064T), 		//One Dual Sim
		IM_A850K(				"IM-A850K",				Maker.PANTECH,	ChipsetType.APQ8064), 		//Vega R3
		C6506(					"C6506",				Maker.Sony,		ChipsetType.APQ8064), 		//Xperia ZL
		HTC_Butterfly(			"HTC Butterfly",		Maker.HTC,		ChipsetType.APQ8064), 		//HTC Butterfly
		LG_E977(				"LG-E977",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		IM_A850L(				"IM-A850L",				Maker.PANTECH,	ChipsetType.APQ8064), 		//Vega R3
		LG_E986(				"LG-E986",				Maker.LGE,		ChipsetType.APQ8064T), 		//Optimus G Pro
		MI_2C(					"MI 2C",				Maker.Xiaomi,	ChipsetType.APQ8064), 		//Xiaomi Mi-Two CDMA
		SGP321(					"SGP321",				Maker.Sony,		ChipsetType.APQ8064), 		//Sony Xperia Tablet Z
		C5503(					"C5503",				Maker.Sony,		ChipsetType.APQ8064), 		//Sony Xperia ZR
		SGP312(					"SGP312",				Maker.Sony,		ChipsetType.APQ8064), 		//Sony Xperia Tablet Z WiFi
		HTX21(					"HTX21",				Maker.HTC,		ChipsetType.APQ8064), 		//INFOBAR A02 HTX21
		SGH_M919V(				"SGH-M919V",			Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S4
		L_01E(					"L-01E",				Maker.LGE,		ChipsetType.APQ8064), 		//Optimus G
		IM_A860S(				"IM-A860S",				Maker.PANTECH,	ChipsetType.APQ8064), 		//Vega No6
		D5503(					"D5503",				Maker.Sony,		ChipsetType.APQ8064), 		//Sony Xperia Tablet Z WiFi 
		IM_A850S(				"IM-A850S",				Maker.PANTECH,	ChipsetType.APQ8064), 		//Vega R3
		SGH_I337(				"SGH-I337",				Maker.samsung,	ChipsetType.APQ8064AB), 	//Galaxy S 4 LTE
		*/
		GT_N7100(				"GT-N7100", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SHV_E250S(				"SHV-E250S", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SHV_E250L(				"SHV-E250L",			Maker.samsung, 	ChipsetType.Exynos4412),	//Galaxy Note2
		GT_N7105(				"GT-N7105", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SGH_T889(				"SGH-T889", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2 
		SCH_I605(				"SCH-I605", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SAMSUNG_SGH_I317(		"SAMSUNG-SGH-I317", 	Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SPH_L900(				"SPH-L900", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SGH_I317M(				"SGH-I317M", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2 
		SC_02E(					"SC-02E", 				Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		GT_N7105T(				"GT-N7105T", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		GT_N7102(				"GT-N7102", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SGH_T889V(				"SGH-T889V", 			Maker.samsung, 	ChipsetType.Exynos4412), 	//Galaxy Note2
		SHV_E300L(				"SHV-E300L", 			Maker.samsung, 	ChipsetType.Exynos5410), 	//Galaxy S4 LTE
		SHV_E300K(				"SHV-E300K", 			Maker.samsung, 	ChipsetType.Exynos5410), 	//Galaxy S4 LTE
		SHV_E300S(				"SHV-E300S", 			Maker.samsung, 	ChipsetType.Exynos5410), 	//Galaxy S4 LTE
		GT_I9500(				"GT-I9500",				Maker.samsung, 	ChipsetType.Exynos5410), 	//Galaxy S4 LTE
		;
		
		public final String modelName;
		public final Maker maker;
		public final ChipsetType chipsetType;
		
		ModelInfo(String modelName, Maker maker, ChipsetType chipsetType) {
			this.modelName = modelName;
			this.maker = maker;
			this.chipsetType = chipsetType;
		}
		
	}
	
	public static ChipsetType getChipsetType() {
		
		for( ModelInfo info: ModelInfo.values()) {
			if( info.modelName.equals(Build.MODEL) ) {
				return info.chipsetType;
			}
		}
		
		return ChipsetType.UNKNOWN;
		
	}

}
