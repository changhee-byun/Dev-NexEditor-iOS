#
# Be sure to run `pod lib lint NexEditorFramework.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'EditorAssets'

  s.version          = '0.0.1' 
  s.summary          = 'NexEditor asset bundles'

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
  s.license          = { :type => 'Proprietary', :file => 'LICENSE' }
  s.author           = { 'NexStreaming Corp.' => 'simon.kim@nexstreaming.com' }
  s.source           = { :git => 'ssh://gerrit.nexstreaming.com:29418/NexEditor_IOS', :branch => 'master' }

  s.resource = '*.bundle'
end
