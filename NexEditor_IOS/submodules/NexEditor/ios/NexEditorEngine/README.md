
# How to use NexEditorEngine Pod.

## Configure default ssh account to ssh://gerrit.nexstreaming.com

Edit ~/.ssh/config and add following lines with your account.
```
Host gerrit.nexstreaming.com
    HostName gerrit.nexstreaming.com
    User your.name
```

## Using NexEditorEngine pod from other projects
    
1. Add NexPods repo, once is enough
```
    $ pod repo add NexPods ssh://gerrit.nexstreaming.com:29418/NexPods
```
2. Edit Podfile to add source Git URL to NexPods and 'NexEditorEngine' pod
```
    source 'ssh://gerrit.nexstreaming.com:29418/NexPods'
    ...
    pod 'NexEditorEngine', '~> 1.2.3'
    ...
```
3. Update Pods/, once per NexEditorEngine version is enough
```
    $ pod install
```

## Pushing udated NexEditorEngine Pod (from NexEditor_IOS project)
```
    $ pod repo add NexPods ssh://gerrit.nexstreaming.com:29418/NexPods
    $ make push-pods
```

