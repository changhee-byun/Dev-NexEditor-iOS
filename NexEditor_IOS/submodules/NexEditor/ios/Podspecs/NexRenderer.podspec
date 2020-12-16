#
# Be sure to run `pod lib lint NexEditorEngine.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'NexRenderer'

  version = "0.10.5"

  s.version          = version
  s.summary          = 'Video Rendering framework from NexStreaming'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
NexStreaming Corp. Internal
NexRenderer from NexStreaming
                       DESC

  s.homepage         = 'http://gerrit.nexstreaming.com:8080/#/admin/projects/NexEditor'
  s.license          = { :type => 'Proprietary', :file => 'ios/Podspecs/LICENSE' }
  s.author           = { 'NexStreaming Corp.' => 'simon.kim@nexstreaming.com' }
  s.source           = { :git => 'ssh://gerrit.nexstreaming.com:29418/NexEditor', :tag => 'ios-nexrenderer-pod-' + s.version.to_s }

  s.requires_arc = false
  s.ios.deployment_target = '9.0'

  s.source_files = [
                        'NexEditorSDK/nexeditorengine/nexTheme/**/*.{h,m,c,mm,cpp,hpp,swift}',
                        'ios/NexEffectRenderer/NexThemeRenderer_Platform/*.{h,m,c,mm,cpp,hpp,swift}',
                        'NexEditorSDK/nexeditorengine/nexEditorUtil/inc/nexTexturesLogger.h',
                        'NexEditorSDK/nexeditorengine/extern_lib/nexSAL/**/*.{h,c}',
                        'NexEditorSDK/nexeditorengine/extern_lib/NexCommon/**/*.h',
                        'NexEditorSDK/nexeditorengine/extern_lib/NxXMLParser/**/*.h',
                        'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/**/*.{h,c,cpp,hpp}',
                    ]
  s.exclude_files = [
                        'NexEditorSDK/nexeditorengine/nexTheme/src/NexTheme_jni.*',
                        'NexEditorSDK/nexeditorengine/nexTheme/src/NexThemeRenderer_SurfaceTexture.*',
                        'NexEditorSDK/nexeditorengine/nexTheme/src/NXT*.*',
                        'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/image_dropbox.*',
                        'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/main.*',
                        'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/slider.*',
                        'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem/message_processor.*',
                        'NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/commonlib/lua-5.3.1/src/lua*.c',
                    ]

  s.public_header_files = [
                            'NexEditorSDK/nexeditorengine/nexTheme/inc/NexThemeRenderer.h',
                            'NexEditorSDK/nexeditorengine/nexTheme/inc/NexTheme_Errors.h',
                            'NexEditorSDK/nexeditorengine/nexTheme/inc/NexThemeRenderer_Platform.h',
                            'ios/NexEffectRenderer/NexThemeRenderer_Platform/NexThemeRenderer_Platform_iOS.h',
                            'ios/NexEffectRenderer/NexThemeRenderer_Platform/NexThemeRenderer_Platform_iOS-API.h',
                            'ios/NexEffectRenderer/NexThemeRenderer_Platform/NexThemeRenderer_Platform_iOS-CAPI.h',
                            ]
  s.vendored_libraries = [
                          'NexEditorSDK/nexeditorengine/extern_lib/NxXMLParser/v2.4.0/libnxXMLParser_ios_na_debug.a',
                         ]

  s.pod_target_xcconfig = { "ENABLE_BITCODE" => "NO" }
  s.framework = 'Photos', 'AVFoundation'
  s.library = 'z'

end
