#
# This podspec is for NexEditor2 Project
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#


Pod::Spec.new do |s|
  s.name             = 'NexEditorForNE2'
  s.version          = '1.0'
  s.summary          = 'NexEditor for NexEditor2 internal development'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
NexStreaming Corp. Internal
NexEditor for NexEditor2 internal development
                       DESC

  s.homepage         = 'http://gerrit.nexstreaming.com:8080/#/admin/projects/NexEditor_IOS'
  s.license          = { :type => 'Proprietary', :file => 'LICENSE' }
  s.author           = { 'NexStreaming Corp.' => 'jake.you@nexstreaming.com' }
  s.source           = { :git => 'ssh://gerrit.nexstreaming.com:29418/NexEditor_IOS', :branch => 'rc1.5.0' }

  s.requires_arc = false
  s.ios.deployment_target = '8.0'

  s.requires_arc = ['NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/*.{h,m}',
                    'NexEditorFramework/AssetReader/AssetReader/include/*.*',
                    'NexEditorFramework/AssetReader/AssetReader/src/**/*.*',
                    'NexEditorFramework/AssetReader/AssetReader/src/**/**/*.*',
    ]
  s.source_files = ['NexEditorFramework/NexEditorFramework/**/*.{h,m,c,mm,cpp,hpp,swift}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/**/*.{h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/nexCAL/**/*.{c,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/NexChunkParser/**/*.{c,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/NexCommon/**/*.{h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/NexDataStruct/**/*.{c,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/NEXResampler/**/*.h',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/nexRAL/**/*.{c,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/nexSAL/**/*.{c,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/NexSecureware/**/*.{cpp,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/nexUtil/**/*.{c,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/**/*.{h,c,cpp,hpp}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexEditorUtil/**/*.{c,cpp,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexLayer/**/*.{h,hpp}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexLayer/porting_Src/ios/*.{m,mm,swift}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/**/*.h',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/src/*.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/porting_Src/ios/*.{m,mm,swift}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/**/*.{c,cpp,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/porting_Src/ios/**/*.{m,mm,h,hpp,cpp,c,swift}',
    'submodules/NexEditor/ios/NexEffectRenderer/NexThemeRenderer_Platform/*.{m,mm,h}',
    'NexEditorFramework/AssetReader/AssetReader/include/*.*',
    'NexEditorFramework/AssetReader/AssetReader/src/**/*.*',
    'NexEditorFramework/AssetReader/AssetReader/src/**/**/*.*',
    ]                    
  s.public_header_files = [
    'NexEditorFramework/NexEditorFramework/include/assetlib/*.h',
    'NexEditorFramework/NexEditorFramework/include/NXEEditorType.h',
    'NexEditorFramework/NexEditorFramework/include/NXEEditorErrorCode.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/*.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/asset/*.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/asset/proxy/*.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/asset/decryption/*.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/apis/NXEAssetItemPrivate.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/externalResourceProvider/*.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/editorUtil/*.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/packages/UserField.h',
    'NexEditorFramework/NexEditorFramework/nexeditor/common/MathUtil.h',
    'NexEditorFramework/AssetReader/AssetReader/include/*.h',
    'NexEditorFramework/AssetReader/AssetReader/src/**/*.h',
    'NexEditorFramework/AssetReader/AssetReader/src/**/**/*.h',
    'submodules/NexEditor/ios/NexEffectRenderer/NexThemeRenderer_Platform/NexThemeRenderer_Platform_iOS.h',
    'submodules/NexEditor/ios/NexEffectRenderer/NexThemeRenderer_Platform/NexThemeRenderer_Platform_iOS-API.h',
    'submodules/NexEditor/ios/NexEffectRenderer/NexThemeRenderer_Platform/NexThemeRenderer_Platform_iOS-CAPI.h',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/inc/NexThemeRenderer.h',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/inc/NexThemeRenderer_Platform.h',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/inc/NexTheme_Errors.h',
    ]
  s.exclude_files = ['NexEditorFramework/NexEditorFramework/Keyczar/Framework/**/*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/commonlib/lua-5.3.1/src/lua*.c',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/image_dropbox.*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/main.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/message_processor.{cpp,h}',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/slider.*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/src/NexTheme_jni.*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/src/NXT*.*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/src/NexThemeRenderer_SurfaceTexture.*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexTheme/inc/NexThemeRenderer_SurfaceTexture.*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/porting_src/porting_android*.*',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/nexSeperatorByte_.c',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_ClockManager.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_DirectExportTask.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_FastPreviewTask.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_HighlightDiffChecker.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_PFrameProcessTask.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_ReverseTask.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_TranscodingTask.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_UDTA.cpp',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_SupportDevices.cpp',
    ]
  s.vendored_frameworks = 'NexEditorFramework/Frameworks/Framework/keyczar.framework', 'NexEditorFramework/Frameworks/Framework/zlib.framework'
  s.vendored_libraries = ['submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NxFFReader/v2.60.4/libNxFFReader_ios_na_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NxFFWriter/3.19.6/libNxFFWriter_ios_na_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NxFFSubtitle/v2.19.1/libNxFFSubtitle_ios_na_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NexCodecUtils/v2.1.6/libNexCodecUtil_ios_arm_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NexCodecUtils/v2.1.6/libNexCodecUtil_ios_simulator_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/AAC_Decoder/libNxAACDec_ios_arm_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/AAC_Decoder/libNxAACDec_ios_simulator_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NexSound/v3.8.5/libNexsound_iOS_ARM_Release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NexSound/v3.8.5/libNexsound_iOS_Simulator_Release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NxXMLParser/v2.4.0/libnxXMLParser_ios_na_debug.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NEXMixer/v1.0.0/libNexMixer_iOS_ARM_release.a',
    'submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/ios/NEXMixer/v1.0.0/libNexMixer_iOS_Simulator_Release.a'
    ]

  s.pod_target_xcconfig = { "ENABLE_BITCODE" => "NO", 
                            'HEADER_SEARCH_PATHS' => '"${PODS_TARGET_SRCROOT}/submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem"' }

  s.resource = 'NexEditorFramework/NexEditorFramework/resource'

end
