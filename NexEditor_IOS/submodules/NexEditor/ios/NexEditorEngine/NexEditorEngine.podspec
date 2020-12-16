#
# Be sure to run `pod lib lint NexEditorEngine.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'NexEditorEngine'

  version = "2.12.41"

  s.version          = version
  s.summary          = 'NexEditor engine for internal development'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
NexStreaming Corp. Internal
NexEditor engine for internal development
                       DESC

  s.homepage         = 'http://gerrit.nexstreaming.com:8080/#/admin/projects/NexEditor'
  s.license          = { :type => 'Proprietary', :file => 'NexEditorFramework/LICENSE' }
  s.author           = { 'NexStreaming Corp.' => 'simon.kim@nexstreaming.com' }
  # tag: prefix 'ios-r' for release, 'ios-rc' for development version
  s.source           = { :git => 'ssh://gerrit.nexstreaming.com:29418/NexEditor', 
                         :tag => 'ios-r' + s.version.to_s, 
                         :submodules => false 
                        }

  s.requires_arc = false
  s.ios.deployment_target = '9.0'

  s.subspec 'NexEditor' do |sp|
    sp.source_files = [
    'ios/NexEditorEngine/include/*.h',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/**/*.{h}',
    'NexEditorSDK/nexeditorengine/extern_lib/nexCAL/**/*.{c,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/NexChunkParser/**/*.{c,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/NexCommon/**/*.{h}',
    'NexEditorSDK/nexeditorengine/extern_lib/NexDataStruct/**/*.{c,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/NEXResampler/**/*.h',
    'NexEditorSDK/nexeditorengine/extern_lib/nexRAL/**/*.{c,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/nexSAL/**/*.{c,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/NexSecureware/**/*.{cpp,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/nexUtil/**/*.{c,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/**/*.{h,c,cpp,hpp}',
    'NexEditorSDK/nexeditorengine/extern_lib/AES/**/*.{c,h,hpp}',
    'NexEditorSDK/nexeditorengine/nexEditorUtil/**/*.{c,cpp,h,hpp}',
    'NexEditorSDK/nexeditorengine/nexLayer/**/*.{h,hpp}',
    'NexEditorSDK/nexeditorengine/nexLayer/porting_Src/ios/*.{m,mm,swift}',
    'NexEditorSDK/nexeditorengine/nexTheme/**/*.h',
    'NexEditorSDK/nexeditorengine/nexTheme/src/*.cpp',
    'NexEditorSDK/nexeditorengine/nexTheme/porting_Src/ios/*.{m,mm,swift}',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/**/*.{c,cpp,h}',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/**/*.{m,mm,h,hpp,cpp,c,swift}',
    'ios/NexEffectRenderer/NexThemeRenderer_Platform/**/*.{m,mm,h,swift}',
    ]
    sp.public_header_files = [
    'ios/NexEditorEngine/include/*.h',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/inc/NEXVIDEOEDITOR_EventTypes.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/NexSceneInfo.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/NexDrawInfo.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/NexEditorEngine.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/NexEditor.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/TextEffecter/TextEffecter.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/NexEditorUtil.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/NexClipInfo.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/EditorEventListener.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/ImageLoaderProtocol.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/CustomLayerProtocol.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/LUTLoaderProtocol.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/VignetteLoaderProtocol.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/NexEditorConfiguration.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/nexlog/NexEditorLog.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/kminternal/NXEAudioClip.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/kminternal/NXEVisualClip.h',
    'NexEditorSDK/nexeditorengine/porting_Src/ios/kminternal/NXEVisualClipInternal.h',
    'NexEditorSDK/nexeditorengine/nexLayer/porting_Src/ios/NexLayer.h',
    'NexEditorSDK/nexeditorengine/nexLayer/porting_Src/ios/NexColorMatrix.h',
    'NexEditorSDK/nexeditorengine/extern_lib/NexCommon/**/*.h',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/inc/NEXVIDEOEDITOR_Error.h',
    ]
    sp.exclude_files = ['NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/commonlib/lua-5.3.1/src/lua*.c',
    'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/image_dropbox.*',
    'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/main.cpp',
    'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/message_processor.{cpp,h}',
    'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/slider.*',
    'NexEditorSDK/nexeditorengine/nexTheme/src/NexTheme_jni.*',
    'NexEditorSDK/nexeditorengine/nexTheme/src/NXT*.*',
    'NexEditorSDK/nexeditorengine/nexTheme/src/NexThemeRenderer_SurfaceTexture.*',
    'NexEditorSDK/nexeditorengine/nexTheme/inc/NexThemeRenderer_SurfaceTexture.*',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/porting_src/porting_android*.*',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/nexSeperatorByte_.c',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_ClockManager.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_DirectExportTask.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_FastPreviewTask.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_Highlighter.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_HighlightDiffChecker.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_PFrameProcessTask.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_ReverseTask.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_TranscodingTask.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_UDTA.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_SupportDevices.cpp',
    'NexEditorSDK/nexeditorengine/nexVideoEditor/src/NEXVIDEOEDITOR_PerformanceChecker.cpp',
    ]
    sp.vendored_libraries = [
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NxFFReader/v2.61.03/libNxFFReader_ios_na_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NxFFWriter/3.19.7/libNxFFWriter_ios_na_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NxFFSubtitle/v2.19.1/libNxFFSubtitle_ios_na_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NexCodecUtils/v2.1.16/libNexCodecUtil_ios_arm_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NexCodecUtils/v2.1.16/libNexCodecUtil_ios_simulator_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/AAC_Decoder/libNxAACDec_ios_arm_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/AAC_Decoder/libNxAACDec_ios_simulator_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NexSound/v3.9.18/libNexsound_iOS_ARM_Release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NexSound/v3.9.18/libNexsound_iOS_Simulator_Release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NxXMLParser/v2.4.0/libnxXMLParser_ios_na_debug.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NEXMixer/v1.1.0/libNexMixer_iOS_ARM_release.a',
    'NexEditorSDK/nexeditorengine/extern_lib/ios/NEXMixer/v1.1.0/libNexMixer_iOS_Simulator_Release.a'
    ]
    sp.pod_target_xcconfig = {'HEADER_SEARCH_PATHS' => '"${PODS_TARGET_SRCROOT}/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem"' }
  end

  s.pod_target_xcconfig = { "ENABLE_BITCODE" => "NO" }

  s.framework = 'Photos', 'AVFoundation'
  s.library = 'z'

end
