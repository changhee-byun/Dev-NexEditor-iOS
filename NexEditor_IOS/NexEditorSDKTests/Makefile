XCODEBUILD_COMMON_FLAGS=-workspace NexEditorSDKTests.xcworkspace
all: 
	@xcodebuild $(XCB_COMMON_FLAGS) -scheme NexEditorSDKTest clean build

clean:
	@rm -rf $(TARGET)
	@xcodebuild $(XCB_COMMON_FLAGS) -scheme NexEditorSDKTest clean -configuration Release -sdk iphonesimulator
	@xcodebuild $(XCB_COMMON_FLAGS) -scheme NexEditorSDKTest clean -configuration Release -sdk iphoneos
