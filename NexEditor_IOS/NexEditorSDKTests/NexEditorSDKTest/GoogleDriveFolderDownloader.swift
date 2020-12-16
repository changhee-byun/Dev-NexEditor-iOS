//
//  GoogleDriveFolderDownloader.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 6/1/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import Foundation
import GoogleAPIClientForREST
import GoogleSignIn

class GoogleDriveFolderDownloader: NSObject, GIDSignInDelegate {
    typealias SignInStatusChanged = (_ user: GIDGoogleUser?, _ error: Error?)->Void
    typealias DownloadStatusChanged = (_ request: Request, _ state: DownloadState, _ folder: GTLRDrive_File?, _ directoryUrl: URL? )->Void
    
    enum DownloadState {
        case started
        case ended
    }
    
    struct Request {
        let folderName: String
        let signatureName: String
        let directoryUrl: URL
        let unzip: Bool
        
        init( _ folderName: String, _ signatureName: String, _ directoryUrl: URL, unzip: Bool) {
            self.folderName = folderName
            self.signatureName = signatureName
            self.directoryUrl = directoryUrl
            self.unzip = unzip
        }
    }
    
    private let scopes = [kGTLRAuthScopeDriveReadonly]
    private let service = GTLRDriveService()
    private lazy var folderScanner: GoogleDriveFolderScanner = {
        return GoogleDriveFolderScanner(service: self.service)
    }()
    private var requests: [Request] = []
    
    var signInStatusChange: SignInStatusChanged? = nil
    var downloadStatusChanged: DownloadStatusChanged? = nil
    
    init(signInStatusChanged: @escaping SignInStatusChanged = { _, _ in }) {
        super.init()
        
        self.signInStatusChange = signInStatusChanged
        
        // Configure Google Sign-in.
        GIDSignIn.sharedInstance().delegate = self
        GIDSignIn.sharedInstance().scopes = scopes
        GIDSignIn.sharedInstance().signInSilently()
    }
    
    func sign(_ signIn: GIDSignIn!, didSignInFor user: GIDGoogleUser!,
              withError error: Error!) {
        
        var startDownload = false
        if error != nil {
            self.service.authorizer = nil
        } else {
            self.service.authorizer = user.authentication.fetcherAuthorizer()
            startDownload = true
        }
        signInStatusChange?(user, error)
        
        if startDownload {
            let reqs = requests
            requests = []
            for request in reqs {
                self.startDownload(request: request)
            }
        }
    }
    
    private func startDownload(request: Request) {
        downloadStatusChanged?(request, .started, nil, nil)
        folderScanner.findFolder(request.folderName, containsFile: request.signatureName) { (folder) in
            if let folder = folder {
                self.folderScanner.download(folder: folder, directory: request.directoryUrl, unzip: request.unzip) {
                    self.downloadStatusChanged?(request, .ended, folder, request.directoryUrl)
                }
            } else {
                self.downloadStatusChanged?(request, .ended, nil, nil)
            }
        }
    }
    
    var authorized : Bool {
        return self.service.authorizer != nil
    }
    
    func downloadFolder( name folderName: String,
                         signatureFileName: String,
                         directoryUrl: URL? = nil,
                         unzip: Bool = true) {
        
        let request = Request(folderName, signatureFileName, directoryUrl ?? FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0], unzip: unzip)
        if authorized {
            startDownload(request: request)
        } else {
            requests.append(request)
        }
    }
}
