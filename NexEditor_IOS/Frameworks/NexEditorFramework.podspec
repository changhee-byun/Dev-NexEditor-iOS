#
# Be sure to run `pod lib lint NexEditorDev.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'NexEditorFramework'
  s.version          = '1.6.0'
  s.summary          = 'NexEditor framework'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
NexEditor SDK, NexStreaming Corp. 
                       DESC

  s.homepage         = 'http://www.nexstreaming.com'
  s.license          = { :type => 'Proprietary', :file => 'LICENSE' }
  s.author           = { 'NexStreaming Corp.' => 'simon.kim@nexstreaming.com' }
  s.source           = { :path => '.'}

  s.ios.deployment_target = '9.0'
  s.ios.vendored_frameworks = 'NexEditorFramework.framework'
end
