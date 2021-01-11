ifeq ($(OUTPUT_DIR),)
	OUTPUT_DIR=Output
endif

TARGET=$(OUTPUT_DIR)/NexEditorFramework.framework

XCODEBUILD_COMMON_FLAGS=-workspace NexEditorFramework.xcworkspace
all: $(TARGET)

$(TARGET):
	@echo OUTPUT_DIR: $(OUTPUT_DIR)
	@xcodebuild $(XCB_COMMON_FLAGS) -scheme NexEditorFramework-Universal clean build

clean:
	@rm -rf $(TARGET)
	@xcodebuild $(XCB_COMMON_FLAGS) -scheme NexEditorFramework clean -configuration Release -sdk iphonesimulator
	@xcodebuild $(XCB_COMMON_FLAGS) -scheme NexEditorFramework clean -configuration Release -sdk iphoneos
