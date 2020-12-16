//
//  ViewController.swift
//  LinkFramework
//
//  Created by MJ.KONG-MAC on 17/04/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import UIKit
import NexEditorFramework

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        _ = NXEEngine.instance()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

