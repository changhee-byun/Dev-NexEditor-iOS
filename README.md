# NexEditorSDK-iOS

## Build NexEditorFramework

1. Install NexEditorEngine pod from NexPods.
```
  $ cd NexEditorFramework
  $ ./pod install
```

2. Build the NexEditorFramework project.

* Open "NexEditorFramework.xcworkspace".
* Build the project.\
* __'NexEditorFramework.framework'__ is generated.

## Build EditorDemo [Demo application]

1. Install NexEditorEngine and NexEditorFramework pod from NexPods.
```
  $ cd EditorDemo
  $ ./pod install
```

2. Build the EditorDemo project.

* Open "EditorDemo.xcworkspace".
* Build the project.
* Run app.


## Distribute NexEditorFramework with CocoaPods [ __whole sources, not  framework.__]

### Testing
```
  $ ./make 
  or
  $./make debug
  or
  $./make release
```

### Release with CocoaPods
```
  $ ./make push
  or
  $./make push-with-tag version=<Version>
```
