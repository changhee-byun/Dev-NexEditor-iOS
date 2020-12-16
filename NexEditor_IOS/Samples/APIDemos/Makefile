PODFILE_DIST=Podfile.dist

all: app

clean:
	@xcodebuild -workspace APIDemos.xcworkspace -scheme APIDemos -sdk iphonesimulator clean

app: Podfile
	@xcodebuild -workspace APIDemos.xcworkspace -scheme APIDemos -sdk iphonesimulator 

pod-dist: Podfile.dist
	@cp $(PODFILE_DIST) Podfile
	pod install

app-dist: pod-dist
	@$(MAKE) app

test:
	@fastlane scan -s APIDemos -a 'iPhone 7 Plus'
