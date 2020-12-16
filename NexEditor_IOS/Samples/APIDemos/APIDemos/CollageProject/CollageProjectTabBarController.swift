/*
 * File Name   : CollageProjectTabBarController.swift
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

class CollageProjectTabBarController: NSObject, UITabBarDelegate {
    typealias DataSourceLoader = ((_ selectedIndex: inout Int?)->[ThumbnailLoadable])

    let tabBar: UITabBar
    let collectionView: UICollectionView
    let tabItems: [Tab: UITabBarItem]
    let cellReuseId: String
    
    enum Tab: Int {
        case clips = 0
        case collages
        case items
    }
    public weak var viewController: UIViewController?
    public var onItemSelected: ((_ controller: CollageProjectTabBarController, _ tab: Tab, _ item: Any?)->Void)?
    public var onTabSelected: ((_ controller: CollageProjectTabBarController, _ tab: Tab)->Void)?
    public var onTabDataSourceLoaderRequired: ((_ controller: CollageProjectTabBarController, _ tab: Tab)->DataSourceLoader)?
    public var onActivityIndicatorRun: ((_ run: @escaping ()->Void )->Void)?

    private var collectionViewDelegate: UICollectionViewDelegate?
    fileprivate var collectionViewDataSource: ThumbnailSequenceDataSource? {
        didSet (old) {
            collectionView.dataSource = collectionViewDataSource
            let delegate = ThumbnailSequenceCollectionViewDelegate(dataSource: collectionViewDataSource!)
            delegate.onSelection = { [weak self] (ds, item, index) in
                guard let me = self else { return }
                guard let selectedTabBarItem = me.tabBar.selectedItem else { return }
                guard let tab = Tab(rawValue: me.tabBar.items!.index(of: selectedTabBarItem)!) else { return }
                me.onItemSelected?(me, tab, item)
            }
            
            collectionView.delegate = delegate
            collectionViewDelegate = delegate
        }
    }
    
    init(_ tabBar: UITabBar, _ collectionView: UICollectionView, cellReuseId: String, tabBarItems: [Tab: UITabBarItem]) {
        self.tabBar = tabBar
        self.collectionView = collectionView
        self.tabItems = tabBarItems
        self.cellReuseId = cellReuseId
        super.init()
        tabBar.delegate = self
    }
    
    public func selectTab(_ tab: Tab) {
        if let tabBarItem = tabItems[tab] {
            tabBar.selectedItem = tabBarItem
            tabBar(tabBar, didSelect: tabBarItem)
        }
    }
    
    public var currentTab: Tab? {
        guard let tabBarItem = tabBar.selectedItem else { return nil }
        return tabForTabBarItem(tabBarItem)
    }
    
    func tabForTabBarItem(_ tabBarItem: UITabBarItem) -> Tab {
        let tab: Tab
        if tabBarItem == tabItems[.collages] {
            tab = .collages
        } else if tabBarItem == tabItems[.items] {
            tab = .items
        } else {
            tab = .clips
        }
        return tab
    }
    
    public func reloadItems(_ tab: Tab? = nil ) {
        guard let currentTab = tab ?? self.currentTab else { return }
        
        if let requestLoader = self.onTabDataSourceLoaderRequired {
            let loadItems = requestLoader(self, currentTab)
            
            let run = onActivityIndicatorRun ?? { (block) in block() }
            run {
                var index: Int? = nil
                let items = loadItems(&index)
                
                self.collectionViewDataSource = ThumbnailSequenceDataSource(items: items,
                                                                            cellReuseId: self.cellReuseId)
                self.collectionView.reloadData()
                if let index = index {
                    let indexPath = IndexPath(row: index, section: 0)
                    self.collectionView.selectItem(at: indexPath, animated: false, scrollPosition: .centeredHorizontally)
                }
            }
        }
    }
    
    // MARK: - UITabBarDelegate
    
    func tabBar(_ tabBar: UITabBar, didSelect item: UITabBarItem) {
        let tab = tabForTabBarItem(item)
        self.onTabSelected?(self, tab)
        reloadItems()
    }
}
