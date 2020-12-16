OUTPUT_BIN_DIR=./bin

# TODO: rename linux/ to cmdtoo/ later
CMD_TOOL=linux

MAC_APP_NAME=NexSecureBinaryEditor.app
MAC_APP=build/Release/$(MAC_APP_NAME)
MAC_APP_BIN=$(OUTPUT_BIN_DIR)/$(MAC_APP_NAME)

CMD_TOOL_BIN=$(OUTPUT_BIN_DIR)/securetool

TARGETS=macapp cmdtool
all:  $(TARGETS)

macapp: $(MAC_APP_BIN)
cmdtool: $(CMD_TOOL_BIN)

$(MAC_APP):
	@xcodebuild

$(OUTPUT_BIN_DIR):
	@mkdir -p $(OUTPUT_BIN_DIR)

$(MAC_APP_BIN): $(MAC_APP) $(OUTPUT_BIN_DIR)
	@mv $(MAC_APP) $(MAC_APP_BIN)


$(CMD_TOOL_BIN): $(OUTPUT_BIN_DIR)
	$(MAKE) -C $(CMD_TOOL)

clean:
	rm -rf $(MAC_APP_BIN)
	xcodebuild clean
	$(MAKE) -C $(CMD_TOOL) clean
