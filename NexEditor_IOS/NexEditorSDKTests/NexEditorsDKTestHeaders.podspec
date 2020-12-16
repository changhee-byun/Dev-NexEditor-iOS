#
# Be sure to run `pod lib lint NexEditorFramework.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'NexEditorSDKTestHeaders'

  project = Xcodeproj::Project.open('../NexEditorFramework/NexEditorFramework.xcodeproj')
  target = project.targets.first
  version = target.build_settings('Debug')['CURRENT_PROJECT_VERSION']

  s.version          = version
  s.summary          = 'Dummy pod for Test project to allow access to private headers of NexEditorFrameowk'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
NexStreaming Corp. Internal
NexEditor framework for internal development
                       DESC

  s.homepage         = 'http://gerrit.nexstreaming.com:8080/#/admin/projects/NexEditor_IOS'
  # s.screenshots     = 'www.example.com/screenshots_1', 'www.example.com/screenshots_2'
  s.license          = { :type => 'Proprietary', :file => 'LICENSE' }
  s.author           = { 'NexStreaming Corp.' => 'simon.kim@nexstreaming.com' }
  #s.source           = { :git => 'ssh://gerrit.nexstreaming.com:29418/NexEditor_IOS', :tag => s.version.to_s }
  s.source           = { :git => 'ssh://gerrit.nexstreaming.com:29418/NexEditor_IOS', :branch => 'master' }

  s.requires_arc = false
  s.ios.deployment_target = '8.0'

  s.requires_arc = [
    ]
  s.source_files = [
    'NexEditorSDKTestHeaders.h',
    'NexEditorSDKTestHeaders.m',
    ]                    
  s.public_header_files = [
    'NexEditorSDKTestHeaders.h',
    ]
  s.exclude_files = [
    ]
  s.vendored_frameworks = [
    ]
  s.vendored_libraries = [
    ]

  s.pod_target_xcconfig = { "ENABLE_BITCODE" => "NO", 
                            #'HEADER_SEARCH_PATHS' => '"${PODS_TARGET_SRCROOT}/submodules/NexEditor/NexEditorSDK/nexeditorengine/extern_lib/RenderItemSDK/renderitem"' 
  }
  s.user_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => '"$(SRCROOT)/../NexEditorFramework/NexEditorFramework/include/**" "$(SRCROOT)/../NexEditorFramework/NexEditorFramework/nexeditor/**" "$(SRCROOT)/../NexEditorFramework/NexEditorFramework/include/assetlib" "$(SRCROOT)/../NexEditorFramework/AssetReader/AssetReader/include"'
  }

  #s.resource = 'NexEditorFramework/NexEditorFramework/resource'

  #s.dependency 'AssetReader'
  #s.dependency 'NexEditorEngine'
end
