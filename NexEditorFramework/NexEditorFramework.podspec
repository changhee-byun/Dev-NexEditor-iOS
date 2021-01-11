#
# Be sure to run `pod lib lint NexEditorFramework.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'NexEditorFramework'

  # project = Xcodeproj::Project.open('NexEditorFramework/NexEditorFramework.xcodeproj')
  # target = project.targets.first
  # version = target.build_settings('Debug')['CURRENT_PROJECT_VERSION']
  version = "1.7.0"

  s.version          = version
  s.summary          = 'NexEditor framework for internal development'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
NexStreaming Corp. Internal
NexEditor framework for internal development
                       DESC

  s.homepage         = 'https://github.com/KineMasterCorp/NexEditor'
  # s.screenshots     = 'www.example.com/screenshots_1', 'www.example.com/screenshots_2'
  s.license          = { :type => 'Proprietary', :file => 'LICENSE' }
  s.author           = { 'KineMaster Corp.' => 'editor@kinemaster.com' }
  #s.source           = { :git => 'ssh://gerrit.nexstreaming.com:29418/NexEditor_IOS', :tag => s.version.to_s }
  s.source           = { 
  :git => 'https://github.com/changhee-byun/Dev-NexEditor-iOS.git', 
  :branch => 'main' 
}

  s.requires_arc = false
  s.ios.deployment_target = '12.0'
  s.swift_version = '5.0'

  # s.requires_arc = [
  #   'NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/*.{h,m}',
  #   'NexEditorFramework/AssetReaderModule/**/*.{h,m,mm,cpp,hpp}'
  #   ]
  s.source_files = [
    'NexEditorFramework/NexEditorFramework/NexEditorFramework/**/*.{h,m,c,mm,cpp,hpp,swift}',
    'NexEditorFramework/NexEditorFramework/AssetReaderModule/**/*.{h,m,mm,cpp,hpp}'
    ]                    
  s.public_header_files = ['NexEditorFramework/NexEditorFramework/NexEditorFramework/include/*.h',
    'NexEditorFramework/NexEditorFramework/NexEditorFramework/include/assetlib/*.h',
    'NexEditorFramework/NexEditorFramework/NexEditorFramework/include/collage/*.h',
    'NexEditorFramework/NexEditorFramework/NexEditorFramework/include/template/*.h',
    'NexEditorFramework/NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/CollageInfo.h',
    'NexEditorFramework/NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/CollageType.h',
    'NexEditorFramework/NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/CollageTextStyle.h',
    'NexEditorFramework/NexEditorFramework/AssetReaderModule/include/*.h'
    ]

  # arc_files = 'NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/*.{h,m}', 'NexEditorFramework/AssetReaderModule/**/*.{h,m,mm,cpp,hpp}'

  s.exclude_files = ['NexEditorFramework/NexEditorFramework/NexEditorFramework/Keyczar/Framework/**/*',
  'NexEditorFramework/NexEditorFramework/NexEditorFramework/Frameworks/Framework/keyczar.framework/**/*',
  'NexEditorFramework/NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/*.{h,m}', 
  'NexEditorFramework/NexEditorFramework/AssetReaderModule/**/*.{h,m,mm,cpp,hpp}'
  ]

  s.subspec 'arc-specific' do |sna|
    sna.requires_arc = true
    sna.source_files = [ 'NexEditorFramework/NexEditorFramework/NexEditorFramework/nexeditor/collageLoader/*.{h,m}', 'NexEditorFramework/NexEditorFramework/AssetReaderModule/**/*.{h,m,mm,cpp,hpp}' ]
  end


  s.vendored_frameworks = 'NexEditorFramework/NexEditorFramework/Frameworks/Framework/zlib.framework'
  s.vendored_libraries = [
    ]

  s.pod_target_xcconfig = { "ENABLE_BITCODE" => "NO", 
                            'HEADER_SEARCH_PATHS' => '"${PODS_TARGET_SRCROOT}/submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem"',
                            'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'arm64' }
                            
  s.resource = 'NexEditorFramework/NexEditorFramework/NexEditorFramework/resource'

  s.dependency 'NexEditorEngine'
end
