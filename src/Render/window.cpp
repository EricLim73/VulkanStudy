#include <stdexcept>
#include "window.h" 

namespace VULKVULK{
Window::Window(int width, int height, std::string name):
    width(width), height(height), windowName(name){
        InitWindow();
    };

Window::~Window(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::InitWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   //  Bloc OPENGL Creation
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); //  Block resize functionability

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    //  set pointer to target window -> this case its "this" the WindowClass object that called this function
    glfwSetWindowUserPointer(window, this); 
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface){
    if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS){
        throw std::runtime_error("Failed to create window surface");
    }
}

void Window::framebufferResizeCallback(GLFWwindow* _window, int width, int height){
    auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(_window));
    window->framebufferResized = true;
    window->width = width;
    window->height = height;
}
//  static_cast => consider as function return(== there exists a temp variable that stores the casted value and returns it, next line will destroy that temp)
//  reinterpret_cast => consider its telling compiler to consider given value(data or pointer) as said type( mostly used for "ptr-ptr" || "ptr-variable" relationship)

}   //  namespace VULKVULK