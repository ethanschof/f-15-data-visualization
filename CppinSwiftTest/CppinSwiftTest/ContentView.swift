//
//  ContentView.swift
//  CppinSwiftTest
//
//  Created by Natali Schofield on 12/6/22.
//

import SwiftUI

CPP_Wrapper().get_my_cpp_integer()

var myName: String
myName = "Ethan"

CPP_Wrapper().hello_(from_cpp_wrapped: myName)


struct ContentView: View {
    var body: some View {
        VStack {
            Image(systemName: "globe")
                .imageScale(.large)
                .foregroundColor(.accentColor)
            Text("Hello, world!")
            

        }
        .padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
