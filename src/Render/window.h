#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace VULKVULK{

class Window{
    public:
        Window(int width, int height, std::string name);
        ~Window();
        //  Delete copy constructor
        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;
        
        //  Getter Functions
        VkExtent2D getExtent() {return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};}
        bool shouldClose() {return glfwWindowShouldClose(window);}
        bool wasFramebufferResized() const {return framebufferResized;}
        GLFWwindow *GetWindow() const {return window;}

        void resetFramebufferResized() {framebufferResized = false;}
        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        void InitWindow();
        static void framebufferResizeCallback(GLFWwindow* _window, int width, int height);

        int width;
        int height;
        bool framebufferResized = false;

        std::string windowName;
        GLFWwindow* window;
};


}   //  namespace VULKVULK


#endif