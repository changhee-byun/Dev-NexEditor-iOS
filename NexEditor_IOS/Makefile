TARGET_FRAMEWORK_DIR=$(shell pwd)/Frameworks/

all: package samples

samples: framework
	$(MAKE) -C Samples 

assets: submodule-updates
	$(MAKE) -C EditorAssets

submodule-updates:
	@git submodule init
	@git submodule update

framework:
	@rm -rf $(TARGET_FRAMEWORK_DIR)/*.framework
	@rm -rf $(TARGET_FRAMEWORK_DIR)/dist/*.framework
	$(MAKE) -C NexEditorFramework OUTPUT_DIR=$(TARGET_FRAMEWORK_DIR)

package: framework
	@bash ./package.sh


# Push Pods to Crop's internal repo
push-pods:
	$(MAKE) -C Podspecs
