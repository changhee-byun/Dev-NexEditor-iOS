//
//  MainListView.swift
//  EditorDemo
//
//  Created by CHANGHEE2 on 2020/12/22.
//

import SwiftUI

struct MainListView: View {
    @State var demoFeatures: [DemoFeature] = [
        DemoFeature(title: "Beat Template")
    ]
    
    var body: some View {
        NavigationView {
            List{
                ForEach(self.demoFeatures, id: \.self) { feature in
                    DemoFeatureListCell(demoFeature: feature)
                }
            }
            .listStyle(PlainListStyle())
            .navigationBarTitle(Text("Editor Demo"), displayMode: .inline)
        }
    }
}

struct DemoFeatureListCell: View {
    var demoFeature: DemoFeature
    var destinationView: BeatTemplateMainView = BeatTemplateMainView()
    
    var body: some View {
        NavigationLink(destination: self.destinationView) {
            Text(self.demoFeature.title)
        }
    }
}

struct MainListView_Previews: PreviewProvider {
    static var previews: some View {
        MainListView()
    }
}
