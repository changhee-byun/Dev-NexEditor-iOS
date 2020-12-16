//
//  GoogleDriveConsoleViewController.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 5/29/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import UIKit
import GoogleSignIn

class TestResourceSyncViewController: UIViewController, GIDSignInUIDelegate {
    @IBOutlet var topbar: UIView!
    @IBOutlet var textView: UITextView!
    @IBOutlet var buttonReset: UIButton!
    
    @IBOutlet var activityIndicator: UIActivityIndicatorView!
    
    let signInButton = GIDSignInButton()
    var downloader: GoogleDriveFolderDownloader!
    
    lazy var documentsUrl: URL = {
        return FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    }()
    
    override func viewDidLoad()
    {
        super.viewDidLoad()

        // Add the sign-in button.
        topbar.addSubview(signInButton)
        
        // Add a UITextView to display output.
        textView.isEditable = false
        textView.contentInset = UIEdgeInsets(top: 20, left: 0, bottom: 20, right: 0)
        textView.text = "Sign in to synchronize clip sources from Google Drive"
        
        title = "Clip source download"
        
        let appDelegate = UIApplication.shared.delegate as! AppDelegate
        if !appDelegate.isGoogleConfigured {
            appDelegate.configureGoogle()
        }
        
        GIDSignIn.sharedInstance().uiDelegate = self
        downloader = GoogleDriveFolderDownloader()
        { (user, error) in
            if let error = error {
                self.showAlert(title: "Authentication Error", message: error.localizedDescription)
            } else {
                self.signInButton.isHidden = true
            }
        }
        
        downloadFolders(downloader: downloader)

    }
    
    func downloadFolders(downloader: GoogleDriveFolderDownloader) {
        buttonReset.isEnabled = false
        var downloadingCount = 2
        var downloadStatusLogs: [String] = []
        
        self.activityIndicator.isHidden = true
        downloader.downloadStatusChanged =  { request, state, folder, url in
            switch(state) {
            case .started:
                if self.activityIndicator.isHidden {
                    self.activityIndicator.isHidden = false
                    self.activityIndicator.startAnimating()
                }
                downloadStatusLogs.append("Downloading \(request.folderName)...")
                
                break
            case .ended:
                downloadingCount -= 1
                if downloadingCount == 0 {
                    self.buttonReset.isEnabled = true
                    self.activityIndicator.stopAnimating()
                    self.activityIndicator.isHidden = true
                }
                if folder != nil {
                    downloadStatusLogs.append("Downloaded \(request.folderName)...")
                } else {
                    let message = "To download files from Google Drive, create \(request.folderName)/\(request.signatureName) file and put files under \(request.folderName)/ folder"
                    self.showAlert(title: "Folder not found", message: message)
                    downloadStatusLogs.append("\(message), and retry")
                }
                
                break
            }
            self.textView.text = downloadStatusLogs.joined(separator: "\n")
        }
        
        downloader.downloadFolder(name: "ClipSources", signatureFileName: ".nexeditorsdktest", directoryUrl: documentsUrl)
        downloader.downloadFolder(name: "AssetBundles", signatureFileName: ".nexeditorsdktest", directoryUrl: documentsUrl)
    }
    
    // Helper for showing an alert
    func showAlert(title : String, message: String) {
        let alert = UIAlertController(
            title: title,
            message: message,
            preferredStyle: UIAlertControllerStyle.alert
        )
        let ok = UIAlertAction(
            title: "OK",
            style: UIAlertActionStyle.default,
            handler: nil
        )
        alert.addAction(ok)
        present(alert, animated: true, completion: nil)
    }
    
    // TODO: Add Sign-out button for switching account, sign-in debugging and so on.
    // @IBAction func signout(_ sender: UIButton) {
    //     GIDSignIn.sharedInstance().signOut()
    // }
    
    @IBAction func resetFiles(_ sender: UIButton) {
        
        self.activityIndicator.isHidden = false
        self.activityIndicator.startAnimating()
        sender.isEnabled = false
        
        DispatchQueue.global().async {
            let alertMessage: String
            do {
                try FileManager.default.removeItem(at: self.documentsUrl.appendingPathComponent("ClipSources"))
                try FileManager.default.removeItem(at: self.documentsUrl.appendingPathComponent("AssetBundles"))
                
                alertMessage = "Removed all files. Download again."
            } catch let e {
                alertMessage = e.localizedDescription
            }
            
            DispatchQueue.main.async {
                self.showAlert(title: "Reset files", message: alertMessage)
                
                self.activityIndicator.stopAnimating()
                self.activityIndicator.isHidden = true
                sender.isEnabled = true
            }
        }
        
    }
}
