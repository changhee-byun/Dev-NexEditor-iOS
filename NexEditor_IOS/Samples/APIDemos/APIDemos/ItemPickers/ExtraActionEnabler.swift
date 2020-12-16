/*
 * File Name   : ExtraActionEnabler.swift
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

protocol TitledAction {
    var title: String { get }
    var action: (_ viewController: UIViewController? ) -> Void { get }
}

struct ExtraAction: TitledAction {
    var title: String
    var action: (_ viewController: UIViewController? ) -> Void
}

/// Instantiate with view controller, call setupNavigationItem() to add a bar button item, and set extraActions. When user selects one of extra actions, it's assocaiated closure, labeld as action, will be invoked.
@objc class ExtraActionEnabler: NSObject {
    
    private weak var viewController: UIViewController?
    public var extraActions: [TitledAction] = []
    
    @objc func showActions(sender:Any?) {
        let titles = self.extraActions.map( {$0.title} )
        viewController?.alertSimpleActionSheet("Actions", message: "", actionTitles: titles, dismissTitle: "Cancel", complete: { [weak self] index in
            if index != NSNotFound {
                self?.act(index)
            }
        })
    }
    
    private func act(_ index: Int) {
        self.extraActions[index].action(self.viewController)
    }
    
    /// Call this method once in the view controller to add a bar button item for extra actions
    public func setupNavigationItem(_ navigationItem: UINavigationItem) {
        if self.extraActions.count == 0 {
            return
        }
        
        let barButton = UIBarButtonItem(barButtonSystemItem: .action, target: self, action: #selector(showActions(sender:)))
        
        var buttons = navigationItem.rightBarButtonItems
        buttons?.append(barButton)
        navigationItem.rightBarButtonItems = buttons
    }
    
    public init(viewController: UIViewController) {
        self.viewController = viewController
    }

}
