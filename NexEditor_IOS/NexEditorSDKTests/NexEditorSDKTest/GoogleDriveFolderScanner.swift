/**
 * File Name   : GoogleDriveFolderScanner.swift
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

import Foundation
import GoogleAPIClientForREST
import Zip

class GoogleDriveFolderScannerStateHolder {
    var downloaders: [RateLimitedDownloader] = []
    
    init() {
        
    }
}

class RateLimitedDownloader {
    private let service: GTLRDriveService
    private let limit = 8
    private var concurrent = 0
    private let unzip: Bool
    var list: [(file: GTLRDrive_File, directory: URL)]
    
    var complete: (()->Void)? = nil
    
    init(_ list: [(file: GTLRDrive_File, directory: URL)], service: GTLRDriveService, unzip: Bool) {
        self.service = service
        self.list = list
        self.unzip = unzip
    }
    
    func start(_ complete: @escaping () -> Void ) {
        if list.count == 0  {
            complete()
            return
        }
        self.complete = complete

        while concurrent < limit && list.count > 0 {
            downloadNext()
        }
    }
    
    private func unzipInplace(_ url: URL) {
        do {
            let unzipDestUrl = url.deletingPathExtension()
            let unzipParentUrl = try Zip.quickUnzipFile(url)
            // Zip workaround. It creates another sub-folder with the same name. hello.zip -> hello/hello/*
            let unzipUrl = unzipParentUrl.appendingPathComponent(url.deletingPathExtension().lastPathComponent)
            print("Unzipped to: \(unzipUrl)")
            try FileManager.default.removeItem(at: url)
            if unzipDestUrl != unzipUrl {
                if FileManager.default.fileExists(atPath: unzipDestUrl.path) {
                    try FileManager.default.removeItem(at: unzipDestUrl)
                }
                try FileManager.default.moveItem(at: unzipUrl, to: unzipDestUrl)
                try FileManager.default.removeItem(at: unzipParentUrl)
                print("Moved to: \(unzipDestUrl)")
            }
        } catch let e {
            print(e.localizedDescription)
        }
    }
    
    private func downloadNext() {
        if let first = list.first {
            list.remove(at: 0)
            
            concurrent += 1
            download(file: first.file, directory: first.directory) { ( file, fileUrl, error ) in
                self.concurrent -= 1
                if error == nil && self.unzip && fileUrl.pathExtension == "zip" {
                    DispatchQueue.global().async {
                        self.unzipInplace(fileUrl)
                    }
                }
                
                if self.concurrent < self.limit {
                    self.downloadNext()
                }
                
            }
        } else {
            if concurrent == 0 {
                complete?()
                complete = nil
            }
        }
    }
    
    func download(file: GTLRDrive_File, directory: URL, complete: @escaping (_ file: GTLRDrive_File, _ fileUrl: URL, _ error: Error?)->Void ) {
        
        if let fileId = file.identifier, let fileName = file.name {
            
            let query = GTLRDriveQuery_FilesGet.queryForMedia(withFileId: fileId)
            let request = service.request(for: query) as URLRequest
            print("Downloading \(fileName) \(request)")
            
            let fetcher = service.fetcherService.fetcher(with: request )
            let fileUrl = directory.appendingPathComponent(fileName)
            fetcher.destinationFileURL = fileUrl
            fetcher.beginFetch(completionHandler: { (data, error) in
                complete( file, fileUrl, error )
            })
        }
    }
    
}

struct GoogleDriveFolderScanner {
    let service: GTLRDriveService
    let stateHolder = GoogleDriveFolderScannerStateHolder()
    
    // finds a folder with specified name that contains a file with specified 'filename'
    func findFolder(_ name: String, containsFile filename: String, complete: @escaping (GTLRDrive_File?)->Void ) {
        
        findFolders(name) { (folders) in
            if folders.count > 0 {
                self.findFirstFolderHasFile(name: filename, in: folders, complete: complete)
            } else {
                complete(nil)
            }
        }
    }
    
    /// finds folders with specified name
    func findFolders(_ name: String, complete: @escaping ([GTLRDrive_File])->Void ) {
        let query = GTLRDriveQuery_FilesList.query()
        query.q = "mimeType='application/vnd.google-apps.folder' and name = '\(name)'"
        service.executeQuery(query) { (ticket, list, error) in
            var found: [GTLRDrive_File] = []
            if let result = list as? GTLRDrive_FileList, let folders = result.files, error == nil {
                found = folders
            }
            complete(found)
        }
    }
    
    func findFirstFolderHasFile( name: String, in folders: [GTLRDrive_File], complete: @escaping (GTLRDrive_File?)->Void) {
        var stop = false
        for folder in folders {
            if stop {
                break;
            }
            self.findFile(name: name, in: folder ) { file in
                if file != nil && !stop {
                    stop = true
                    complete(folder)
                }
            }
        }
    }
    
    func findFile(name: String, in folder: GTLRDrive_File, complete: @escaping (GTLRDrive_File?)->Void) {
        
        findFiles(in: folder) { files, error in
            var found: GTLRDrive_File? = nil
            for file in files {
                if file.name == name {
                    found = file
                    break
                }
            }
            complete(found)
        }
    }
    struct ScannerError: Error {
        enum Code {
            case badIdentifier
        }
        
        let type: Code
    }
    
    /// asynchrnously finds files in the Google drive folder
    /// result will be passed to the colsure. result will be empty array if 'folder' is not a folder, without identifier, or no files in the folder.
    func findFiles(in folder: GTLRDrive_File, complete: @escaping ([GTLRDrive_File], Error?)->Void) {
        guard let folderId = folder.identifier else {
            complete([], ScannerError(type: .badIdentifier))
            return
        }
        
        let query = GTLRDriveQuery_FilesList.query()
        query.q = "'\(folderId)' in parents and trashed = false"
        service.executeQuery(query) { (ticket, list, error) in
            var found: [GTLRDrive_File] = []
            

            if let result = list as? GTLRDrive_FileList, let files = result.files {
                found = files
            }
            if let error = error {
                print("findFiles: \(folder.name!) \(folderId) failed: \(error.localizedDescription)")
            }
            complete(found, error)
        }
    }
    
//    func download(_ files: [GTLRDrive_File]) {
//        let documentsUrl = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
//        let driveDirectoryUrl = documentsUrl.appendingPathComponent("drive", isDirectory: true)
//        
//        for file in files.prefix(1) {
//            download(file: file,
//                     directory: driveDirectoryUrl) { (file, error) in
//                        if let error = error {
//                            print("error: \(error)")
//                        } else {
//                            print("downloaded: \(file.name!)")
//                        }
//            }
//        }
//    }
//    
//    func download(file: GTLRDrive_File, directory: URL, complete: @escaping (_ file: GTLRDrive_File, _ error: Error?)->Void ) {
//        
//        if let fileId = file.identifier, let fileName = file.name {
//            
//            let query = GTLRDriveQuery_FilesGet.queryForMedia(withFileId: fileId)
//            let request = service.request(for: query) as URLRequest
//            print("Downloading \(request)")
//            
//            let fetcher = service.fetcherService.fetcher(with: request )
//            fetcher.destinationFileURL = directory.appendingPathComponent(fileName)
//            fetcher.beginFetch(completionHandler: { (data, error) in
//                complete( file, error )
//            })
//        }
//    }
    
    func scanDownloadList(folder: GTLRDrive_File, directory url: URL, subpath: String?, complete: @escaping ([(file: GTLRDrive_File, directory: URL)])->Void ) {
        var result: [(file: GTLRDrive_File, directory: URL)] = []
        
        let folderSubpath: String
        if subpath != nil {
            folderSubpath = "\(subpath!)/\(folder.name!)"
        } else {
            folderSubpath = folder.name!
        }
        
        let finished = {
            print("scan folder: \(folderSubpath) count: \(result.count)")
            complete(result)
        }
        print("scan folder: \(folderSubpath)")
        
        let mimeTypeFolder = "application/vnd.google-apps.folder"
        findFiles(in: folder) { (files, error) in
            if let error = error {
                print("scan folder: \(folderSubpath) error: \(error.localizedDescription)")
                finished()
                return
            }

            let filesOnly = files.filter( {$0.mimeType != mimeTypeFolder})
            
            let directoryUrl = url.appendingPathComponent(folderSubpath)
            for file in filesOnly {
                let url = directoryUrl.appendingPathComponent(file.name!)
                if !FileManager.default.fileExists(atPath: url.path ) {
                    result.append( (file: file, directory: directoryUrl) )
                }
            }
            
            var count = 0;
            let foldersOnly = files.filter( {$0.mimeType == mimeTypeFolder})
            if foldersOnly.count == 0 {
                finished()
            } else {
                let folderNames = foldersOnly.map( { $0.name! })
                print("scan folder: \(folderSubpath) subfolders: \(folderNames)")
                for file in foldersOnly  {
                    count += 1
                    
                    self.scanDownloadList(folder: file, directory: url, subpath: folderSubpath) { list in
                        result += list
                        count -= 1
                        if count == 0 {
                            finished()
                        }
                    }
                }
            }
        }
    }
    
    func download(folder: GTLRDrive_File, directory url: URL, unzip: Bool, complete: @escaping ()->Void ) {

        scanDownloadList(folder: folder, directory: url, subpath: nil) { list in

        
            var downloadList = list
            if unzip  {
                let zipsToSkip = list.filter({
                    (($0.file.name)! as NSString).pathExtension == "zip" && FileManager.default.fileExists(atPath: $0.directory.appendingPathComponent($0.file.name!).deletingPathExtension().path)
                })
                
                for zip in zipsToSkip {
                    let index = downloadList.index(where: { $0.file == zip.file && $0.directory == zip.directory})!
                    downloadList.remove(at: index)
                }
            }
            print("\(folder.name!) number of files to download: \(downloadList.count)")
            let rldownloader = RateLimitedDownloader(downloadList, service: self.service, unzip: unzip)
            self.stateHolder.downloaders.append(rldownloader)
            rldownloader.start() {
                let index = self.stateHolder.downloaders.index(where: { $0 === rldownloader })!
                self.stateHolder.downloaders.remove(at: index)
                complete()
            }
        }
    }

}
