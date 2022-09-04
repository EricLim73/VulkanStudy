#ifndef APP_H
#define APP_H

#include "../Render/window.h"
#include "../Render/pipeline.h"
#include "../Render/device.h"
#include "../Render/renderer.h"
//  #include "../Render/model.h"
#include "../GameAsset/gameObject.h"    //  -> contains model.h


#include <memory>
#include <vector>

namespace VULKVULK{
class App{
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 960;

        App();
       ~App();
       //   BC we have vulkan Object within our class, we delete our copt constructor
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void run();
    private:
        //void loadModels();
        void loadGameObjects();


        Window myWindow{WIDTH, HEIGHT, "Hello Vulkan"};
        Device myDevice{myWindow}; 
        
        Renderer myRenderer{myWindow, myDevice}; 

        std::vector<GameObject> myGameObjects;
};

}   //  namespace VULKVULK


#endif