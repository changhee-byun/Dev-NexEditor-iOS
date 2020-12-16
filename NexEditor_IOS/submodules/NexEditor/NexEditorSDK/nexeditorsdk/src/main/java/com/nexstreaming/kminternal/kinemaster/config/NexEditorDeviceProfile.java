package com.nexstreaming.kminternal.kinemaster.config;

import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Pattern;

import android.content.Context;
import android.media.CamcorderProfile;
import android.os.Build;
import android.util.Log;

import com.nexstreaming.app.common.tracelog.DeviceSupportResponse;
import com.nexstreaming.app.common.tracelog.DeviceSupportResponse.DeviceInfo;
import com.nexstreaming.app.common.tracelog.DeviceSupportResponse.ExportResInfo;

import com.nexstreaming.app.common.util.CSVHeaderReader;
import com.nexstreaming.app.common.util.UserInfo;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;

public class NexEditorDeviceProfile implements Serializable {
    private static final long serialVersionUID = 1L;

    private static final String LOG_TAG = "NexEditorDeviceProfile";
    private static final boolean DB_LOGGING = true;

    private final int MEM_UHD_SIZE = (3840 * 2176);
    private final int MEM_1080P_SIZE = (1920 * 1088);
    private final int MEM_720P_SIZE = (1280 * 720);
    private final int MAX_SUPPORTED_FPS = 120;

    public static final int UNKNOWN = Integer.MIN_VALUE;
    private Size m_maxCaptureSize;

    public static enum MediaRecordingMode {
        Disable, UseNative, UseKineMaster;

        public static MediaRecordingMode fromValue(int value) {
            switch (value) {
                case 0:
                    return Disable;
                case 1:
                    return UseNative;
                case 2:
                    return UseKineMaster;
                default:
                    return Disable;
            }
        }
    }

    private final int m_maxImportSWSize;
    private final int m_maxImportHWSize;

    private final int m_NXSWAVCDecBaselineSize;
    private final int m_NXSWAVCDecMainSize;
    private final int m_NXSWAVCDecHighSize;

    private final int m_MCSWAVCDecBaselineSize;
    private final int m_MCSWAVCDecMainSize;
    private final int m_MCSWAVCDecHighSize;

    private final int m_MCHWAVCDecBaselineSize;
    private final int m_MCHWAVCDecMainSize;
    private final int m_MCHWAVCDecHighSize;

    private final String mBoardPlatform;
    private final NexExportProfile m_exportProfilesSW[];
    private final NexExportProfile m_exportProfilesHW[];
    private final NexExportProfile m_exportExtraProfilesSW[];
    private final NexExportProfile m_exportExtraProfilesHW[];
    private final MediaRecordingMode m_videoRecording;
    private final MediaRecordingMode m_imageRecording;
    private final boolean m_useNativeMediaDB;
    private final boolean m_useEditorView;
    //	private final NexExportProfile m_defaultExportProfile;
    private int m_fullHDMaxTransitionTime;
    private int m_maxSupportedFPS;
    private final boolean m_enableProjectProtection;
    private final boolean m_visualMediaFromKineMasterFolderOnly;
    private final boolean m_needSeekBeforeFastPreview;
    //	private final int m_videoRecordingQuality;
//	private final boolean m_includeVideoRecordingQualityInIntent;
    private final boolean m_limitTextInputHeight;
    private final int m_glDepthBufferBits;
    private final boolean m_glMultisample;
    //	private final Chipset m_chipset;
    private int m_extraDurationForSplit = 0;//Default : 1500
    private int m_maxCamcorderProfileSizeForUnknownDevice = 0;

    private final int m_hardwareCodecMemSize;
    private final int m_hardwareDecMaxCount;
    private final int m_hardwareEncMaxCount;
    private final boolean m_usedContext;
    private final boolean m_usedDSR;
    private final boolean m_support;
    private final int m_supportIfUpgradeVersion;
    private final boolean m_supportAVC;
    private final boolean m_supportMPEGV4;
    private final int m_audioCodecCount;
    private final ProfileSource m_profileSource;
    private final Map<String, String> m_properties;
    private final MatchInfo m_matchInfo;
    private final DeviceSupportResponse m_dsr;

    private int m_maxSupportedResolution;

    private boolean m_bSetUserConfig;

    private final int m_forceDirectExport;
    private final int m_nativeLogLevel;
    private final int m_supportedTimeCheker;
    private final int m_deviceMaxLightLevel;
    private final int m_deviceMaxGamma;
    private NexExportProfile[] m_actualSupportedExportProfilesSW = null;
    private NexExportProfile[] m_actualSupportedExportProfilesHW = null;

//	private final boolean m_supportVideoRecording;

    private static NexEditorDeviceProfile m_deviceProfile = null;

    public static void setAppContext(Context context, int[] prop) {
        if (m_deviceProfile != null && (m_deviceProfile.m_usedContext || m_deviceProfile.m_usedDSR))
            return;
        m_deviceProfile = new NexEditorDeviceProfile(context.getApplicationContext(), null, prop);
    }

    public static void setDeviceSupportResponse(DeviceSupportResponse dsr) {
        if (m_deviceProfile != null && m_deviceProfile.m_usedDSR)
            return;
        m_deviceProfile = new NexEditorDeviceProfile(null, dsr, null);
    }

    public static NexEditorDeviceProfile getDeviceProfile() {
        if (m_deviceProfile == null)
            m_deviceProfile = new NexEditorDeviceProfile(null, null, null);
        return m_deviceProfile;
    }

    // Case : for mantis 9091 problem.
//	public static enum Chipset {
//		MSM8064("gee", "GKKT", "GKU", "GKSK"), MSM8064_NEXUS7, MSM8960, exynos4, exynos5, MT6592, MT6589, MT6582, MSM8974, MSM8974_NOTE3, MSM8x26("MSM8226", "MSM8926"), UNKNOWN;
//		
//		private final String[] mMatches;
//		private Chipset(String...matches) {
//			mMatches = matches;
//		}
//		public boolean match(String s) {
//			if( s.equalsIgnoreCase(name()))
//				return true;
//			for( String m:mMatches ) {
//				if( s.equalsIgnoreCase(m) ) {
//					return true;
//				}
//			}
//			return false;
//		}
//	}

    public static enum ProfileSource {
        Local, Server, Unknown
    }

    private static void parseAPILevel(String apiLevel, int[] output) {
        int minAPILevel = 0;
        int maxAPILevel = 1000;
        if (apiLevel != null) {
            apiLevel = apiLevel.trim();
            int tildeIdx = apiLevel.indexOf('~');
            if (tildeIdx >= 0) {
                String fromLevel = apiLevel.substring(0, tildeIdx).trim();
                String toLevel = apiLevel.substring(tildeIdx + 1).trim();
                if (fromLevel.length() > 0) {
                    try {
                        minAPILevel = Integer.parseInt(fromLevel);
                    } catch (NumberFormatException e) {
                        minAPILevel = 0;
                    }
                }
                if (toLevel.length() > 0) {
                    try {
                        maxAPILevel = Integer.parseInt(toLevel);
                    } catch (NumberFormatException e) {
                        maxAPILevel = 1000;
                    }
                }
            } else if (apiLevel.length() > 0) {
                try {
                    int apiInt = Integer.parseInt(apiLevel);
                    minAPILevel = apiInt;
                    maxAPILevel = apiInt;
                } catch (NumberFormatException e) {
                    minAPILevel = 0;
                    maxAPILevel = 1000;
                }
            }
        }
        output[0] = minAPILevel;
        output[1] = maxAPILevel;
    }

    private static String trimAndNullEmpty(String s) {
        if (s == null)
            return "";
        s = s.trim();
        if (s.length() < 1)
            return "";
        return s;
    }

    private static boolean matchOne(String match, String list) {
        if (match == null || list == null) {
            return false;
        }
        String[] items = list.split("\n");
        for (String item : items) {
            String trimmedItem = item.trim();
            if (trimmedItem.length() > 0 && trimmedItem.equals(match))
                return true;
        }
        return false;
    }

    private static int parseMult(String s) {
        int value = 0;
        int n;
        boolean first = true;
        for (String part : s.split(Pattern.quote("*"))) {
            if (part.trim().length() < 1)
                continue;
            try {
                n = Integer.parseInt(part.trim());
            } catch (NumberFormatException e) {
                continue;
            }
            if (first) {
                value = n;
                first = false;
            } else {
                value *= n;
            }
        }
        return value;
    }

    private static NexExportProfile[] parseResList(List<ExportResInfo> resList) {
        NexExportProfile[] result = new NexExportProfile[resList.size()];
        for (int i = 0; i < result.length; i++) {
            ExportResInfo ri = resList.get(i);
            result[i] = new NexExportProfile(ri.width, ri.height, ri.display_height, ri.bitrate, NexExportProfile.getLabelResource(ri.width, ri.display_height));
        }
        return result;
    }

    private static NexExportProfile[] parseResList(String s) {

        if (s == null)
            return new NexExportProfile[0];

        List<NexExportProfile> result = new ArrayList<NexExportProfile>();
        for (String line : s.split("\\|")) {
            String fields[] = line.split(",");
            if (fields.length < 4)
                continue;
            try {
                int width = Integer.parseInt(fields[0].trim());
                int height = Integer.parseInt(fields[1].trim());
                int displayHeight = Integer.parseInt(fields[2].trim());
                int bitrate = parseMult(fields[3].trim());

                if (width < 1 || height < 1 || displayHeight < 1 || bitrate < 1)
                    continue;


                result.add(new NexExportProfile(width, height, displayHeight, bitrate, NexExportProfile.getLabelResource(width, displayHeight)));

            } catch (NumberFormatException e) {
                if (LL.W) Log.w(LOG_TAG, "parse error; skipping line", e);
            }
        }

        return result.toArray(new NexExportProfile[result.size()]);
    }

    private static Map<String, String> parseProperties(String s) {
        if (s == null)
            return Collections.<String, String>emptyMap();
        Map<String, String> result = new HashMap<String, String>();
        for (String prop : s.split("\\|")) {
            prop = prop.trim();
            int eq = prop.indexOf('=');
            if (eq < 1)
                continue;
            String key = prop.substring(0, eq);
            String value = prop.substring(eq + 1);
            result.put(key, value);
        }
        return result;
    }

    public static class MatchInfo {
        //		String alias_model;
        int record_number;
        String build_model;
        String build_device;
        String manufacturer;
        String board_platform;
        int os_api_level_min;
        int os_api_level_max;
        String predicate_name;

        public String getDebugString() {
            StringBuilder sb = new StringBuilder();
            sb.append("Match Basis:\n");
            sb.append("    predicate_name: ").append(String.valueOf(predicate_name)).append("\n");
            sb.append("    record_number: ").append(String.valueOf(record_number)).append("\n");
            sb.append("    board_platform: ").append(String.valueOf(board_platform)).append("\n");
            sb.append("    build_device: ").append(String.valueOf(build_device)).append("\n");
            sb.append("    build_model: ").append(String.valueOf(build_model)).append("\n");
            sb.append("    os_api_level_min: ").append(String.valueOf(os_api_level_min)).append("\n");
            sb.append("    os_api_level_max: ").append(String.valueOf(os_api_level_max)).append("\n");
            sb.append("    manufacturer: ").append(String.valueOf(manufacturer)).append("\n");
            return sb.toString();
        }
    }

    private abstract class Predicate {
        String name;

        Predicate(String name) {
            this.name = name;
        }

        @Override
        public String toString() {
            return "<Predicate " + name + ">";
        }

        abstract boolean matches(MatchInfo info, int checkSDKVersion);
    }

    private boolean isNullOrEmpty(String s) {
        return s == null || s.trim().length() < 1;
    }

    private CSVHeaderReader findLocalDBEntry(Context context, MatchInfo matchInfo, Predicate[] predicates, int sdkVersion) {
        CSVHeaderReader reader = null;
        PredicateSearch:
        for (Predicate p : predicates) {
            matchInfo.predicate_name = p.name;
            try {
                reader = new CSVHeaderReader(context.getAssets().open("km_device_support.csv"));
                try {
                    while (reader.next()) {
                        matchInfo.record_number = reader.getRecordNumber();
                        matchInfo.build_model = trimAndNullEmpty(reader.getString("build_model", null));
                        matchInfo.build_device = trimAndNullEmpty(reader.getString("build_device", null));
                        matchInfo.manufacturer = trimAndNullEmpty(reader.getString("manufacturer", null));
                        matchInfo.board_platform = trimAndNullEmpty(reader.getString("board_platform", null));
                        matchInfo.os_api_level_min = reader.getInt("os_api_level_min", Integer.MIN_VALUE);
                        matchInfo.os_api_level_max = reader.getInt("os_api_level_max", Integer.MAX_VALUE);
                        if (p.matches(matchInfo, Build.VERSION.SDK_INT)) {
                            if (LL.D) Log.d(LOG_TAG, "Match on " + p);
                            break PredicateSearch;
                        }
                    }
                } finally {
                    reader.close();
                }
            } catch (IOException e) {
                if (LL.E) Log.e(LOG_TAG, "Device Suport - Read Error", e);
            }
            reader = null;
        }
        return reader;
    }

    private NexEditorDeviceProfile(Context context, DeviceSupportResponse dsr, int[] prop) { //yoon delete

        m_usedContext = (context != null);
        /*yoon modfiy */
        m_usedDSR = (dsr != null);

        if (LL.D) Log.d(LOG_TAG, "Building device profile : hasContext=" + m_usedContext);
		
		/*
		String version = System.getProperty("os.version");
		if (version.contains("cyanogenmod")) {
		    isCyanogenMode = true;
		}
		*/

//		NexExportProfile defaultExportProfile = null;

//		String productModel = NexEditor.getSysProperty("ro.product.model");
//		String boardPlatform = NexEditor.getSysProperty("ro.board.platform");
//		if( boardPlatform==null || boardPlatform.trim().length()<1 ) {
//			boardPlatform = NexEditor.getSysProperty("ro.mediatek.platform");
//		}
//		if( boardPlatform==null || boardPlatform.trim().length()<1 ) {
//			boardPlatform = Build.HARDWARE;
//		}

        mBoardPlatform = UserInfo.getBoardPlatform();

//		Chipset chipset = Chipset.UNKNOWN;
//		
//		if( productModel.toUpperCase(Locale.US).contains("COOLPAD 9970L")  ) {
//			chipset = Chipset.MSM8974;
//		}		
//				
//		if( productModel.toUpperCase(Locale.US).contains("COOLPAD 9190L")  ) {
//			chipset = Chipset.MSM8x26;
//		}		
//
//		if( productModel.toUpperCase(Locale.US).contains("NEXUS 9") ) {
//			chipset = Chipset.MSM8064_NEXUS7;
//		}
//
//		if( productModel.toUpperCase(Locale.US).contains("NEXUS 7") ) {
//			chipset = Chipset.MSM8064_NEXUS7;
//		}
//		
//		if( productModel.toUpperCase(Locale.US).contains("NEXUS 4") ) {
//			chipset = Chipset.MSM8064_NEXUS7;
//		}	
//		
//		if( productModel.toUpperCase(Locale.US).contains("XT1060") ) {
//			chipset = Chipset.MSM8064_NEXUS7;
//		}		
//		
//		if( 	productModel.toUpperCase(Locale.US).contains("C6602") || // Xperia Z
//				productModel.toUpperCase(Locale.US).contains("C6603") || // Xperia Z
//				productModel.toUpperCase(Locale.US).contains("SO-04E") || // Xperia A
//				productModel.toUpperCase(Locale.US).contains("SOL22") || // Xperia UL
//				productModel.toUpperCase(Locale.US).contains("SO-02E") || // Xperia Z
//				productModel.toUpperCase(Locale.US).contains("C5502") || // Xperia ZR
//				productModel.toUpperCase(Locale.US).contains("C6502") || // Xperia ZL
//				productModel.toUpperCase(Locale.US).contains("C6606") || // Xperia Z
//				productModel.toUpperCase(Locale.US).contains("C6503") || // Xperia ZL
//				productModel.toUpperCase(Locale.US).contains("C6506") || // Xperia ZL
//				productModel.toUpperCase(Locale.US).contains("SGP321") || // Sony Xperia Tablet Z
//				productModel.toUpperCase(Locale.US).contains("C5503") || // Sony Xperia ZR
//				productModel.toUpperCase(Locale.US).contains("SGP312") // Sony Xperia Tablet Z WiFi
//				// productModel.toUpperCase(Locale.US).contains("D5503")  Sony Xperia Tablet Z WiFi 
//				) {
//			chipset = Chipset.MSM8064_NEXUS7;
//		}
//		
//		if( chipset==Chipset.UNKNOWN ) {
//			for( Chipset c: Chipset.values() ) {
//				if( c!=Chipset.UNKNOWN && (c.match(boardPlatform) || c.match(Build.BOARD.trim()) || c.match(Build.HARDWARE.trim())) ) {
//					chipset = c;
//					break;
//				}
//			}
//		}


//		if(LL.D) Log.d(LOG_TAG,"chipset=" + chipset + " board=" + Build.BOARD + " hardware=" + Build.HARDWARE + " model=" + Build.MODEL + " boardPlatform=" + boardPlatform + " api_level=" + Build.VERSION.SDK_INT);

        if (dsr != null && EditorGlobal.ENABLE_EXTENDED_DEVICE_SUPPORT && dsr.result == 0 && dsr.match_info != null && dsr.device_info != null) {

            MatchInfo mi = new MatchInfo();
            mi.board_platform = dsr.match_info.board_platform;
            mi.build_device = dsr.match_info.build_device;
            mi.build_model = dsr.match_info.build_model;
            mi.manufacturer = dsr.match_info.manufacturer;
            mi.os_api_level_min = dsr.match_info.os_api_level_min;
            mi.os_api_level_max = dsr.match_info.os_api_level_max;
            mi.predicate_name = "server";
            mi.record_number = dsr.match_info.record_idx;

            DeviceInfo di = dsr.device_info;
            m_support = (di.supported == 1);
            m_supportAVC = (di.support_avc == 3);
            m_supportMPEGV4 = (di.support_mpeg4v == 3);
            m_maxSupportedFPS = di.max_fps;
            m_hardwareCodecMemSize = di.max_codec_mem_size;
            m_hardwareDecMaxCount = di.max_dec_count;
            m_hardwareEncMaxCount = di.max_enc_count;
            int fullHDMaxTransitionTime = di.max_fhd_trans_time;
            m_imageRecording = MediaRecordingMode.fromValue(di.rec_image_mode);
            m_videoRecording = MediaRecordingMode.fromValue(di.rec_video_mode);
            m_audioCodecCount = di.audio_codec_count;
            m_maxImportSWSize = di.max_sw_import_res;
            m_maxImportHWSize = di.max_hw_import_res;
            m_NXSWAVCDecBaselineSize = di.max_dec_res_nexsw_b;
            m_NXSWAVCDecMainSize = di.max_dec_res_nexsw_m;
            m_NXSWAVCDecHighSize = di.max_dec_res_nexsw_h;
            m_MCSWAVCDecBaselineSize = di.max_dec_res_sw_b;
            m_MCSWAVCDecMainSize = di.max_dec_res_sw_m;
            m_MCSWAVCDecHighSize = di.max_dec_res_sw_h;
            m_MCHWAVCDecBaselineSize = di.max_dec_res_hw_b;
            m_MCHWAVCDecMainSize = di.max_dec_res_hw_m;
            m_MCHWAVCDecHighSize = di.max_dec_res_hw_h;
            m_exportProfilesSW = parseResList(di.export_res_sw);
            m_exportProfilesHW = parseResList(di.export_res_hw);
            m_exportExtraProfilesHW = parseResList(di.export_res_extra);
            m_exportExtraProfilesSW = parseResList(di.export_res_extra);
            m_properties = di.properties;

            m_fullHDMaxTransitionTime = fullHDMaxTransitionTime < 0 ? Integer.MAX_VALUE : fullHDMaxTransitionTime;

            m_visualMediaFromKineMasterFolderOnly = false;
            m_useNativeMediaDB = true;
            m_useEditorView = false;
            m_needSeekBeforeFastPreview = true;
            m_limitTextInputHeight = false;
            m_glMultisample = true;
            m_glDepthBufferBits = 16;
            m_enableProjectProtection = false;
            m_matchInfo = mi;
            m_supportIfUpgradeVersion = 0;

            m_profileSource = ProfileSource.Server;
            m_dsr = dsr;
            m_forceDirectExport = 0; //yoon
            m_nativeLogLevel = 2;
            return;
        }

        m_dsr = null;
        boolean bUseCVS = false;

        if (bUseCVS && context != null && EditorGlobal.ENABLE_EXTENDED_DEVICE_SUPPORT) {
            final String ro_board_platform = mBoardPlatform;

            MatchInfo matchInfo = new MatchInfo();
            Predicate predicates[] = {
                    new Predicate("Device") {
                        @Override
                        public boolean matches(MatchInfo info, int checkSDKVersion) {
                            return checkSDKVersion >= info.os_api_level_min
                                    && checkSDKVersion <= info.os_api_level_max
                                    && info.build_device.equalsIgnoreCase(Build.DEVICE);
                        }
                    },
                    new Predicate("Model") {
                        @Override
                        public boolean matches(MatchInfo info, int checkSDKVersion) {
                            return checkSDKVersion >= info.os_api_level_min
                                    && checkSDKVersion <= info.os_api_level_max
                                    && info.build_model.equalsIgnoreCase(Build.MODEL);
                        }
                    },
                    new Predicate("Mfg+Chip") {
                        @Override
                        public boolean matches(MatchInfo info, int checkSDKVersion) {
                            return checkSDKVersion >= info.os_api_level_min
                                    && checkSDKVersion <= info.os_api_level_max
                                    && isNullOrEmpty(info.build_device)
                                    && isNullOrEmpty(info.build_model)
                                    && info.manufacturer.equalsIgnoreCase(Build.MANUFACTURER)
                                    && info.board_platform.equalsIgnoreCase(ro_board_platform);
                        }
                    },
                    new Predicate("Mfg") {
                        @Override
                        public boolean matches(MatchInfo info, int checkSDKVersion) {
                            return checkSDKVersion >= info.os_api_level_min
                                    && checkSDKVersion <= info.os_api_level_max
                                    && isNullOrEmpty(info.build_device)
                                    && isNullOrEmpty(info.build_model)
                                    && isNullOrEmpty(info.board_platform)
                                    && info.manufacturer.equalsIgnoreCase(Build.MANUFACTURER);
                        }
                    },
                    new Predicate("Chip") {
                        @Override
                        public boolean matches(MatchInfo info, int checkSDKVersion) {
                            return checkSDKVersion >= info.os_api_level_min
                                    && checkSDKVersion <= info.os_api_level_max
                                    && isNullOrEmpty(info.build_device)
                                    && isNullOrEmpty(info.build_model)
                                    && isNullOrEmpty(info.manufacturer)
                                    && info.board_platform.equalsIgnoreCase(ro_board_platform);
                        }
                    },
            };

            CSVHeaderReader reader = findLocalDBEntry(context, matchInfo, predicates, Build.VERSION.SDK_INT);
//			PredicateSearch: for( Predicate p: predicates ) {
//				matchInfo.predicate_name = p.name;
//				try {
//					reader = new CSVHeaderReader(context.getAssets().open("km_device_support.csv"));
//					try {
//						while( reader.next() ) {
//							matchInfo.record_number		= reader.getRecordNumber();
//							matchInfo.build_model		= trimAndNullEmpty(reader.getString("build_model", 		null));
//							matchInfo.build_device		= trimAndNullEmpty(reader.getString("build_device", 	null));
//							matchInfo.manufacturer		= trimAndNullEmpty(reader.getString("manufacturer",		null));
//							matchInfo.board_platform	= trimAndNullEmpty(reader.getString("board_platform",	null));
//							matchInfo.os_api_level_min 	= reader.getInt("os_api_level_min", Integer.MIN_VALUE);
//							matchInfo.os_api_level_max 	= reader.getInt("os_api_level_max", Integer.MAX_VALUE);
//							if( p.matches(matchInfo,Build.VERSION.SDK_INT) ) {
//								if( LL.D ) Log.d(LOG_TAG,"Match on " + p);
//								break PredicateSearch;
//							}
//						}
//					} finally {
//						reader.close();
//					}
//				} catch (IOException e) {
//					if( LL.E ) Log.e(LOG_TAG,"Device Suport - Read Error",e);
//				}
//				reader = null;
//			}

            if (reader != null) {
//				reader.setCurrentRecord(useMatch);

                if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "Found match, parsing...");

                m_support = (reader.getInt("supported", 0) == 1);
                m_supportAVC = (reader.getInt("support_avc", 0) == 3);
                m_supportMPEGV4 = (reader.getInt("support_mpeg4v", 0) == 3);
                m_maxSupportedFPS = reader.getInt("max_fps", 0);
                m_hardwareCodecMemSize = reader.getInt("max_codec_mem_size", 0);
                m_hardwareDecMaxCount = reader.getInt("max_dec_count", 0);
                m_hardwareEncMaxCount = reader.getInt("max_enc_count", 0);
                int fullHDMaxTransitionTime = reader.getInt("max_fhd_trans_time", Integer.MAX_VALUE);
                m_imageRecording = MediaRecordingMode.fromValue(reader.getInt("rec_image_mode", 0));
                m_videoRecording = MediaRecordingMode.fromValue(reader.getInt("rec_video_mode", 0));
                m_audioCodecCount = reader.getInt("audio_codec_count", 0);
                m_maxImportSWSize = reader.getInt("max_sw_import_res", 0);
                m_maxImportHWSize = reader.getInt("max_hw_import_res", 0);
                m_NXSWAVCDecBaselineSize = reader.getInt("max_dec_res_nexsw_b", 0);
                m_NXSWAVCDecMainSize = reader.getInt("max_dec_res_nexsw_m", 0);
                m_NXSWAVCDecHighSize = reader.getInt("max_dec_res_nexsw_h", 0);
                m_MCSWAVCDecBaselineSize = reader.getInt("max_dec_res_sw_b", 0);
                m_MCSWAVCDecMainSize = reader.getInt("max_dec_res_sw_m", 0);
                m_MCSWAVCDecHighSize = reader.getInt("max_dec_res_sw_h", 0);
                m_MCHWAVCDecBaselineSize = reader.getInt("max_dec_res_hw_b", 0);
                m_MCHWAVCDecMainSize = reader.getInt("max_dec_res_hw_m", 0);
                m_MCHWAVCDecHighSize = reader.getInt("max_dec_res_hw_h", 0);
                m_exportProfilesSW = parseResList(reader.getString("export_res_sw", null));
                m_exportProfilesHW = parseResList(reader.getString("export_res_hw", null));
                m_exportExtraProfilesHW = parseResList(reader.getString("export_res_extra", null));
                m_exportExtraProfilesSW = parseResList(reader.getString("export_res_extra", null));
                m_properties = parseProperties(reader.getString("properties", null));

                m_fullHDMaxTransitionTime = fullHDMaxTransitionTime < 0 ? Integer.MAX_VALUE : fullHDMaxTransitionTime;

                m_visualMediaFromKineMasterFolderOnly = false;
                m_useNativeMediaDB = true;
                m_useEditorView = false;
                m_needSeekBeforeFastPreview = true;
                m_limitTextInputHeight = false;
                m_glMultisample = true;
                m_glDepthBufferBits = 16;
                m_enableProjectProtection = false;
                m_matchInfo = matchInfo;
                m_forceDirectExport = 0; //yoon
                m_nativeLogLevel = 2;
                if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "  m_support = " + m_support);
                if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "  m_supportAVC = " + m_supportAVC);
                if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "  m_supportMPEGV4 = " + m_supportMPEGV4);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_maxSupportedFPS = " + m_maxSupportedFPS);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_hardwareCodecMemSize = " + m_hardwareCodecMemSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_hardwareDecMaxCount = " + m_hardwareDecMaxCount);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_hardwareEncMaxCount = " + m_hardwareEncMaxCount);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_fullHDMaxTransitionTime = " + m_fullHDMaxTransitionTime);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_imageRecording = " + m_imageRecording.name());
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_videoRecording = " + m_videoRecording.name());
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_maxImportSWSize = " + m_maxImportSWSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_maxImportHWSize = " + m_maxImportHWSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_NXSWAVCDecBaselineSize = " + m_NXSWAVCDecBaselineSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_NXSWAVCDecMainSize = " + m_NXSWAVCDecMainSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_NXSWAVCDecHighSize = " + m_NXSWAVCDecHighSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_MCSWAVCDecBaselineSize = " + m_MCSWAVCDecBaselineSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_MCSWAVCDecMainSize = " + m_MCSWAVCDecMainSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_MCSWAVCDecHighSize = " + m_MCSWAVCDecHighSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_MCHWAVCDecBaselineSize = " + m_MCHWAVCDecBaselineSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_MCHWAVCDecMainSize = " + m_MCHWAVCDecMainSize);
                if (LL.D && DB_LOGGING)
                    Log.d(LOG_TAG, "  m_MCHWAVCDecHighSize = " + m_MCHWAVCDecHighSize);
                if (LL.D && DB_LOGGING) {
                    if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "  m_exportProfilesSW:");
                    for (NexExportProfile prof : m_exportProfilesSW) {
                        if (LL.D && DB_LOGGING)
                            Log.d(LOG_TAG, "      " + prof.width() + "x" + prof.height() + "  " + prof.bitrate() + "   " + prof.label(context.getResources()));
                    }
                    if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "  m_exportProfilesHW:");
                    for (NexExportProfile prof : m_exportProfilesHW) {
                        if (LL.D && DB_LOGGING)
                            Log.d(LOG_TAG, "      " + prof.width() + "x" + prof.height() + "  " + prof.bitrate() + "   " + prof.label(context.getResources()));
                    }
                    if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "  m_exportExtraProfilesSW:");
                    for (NexExportProfile prof : m_exportExtraProfilesSW) {
                        if (LL.D && DB_LOGGING)
                            Log.d(LOG_TAG, "      " + prof.width() + "x" + prof.height() + "  " + prof.bitrate() + "   " + prof.label(context.getResources()));
                    }
                    if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "  m_exportExtraProfilesHW:");
                    for (NexExportProfile prof : m_exportExtraProfilesHW) {
                        if (LL.D && DB_LOGGING)
                            Log.d(LOG_TAG, "      " + prof.width() + "x" + prof.height() + "  " + prof.bitrate() + "   " + prof.label(context.getResources()));
                    }
                }

                m_profileSource = ProfileSource.Local;

                int supportIfUpgradeVersion = 0;

                if (!getIsDeviceSupported()) {
                    CSVHeaderReader upgradeReader = findLocalDBEntry(context, matchInfo, predicates, 10000);
                    if (upgradeReader != null) {
                        boolean support = (upgradeReader.getInt("supported", 0) == 1);
                        boolean supportAVC = (upgradeReader.getInt("support_avc", 0) == 3);
                        boolean supportMPEGV4 = (upgradeReader.getInt("support_mpeg4v", 0) == 3);
                        int acc = reader.getInt("audio_codec_count", 0);

                        if (support && (acc == -1 || acc >= 6) && (supportAVC || supportMPEGV4)) {
                            supportIfUpgradeVersion = upgradeReader.getInt("os_api_level_min", 0);
                        }
                    }
                }

                m_supportIfUpgradeVersion = supportIfUpgradeVersion;
//				m_chipset = null;
                return;
            }

        }
//		m_chipset = chipset;


//		if( productModel.toUpperCase(Locale.US).contains("C6903") /* Xperia Z1 */) {
//			m_videoRecording = MediaRecordingMode.Disable;
//			m_imageRecording = MediaRecordingMode.UseNative;
//		} else {
        m_videoRecording = MediaRecordingMode.UseKineMaster;
        m_imageRecording = MediaRecordingMode.UseNative;
//		}

        m_supportIfUpgradeVersion = 0;
        m_useNativeMediaDB = true;
        m_needSeekBeforeFastPreview = true;
        m_limitTextInputHeight = false;
        m_glDepthBufferBits = 16;
        m_glMultisample = true;
        m_useEditorView = false;
        m_supportAVC = true;
        m_support = true;

        m_audioCodecCount = -1;
        m_matchInfo = null;

        if (prop == null)
            m_bSetUserConfig = false;
        else
            m_bSetUserConfig = (prop[0] == 1) ? true : false;

        if (prop == null)
            m_hardwareCodecMemSize = MEM_UHD_SIZE;
        else
            m_hardwareCodecMemSize = prop[1];

        if (prop == null)
            m_hardwareDecMaxCount = 2;
        else
            m_hardwareDecMaxCount = prop[2];

        if (prop == null)
            m_maxSupportedFPS = MAX_SUPPORTED_FPS;
        else
            m_maxSupportedFPS = prop[3];

        if (prop == null)
            m_supportMPEGV4 = false;
        else
            m_supportMPEGV4 = prop[4] == 0 ? false : true;

        if (prop == null)
            m_maxSupportedResolution = MEM_UHD_SIZE;
        else
            m_maxSupportedResolution = prop[5];

        if (prop == null)
            m_forceDirectExport = 0;
        else
            m_forceDirectExport = prop[6];

        if (prop == null)
            m_nativeLogLevel = 2;
        else
            m_nativeLogLevel = prop[7];

        if (prop == null)
            m_supportedTimeCheker = 1;
        else
            m_supportedTimeCheker = prop[8];

        if (prop == null)
            m_deviceMaxLightLevel = 550;
        else
            m_deviceMaxLightLevel = prop[9];

        if (prop == null)
            m_deviceMaxGamma = 550;
        else
            m_deviceMaxGamma = prop[10];

        if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "User set supportMPEGV4 = " + m_supportMPEGV4);
        if (LL.D && DB_LOGGING)
            Log.d(LOG_TAG, "User set hardwareDecMaxCount = " + m_hardwareDecMaxCount);
        if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "User set maxSupportedFPS = " + m_maxSupportedFPS);
        if (LL.D && DB_LOGGING)
            Log.d(LOG_TAG, "User set hardwareCodecMemSize = " + m_hardwareCodecMemSize);
        if (LL.D && DB_LOGGING)
            Log.d(LOG_TAG, "User set maxSupportedResolution = " + m_maxSupportedResolution);
        if (LL.D && DB_LOGGING)
            Log.d(LOG_TAG, "User set forceDirectExport = " + m_forceDirectExport);
        if (LL.D && DB_LOGGING) Log.d(LOG_TAG, "User set m_nativeLogLevel = " + m_nativeLogLevel);
        if (LL.D && DB_LOGGING)
            Log.d(LOG_TAG, "User set m_supportedTimeCheker = " + m_supportedTimeCheker);
        if (LL.D && DB_LOGGING)
            Log.d(LOG_TAG, "User set device LightLevel = " + m_deviceMaxLightLevel + ", Gamma=" + m_deviceMaxGamma);
        m_hardwareEncMaxCount = 1;

        m_maxImportSWSize = UNKNOWN;
        m_maxImportHWSize = UNKNOWN;

        m_NXSWAVCDecBaselineSize = UNKNOWN;
        m_NXSWAVCDecMainSize = UNKNOWN;
        m_NXSWAVCDecHighSize = UNKNOWN;

        m_MCSWAVCDecBaselineSize = UNKNOWN;
        m_MCSWAVCDecMainSize = UNKNOWN;
        m_MCSWAVCDecHighSize = UNKNOWN;

        m_MCHWAVCDecBaselineSize = UNKNOWN;
        m_MCHWAVCDecMainSize = UNKNOWN;
        m_MCHWAVCDecHighSize = UNKNOWN;

        m_exportProfilesSW = null; // UNKNOWN
        m_exportProfilesHW = null; // UNKNOWN

        m_fullHDMaxTransitionTime = Integer.MAX_VALUE;

        m_profileSource = ProfileSource.Unknown;

        m_properties = new HashMap<String, String>();

        int quality[] = {CamcorderProfile.QUALITY_HIGH, CamcorderProfile.QUALITY_1080P, CamcorderProfile.QUALITY_720P, CamcorderProfile.QUALITY_480P, CamcorderProfile.QUALITY_CIF, CamcorderProfile.QUALITY_QVGA};
        int bestCamcorderProfileSize = 0;
        for (int i = 0; i < quality.length; i++) {
            if (CamcorderProfile.hasProfile(quality[i])) {
                CamcorderProfile profile = CamcorderProfile.get(quality[i]);
                if (profile == null) continue;

                int adjHeight = ((profile.videoFrameHeight + 31) & ~0xF);
                if (profile.videoFrameWidth * adjHeight > bestCamcorderProfileSize) {
                    bestCamcorderProfileSize = profile.videoFrameWidth * adjHeight;
                }
            }
        }
        m_maxCamcorderProfileSizeForUnknownDevice = bestCamcorderProfileSize;
        m_exportExtraProfilesSW = new NexExportProfile[]{NexExportProfile.EXPORT_320_180};
        m_exportExtraProfilesHW = new NexExportProfile[]{NexExportProfile.EXPORT_320_180};
        m_enableProjectProtection = false;
        m_visualMediaFromKineMasterFolderOnly = false;
    }

    public ProfileSource getProfileSource() {
        return m_profileSource;
    }

    public boolean isUnknownDevice() {
        return m_profileSource == ProfileSource.Unknown;
    }

    public int getMaxResolution() {
        return m_maxSupportedResolution;
    }

    public int getMaxImportSize(boolean usingSoftwareCodec) {
        if (EditorGlobal.ENABLE_EXTENDED_DEVICE_SUPPORT) {
            if (isUnknownDevice()) {
                NexEditor editor = EditorGlobal.getEditor();
                if (editor == null) {
                    return m_maxCamcorderProfileSizeForUnknownDevice;
                } else if (m_maxCamcorderProfileSizeForUnknownDevice <= 0) {
                    return editor.getVisualClipChecker().getMaxImportSizeWithHWDecoder();
                } else {
                    return Math.min(m_maxCamcorderProfileSizeForUnknownDevice, editor.getVisualClipChecker().getMaxImportSizeWithHWDecoder());
                }
            } else {
                return usingSoftwareCodec ? m_maxImportSWSize : m_maxImportHWSize;
            }
        } else {
            return usingSoftwareCodec ? m_maxImportSWSize : m_maxImportHWSize;
        }
    }

    private int getUnknownPropFromEngine(int value, String dependent_prop, String prop) {
        if (value == UNKNOWN) {
            NexEditor editor = EditorGlobal.getEditor();
            if (editor == null)
                return m_maxCamcorderProfileSizeForUnknownDevice;
            if (dependent_prop != null) {
                if (!editor.getPropertyBool(dependent_prop, false)) {
                    return 0;
                }
            }
            if (prop != null) {
                if (m_maxCamcorderProfileSizeForUnknownDevice <= 0) {
                    return editor.getPropertyInt(prop, 0);
                } else {
                    return Math.min(m_maxCamcorderProfileSizeForUnknownDevice, editor.getPropertyInt(prop, 0));
                }
            }
            return 0;
        } else {
            return value;
        }
    }

    public int getMaxCamcorderProfileSizeForUnknownDevice() {
        return m_maxCamcorderProfileSizeForUnknownDevice;
    }

    public int getNXSWAVCDecBaselineSize() {
        return getUnknownPropFromEngine(m_NXSWAVCDecBaselineSize, "canUseSoftwareCodec", "NXSWAVCDecBaselineLevelSize");
    }

    public int getNXSWAVCDecMainSize() {
        return getUnknownPropFromEngine(m_NXSWAVCDecMainSize, "canUseSoftwareCodec", "NXSWAVCDecMainLevelSize");
    }

    public int getNXSWAVCDecHighSize() {
        return getUnknownPropFromEngine(m_NXSWAVCDecHighSize, "canUseSoftwareCodec", "NXSWAVCDecHighLevelSize");
    }


    public int getMCSWAVCDecBaselineSize() {
        return getUnknownPropFromEngine(m_MCSWAVCDecBaselineSize, "canUseMCSoftwareCodec", "MCSWAVCDecBaselineLevelSize");
    }

    public int getMCSWAVCDecMainSize() {
        return getUnknownPropFromEngine(m_MCSWAVCDecMainSize, "canUseMCSoftwareCodec", "MCSWAVCDecMainLevelSize");
    }

    public int getMCSWAVCDecHighSize() {
        return getUnknownPropFromEngine(m_MCSWAVCDecHighSize, "canUseMCSoftwareCodec", "MCSWAVCDecMainLevelSize");
    }


    public int getMCHWAVCDecBaselineSize() {
        return getUnknownPropFromEngine(m_MCHWAVCDecBaselineSize, null, "MCHWAVCDecBaselineLevelSize");
    }

    public int getMCHWAVCDecMainSize() {
        return getUnknownPropFromEngine(m_MCHWAVCDecMainSize, null, "MCHWAVCDecMainLevelSize");
    }

    public int getMCHWAVCDecHighSize() {
        return getUnknownPropFromEngine(m_MCHWAVCDecHighSize, null, "MCHWAVCDecHighLevelSize");
    }


	/*
	public Size getMaxImportResolution(boolean usingSoftwareCode) {
		return new Size(getMaxImportWidth(usingSoftwareCode),getMaxImportHeight(usingSoftwareCode));
	}
	*/

    public int getMaxSupportedFPS() {
        return m_maxSupportedFPS;
    }

    public int getMaxSupportedFPS(int width, int height) {
        return m_maxSupportedFPS;
    }

    public int getFullHDMaxTransitionTime() {
        return m_fullHDMaxTransitionTime;
    }

    public MediaRecordingMode getImageRecordingMode() {
        return m_imageRecording;
    }

    public MediaRecordingMode getVideoRecordingMode() {
        return m_videoRecording;
    }

    public boolean getUseNativeMediaDB() {
        return m_useNativeMediaDB;
    }

    public boolean getUseEditorView() {
        return m_useEditorView;
    }

    public NexExportProfile[] getSupportedExportProfiles(boolean usingSoftwareCodec) {
        NexExportProfile[] result = null;


        if (usingSoftwareCodec) {
            result = m_actualSupportedExportProfilesSW;
        } else {
            result = m_actualSupportedExportProfilesHW;
        }

        if (result != null)
            return filterByMaxImportSize(result, usingSoftwareCodec);

        if (usingSoftwareCodec) {
            result = m_exportProfilesSW;
        } else {
            result = m_exportProfilesHW;
        }

        if (result == null) {
            NexEditor editor = EditorGlobal.getEditor();
            if (editor == null) {
                result = new NexExportProfile[0];
            } else {
                List<NexExportProfile> listProfile = NexExportProfile.getExportProfiles(editor.getVisualClipChecker().getMaxHardwareAVCEncBaselineSize() /*TODO: CHECK??*/);
                result = listProfile.toArray(new NexExportProfile[listProfile.size()]);
            }

        }

        if (usingSoftwareCodec) {
            m_actualSupportedExportProfilesSW = result;
        } else {
            m_actualSupportedExportProfilesHW = result;
        }

        return filterByMaxImportSize(result, usingSoftwareCodec);
    }

    private NexExportProfile[] filterByMaxImportSize(NexExportProfile[] exportProfiles, boolean usingSoftwareCodec) {

        List<NexExportProfile> filteredProfiles = new ArrayList<NexExportProfile>(exportProfiles.length);
        int maxImportSize = getMaxImportSize(usingSoftwareCodec);

        NexExportProfile smallest = null;

        for (NexExportProfile profile : exportProfiles) {
            if (profile.width() * (profile.height() - 31) <= maxImportSize) {
                filteredProfiles.add(profile);
            } else if (smallest == null || smallest.width() * smallest.height() > profile.width() * profile.height()) {
                smallest = profile;
            }
        }

        if (filteredProfiles.size() < 1 && smallest != null) {
            filteredProfiles.add(smallest);
        }

        return filteredProfiles.toArray(new NexExportProfile[filteredProfiles.size()]);
    }

    public NexExportProfile[] getSupportedExtraProfiles(boolean usingSoftwareCodec) {
        if (usingSoftwareCodec) {
            return m_exportExtraProfilesSW;
        } else {
            return m_exportExtraProfilesHW;
        }
    }

    public NexExportProfile getDefaultExportProfile(boolean usingSoftwareCodec) {

        NexExportProfile defaultExportProfile = null;

        NexExportProfile[] supported_profiles = getSupportedExportProfiles(usingSoftwareCodec);

        // Default to 720p if available and no explicit default for this device
        if (defaultExportProfile == null) {
            for (NexExportProfile p : supported_profiles) {
                if (p.height() == 720 || p.height() == 736) {
                    defaultExportProfile = p;
                    break;
                }
            }
        }

        // Default to 1080p if 720p not available and no explicit default for this device
        if (defaultExportProfile == null) {
            for (NexExportProfile p : supported_profiles) {
                if (p.height() == 1080 && p.height() == 1088) {
                    defaultExportProfile = p;
                    break;
                }
            }
        }

        // If neither 1080p nor 720p are available, default to first listed profile
        if (defaultExportProfile == null) {
            defaultExportProfile = supported_profiles[0];
        }

        return defaultExportProfile;
    }

    public boolean getEnableProjectProtection() {
        return m_enableProjectProtection;
    }

    public boolean getVisualMediaFromKineMasterFolderOnly() {
        return m_visualMediaFromKineMasterFolderOnly;
    }

    public boolean getNeedSeekBeforeFastPreview() {
        return m_needSeekBeforeFastPreview;
    }

//	public boolean getIncludeVideoRecordingQualityInIntent() {
//		return m_includeVideoRecordingQualityInIntent;
//	}
//	
//	public int getVideoRecordingQuality() {
//		return m_videoRecordingQuality;
//	}

//	public boolean getSupportVideoRecording() {
//		return m_supportVideoRecording;
//	}

    public boolean getLimitTextInputHeight() {
        return m_limitTextInputHeight;
    }

    public int getGLDepthBufferBits() {
        return m_glDepthBufferBits;
    }

    public boolean getGLMultisample() {
        return m_glMultisample;
    }

//	public Chipset getChipset() {
//		return m_chipset;			// XXX ???
//	}

    public boolean getUserConfigSettings() {
        return m_bSetUserConfig;
    }

    public int getHardwareCodecMemSize() {
        return m_hardwareCodecMemSize;
    }

    public int getHardwareDecMaxCount() {
        return m_hardwareDecMaxCount;
    }

    public boolean allowOverlappingVideo() {
        return (getHardwareDecMaxCount() >= 2);
    }

    public int getHardwareEncMaxCount() {
        return m_hardwareEncMaxCount;
    }

    public int getForceDirectExport() {
        return m_forceDirectExport;
    } //yoon

    public int getNativeLogLevel() {
        return m_nativeLogLevel;
    } //yoon

    public int getSupportedTimeCheker() {
        return m_supportedTimeCheker;
    } //yoon

    public int getDeviceMaxLightLevel() {
        return m_deviceMaxLightLevel;
    } //yoon

    public int getDeviceMaxGamma() {
        return m_deviceMaxGamma;
    } //yoon

    private static final Size s_transcodeMaxInputResolution = new Size(1920, 1088);

    public Size getTranscodeMaxInputResolution(boolean usingSoftwareCodec) {
        return s_transcodeMaxInputResolution;
    }

    private NexExportProfile m_transcodeProfile = null;

    public NexExportProfile getTranscodeProfile(boolean usingSoftwareCodec) {
        final int maxWidth = 1920;
        final int maxHeight = 1088;
        int maxImportSize = getMaxImportSize(usingSoftwareCodec);
        if (m_transcodeProfile == null) {
            for (NexExportProfile profile : getSupportedExportProfiles(usingSoftwareCodec)) {
                if (profile.width() * (profile.height() - 31) > maxImportSize)
                    continue;
                if (profile.width() > maxWidth || profile.height() > maxHeight) {
                    continue;
                } else if (m_transcodeProfile == null || m_transcodeProfile.height() < profile.height()) {
                    m_transcodeProfile = profile;
                }
            }
        }
        if (m_transcodeProfile == null) {
            throw new InternalError();
        }
        return m_transcodeProfile;
    }

    public NexExportProfile getTranscodeProfile(boolean usingSoftwareCodec, int iWidth, int iHeight) {
        int iResSize = iWidth * iHeight;
        int iImportSize = usingSoftwareCodec ? m_maxImportSWSize : m_maxImportHWSize;
        NexExportProfile transcodeProfile = null;
        int maxImportSize = getMaxImportSize(usingSoftwareCodec);

        for (NexExportProfile profile : getSupportedExportProfiles(usingSoftwareCodec)) {
            if (profile.width() * (profile.height() - 31) > maxImportSize)
                continue;
            int iProfileSize = profile.width() * profile.height();
            if (iProfileSize <= (iResSize * 105 / 100) && iProfileSize <= iImportSize) {
                transcodeProfile = profile;
                break;
            }
        }
        if (transcodeProfile == null) {
            transcodeProfile = NexExportProfile.getExportProfiles(iWidth, iHeight);
        }

        if (transcodeProfile == null) {
            throw new InternalError();
        }
        return transcodeProfile;
    }

    public static class Size {
        public final int width;
        public final int height;

        public Size(int width, int height) {
            this.width = width;
            this.height = height;
        }

        @Override
        public int hashCode() {
            return width * 31 + height;
        }

        @Override
        public boolean equals(Object o) {
            if (o instanceof Size) {
                return ((Size) o).width == width && ((Size) o).height == height;
            } else {
                return false;
            }
        }
    }

    public int getMaxSpeedCtrlValue(int iWidth, int iHeight) {
        if (iWidth * iHeight <= m_maxImportHWSize) {
            return 150;
        }

        if (iWidth * iHeight > 1280 * 736) {
            return 100;
        }
        return 150;
    }

    public int getMinSpeedCtrlValue() {
        return 25;
    }

    public int getExtraDurationForSplit() {//for 1080p contents
        return m_extraDurationForSplit;
    }

    public boolean getAVCSupported() {
        return m_supportAVC;
    }

    public boolean getMPEGV4Supported() {
        return m_supportMPEGV4;
    }

    public boolean getNeedsColorFormatCheck() {
        if (isUnknownDevice()) {
            if (Build.VERSION.SDK_INT <= 17 && mBoardPlatform.trim().toLowerCase(Locale.US).contains("exynos")) {
                return true;
            } else {
                return false;
            }
        }
        return (getIntProperty("chk_color_fmt", 0) == 1);
    }

    public String getStringProperty(String name, String defaultValue) {
        if (m_properties.containsKey(name)) {
            return m_properties.get(name);
        } else {
            return defaultValue;
        }
    }

    public int getIntProperty(String name, int defaultValue) {
        if (m_properties.containsKey(name)) {
            try {
                return Integer.parseInt(m_properties.get(name));
            } catch (NumberFormatException e) {
                return defaultValue;
            }
        } else {
            return defaultValue;
        }
    }

    public boolean getBooleanProperty(String name, boolean defaultValue) {
        if (m_properties.containsKey(name)) {
            String prop = m_properties.get(name);
            if (prop.trim().equalsIgnoreCase("true"))
                return true;
            else if (prop.trim().equalsIgnoreCase("false"))
                return false;
            else
                return defaultValue;
        } else {
            return defaultValue;
        }
    }

    public long getLongProperty(String name, long defaultValue) {
        if (m_properties.containsKey(name)) {
            try {
                return Long.parseLong(m_properties.get(name));
            } catch (NumberFormatException e) {
                return defaultValue;
            }
        } else {
            return defaultValue;
        }
    }

    public MatchInfo getMatchInfo() {
        return m_matchInfo;
    }

    public int getAudioCodecMaxCount() {
        if (m_audioCodecCount == -1)
            return Integer.MAX_VALUE;
        else if (m_audioCodecCount < 0)
            return 0;
        else
            return m_audioCodecCount;
    }

    public boolean getIsDeviceSupported() {
        return m_support && (m_audioCodecCount == -1 || m_audioCodecCount >= 6) && (getAVCSupported() || getMPEGV4Supported());
    }

    // Returns 0: upgrading doesn't improve support
    //        >0: upgading to SDK level matching return value will allow support
    public int getSupportIfUpgradeVersion() {
        return m_supportIfUpgradeVersion;
    }

    /*	TODO : yoon callback process ????
        public BailActivity.ReasonCode getDeviceNotSupportedReason() {
            if( !m_support )
                return BailActivity.ReasonCode.DeviceNoSupport;
            else if( !getAVCSupported() && !getMPEGV4Supported() )
                return BailActivity.ReasonCode.NoAVCOrMP4Support;
            else if( m_audioCodecCount < -1 )
                return BailActivity.ReasonCode.InternalError;
            else if( m_audioCodecCount > -1 && m_audioCodecCount < 6 )
                return BailActivity.ReasonCode.NotEnoughAudioCodecInstances;
            else
                return BailActivity.ReasonCode.DeviceNoSupportUnknownReason;
        }
    */
    public boolean getSupportsVideoLayers(boolean usingSoftwareCodec) {
        return true;
    }

    public boolean getUseAndroidJPEGDecoder() {
        return (getIntProperty("use_android_jpeg_dec", 1) == 1);
    }

    public int getMaxSupportedVideoBitrate(int iDefault) {
        return getIntProperty("max_supported_video_bitrate", iDefault);
    }

    public int getMaxSupportedAudioSamplingRate(int iDefault) {
        return getIntProperty("max_supported_audio_samplingrate", iDefault);
    }

    public int getMaxSupportedAudioChannels(int iDefault) {
        return getIntProperty("max_supported_audio_channels", iDefault);
    }

    public Size getCaptureSize() {
        if (m_maxCaptureSize == null) {
            int maxCaptureWidth = 1920;
            int maxCaptureHeight = 1080;
            m_maxCaptureSize = new Size(maxCaptureWidth, maxCaptureHeight);
        }
        return m_maxCaptureSize;
    }

    public boolean getWaitToGLFinish() {
        return getBooleanProperty("use_wait_gl_finish", false);
    }

    public boolean getSupportFrameTimeChecker() {
        return getBooleanProperty("use_frame_time_checker", true);
    }

    public boolean getUseMediaExtractor() {
        return (getIntProperty("use_mediaextractor", 1) == 1);
    }

    public DeviceSupportResponse getDSR() {
        return m_dsr;
    }
}
