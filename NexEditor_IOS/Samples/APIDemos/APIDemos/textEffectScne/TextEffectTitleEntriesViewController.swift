/**
 * File Name   : TextEffectTitleEntriesViewController.swift
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

typealias TextEffectTitleEntriesComplete = (_ viewController: TextEffectTitleEntriesViewController, _ entered: Bool, _ texts: [String]?) -> (Void)

class TextEffectTitleEntriesViewController: UITableViewController  {
    
    var onComplete: TextEffectTitleEntriesComplete?
    
    var titles: [String] = ["Intro Title", "Intro SubTitle", "Outro Title", "Outro SubTitle"]
    var texts: [String] = ["", "", "", ""]
    
    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 4
    }
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "TextEffectCell", for: indexPath) as! CommonTypeETableViewCell

        cell.titleLabel.text = titles[indexPath.row]
        cell.titleField.text = texts[indexPath.row]
        
        cell.titleField.addTarget(self, action:#selector(self.textFieldDidEnd(_:)), for:UIControl.Event.editingChanged)
        cell.titleField.tag = indexPath.row
        
        return cell
    }
    
    @objc func textFieldDidEnd(_ textField: UITextField) {
        texts[textField.tag] = textField.text!
    }
    
    @IBAction func done(_ sender: Any) {
        if let onComplete = onComplete {
            self.view.endEditing(true)
            onComplete(self, true, texts)
            self.onComplete = nil
        }
    }
    
    @IBAction func cancel(_ sender: Any) {
        if let onComplete = onComplete {
            onComplete(self, false, nil)
            self.onComplete = nil
        }
    }
    
}
