TARGETS=lint

REPONAME=NexPods
REPOQUERY=$(shell pod repo list | grep "$(REPONAME)")
PODSPEC=NexEditorEngine.podspec

ifeq ($(REPOQUERY),)
TARGETS=validate-repo
endif
# To push the pod,
# $ make push
#
all: $(TARGETS)

push:
	@pod repo push $(REPONAME) $(PODSPEC) --allow-warnings

validate-repo:
	@if [ "$(REPOQUERY)" == "" ]; then echo "== ERROR $(REPONAME) not found. Add $(REPONAME) with 'pod repo add $(REPONAME) <URL>' command"; exit -1; fi

lint:
	@pod spec lint --clean $(PODSPEC) --allow-warnings --verbose

