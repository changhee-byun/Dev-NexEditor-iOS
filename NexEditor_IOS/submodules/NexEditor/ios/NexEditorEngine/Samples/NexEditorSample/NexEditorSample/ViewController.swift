//
//  ViewController.swift
//  NexEditorSample
//
//  Created by Simon Kim on 5/16/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import UIKit
import NexEditorEngine
class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        _ = NexEditor.sharedInstance()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

