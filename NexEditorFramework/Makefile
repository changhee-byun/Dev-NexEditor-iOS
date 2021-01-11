TARGETS=debug

REPONAME=NexPods
REPOQUERY=$(shell pod repo list | grep "$(REPONAME)")
PODSPEC='NexEditorFramework.podspec'

ifeq ($(REPOQUERY),)
TARGETS=validate-repo
endif

USER_TAGNAME=v$(version)

DEPENDENCY_SOURCES='https://github.com/KineMasterCorp/NexPods.git,https://github.com/CocoaPods/Specs'

# To push the pod,
# $ make push

all: $(TARGETS)

push:
	@pod repo push --sources=$(DEPENDENCY_SOURCES) $(REPONAME) $(PODSPEC) --allow-warnings --verbose

push-with-tag: git-push-tag push	

release:
	@pod spec lint --clean --sources=$(DEPENDENCY_SOURCES) $(PODSPEC) --allow-warnings --verbose

debug:
	@pod lib lint --clean --sources=$(DEPENDENCY_SOURCES) $(PODSPEC) --verbose --allow-warnings

debug-no-clean:
	@pod lib lint --sources=$(DEPENDENCY_SOURCES) $(PODSPEC) --verbose --allow-warnings --no-clean

release-no-clean:
	@pod spec lint --sources=$(DEPENDENCY_SOURCES) $(PODSPEC) --verbose --allow-warnings --no-clean

validate-repo:
	@if [ "$(REPOQUERY)" == "" ]; then echo "== ERROR $(REPONAME) not found. Add $(REPONAME) with 'pod repo add $(REPONAME) <URL>' command"; exit -1; fi

git-push-tag:
	@test -n "$(version)" || (echo "ERROR: add version information as below.\n 'make $@ version=<ENGINE VERION>'" ; exit 1)
	@echo "\n"
	@echo "Warning: Are you sure to push the tag name '$(USER_TAGNAME)' ? [Y/n]";\
	read line; if [ $$line != "" ] && [ $$line != "y" ] && [ $$line != "Y" ]; then echo aborting; exit 1 ; fi;\
	git tag $(USER_TAGNAME)
	git push origin $(USER_TAGNAME)

