#define GLFW_INCLUDE_VULKAN//GLFW将包含自己的定义并自动加载Vulkan标头
#include <GLFW/glfw3.h>
//std
#include <iostream>//这两个头文件用于报告和传播error
#include <stdexcept>
#include <cstdlib>//提供EXIT_SUCCESS和EXIT_FAILURE宏
#include<vector>
#include<cstring>
#include<optional>
#include<set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include<fstream>
/*
vkCreateXXX 函数创建
vkAllocateXXX 对象分配
hint 提示、暗示
actual 实际的
resizable 可调整大小的
terminate 终止
enumerate 列举、枚举
feature 特色特征
capability 能力、容量
scissor 剪刀
uniform 统一、制服

validation 验证层
allocator 分配器
messenger 信使。这里所有的都表示验证消息调试信使，同debugMessenger
layer 层、图层。这里所有的都表示验证层，同validationLayer或validation
extension 扩展
physicalDevice 物理设备-显卡
device 设备。这里所有的都表示逻辑设备，同logicalDevice
queue 队列
queueFamily 队列族
surface 表面。这里所有的都表示窗口表面windowSurface，surface是一个window渲染目标的抽象
swapChain 交换链
imageView 图像视图。
pipeline 管线。这里所有的都表示图形管线，同graphicPipeline
scissorRectangle 裁剪矩形
rasterizer 光栅化器
renderPass 渲染通道
attachment 附件
drawFrame 绘制
commandPool 命令池(管理用于存储缓冲区的内存，并从中分配命令缓冲区)
semaphore 信号量
fence 围栏
*/
const uint32_t WIDTH = 800;//常量 窗口的宽度
const uint32_t HEIGHT = 600;//常量 窗口的高度
const int MAX_FRAMES_IN_FLIGHT = 2;//定义同时处理的帧数。
//启用vkSDK提供的标准验证层。验证层中可以：根据规范检查参数值以检测误用；跟踪对象的创建和销毁以查找资源泄漏；通过跟踪调用源自的线程来检查线程安全；将每个调用及其参数记录到标准输出；跟踪 Vulkan 调用以进行分析和重放。
const std::vector<const char*> validationLayers = { //存储要使用的全局验证层
    "VK_LAYER_KHRONOS_validation"//验证层需要通过指定名称来启用，所有标准验证都捆绑在SDK中的一个层里，称为VK_LAYER_KHRONOS_validation
};
//交换链扩展名
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME//vk提供的宏，定义为VK_KHR_SWAPCHAIN,用这个宏的好处是编译器会发现拼写错误
};
//配置变量-是否启用validationLayer
#ifdef NDEBUG//非调试
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
//接收信使创建信息VkDebugUtilsMessengerCreateInfoEXT以创建vkCreateDebugUtilsMessengerEXT信使对象的函数。
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");//vkCreateDebugUtilsMessengerEXT创建信使对象函数是一个扩展(不会自动加载)，必须使用vkGetInstanceProcAddr查找它的地址，这里应该使用代理函数在后台解决这个问题
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
//销毁messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//代理函数，查找销毁信使的函数 的地址
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
//用于支持图形命令的队列族索引
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t>presentFamily;//找到支持呈现到表面的队列族
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();//检测不为null
    }
};
//用于支持交换链的属性
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;//基本表面功能(交换链中图像的最小/最大数量,图像的最小/最大宽度和高度)
    std::vector<VkSurfaceFormatKHR>formats;//表面格式(像素格式,色彩空间)
    std::vector<VkPresentModeKHR>presentModes;//可用的演示模式
};
//程序本身被包装到一个类中
class HelloTriangleApplication {
public:
    void run() {//主执行程序
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
private:
    GLFWwindow* window;//实际的窗口

    VkInstance instance;//vk实例的句柄
    VkDebugUtilsMessengerEXT debugMessenger;//调试信使句柄。验证消息回调是调试信使的一部分
    VkSurfaceKHR surface;//vk与窗口系统的连接桥梁//他在glfwGetRequiredInstanceExtensions返回列表中已经启用过
    
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//选择的显卡的句柄。当vk实例被销毁时，这个句柄会被隐式销毁，因此不需要在cleanUp中执行任何操作
    VkDevice device;//逻辑设备句柄
   
    VkQueue graphicsQueue;//图形队列的句柄(队列与逻辑设备一起自动创建 但是需要与他们交互的句柄)设备被销毁时会被隐式销毁
    VkQueue presentQueue;//演示队列的句柄
    
    VkSwapchainKHR swapChain;//存储VkSwapchainKHR对象
    std::vector<VkImage>swapChainImages;//存储交换链中vkImages的句柄
    VkFormat swapChainImageFormat;//存储为交换链选择的格式
    VkExtent2D swapChainExtent;//存储为交换链选择的范围
    std::vector<VkImageView>swapChainImageViews;//存储图像视图,图像视图实际上是对图像的视图，描述了如何访问图像以及访问图像的哪个部分
    std::vector<VkFramebuffer>swapChainFramebuffers;//帧缓冲区
    
    VkRenderPass renderPass;//渲染通道
    VkPipelineLayout pipelineLayout;//管线布局
    VkPipeline graphicsPipeline;//图形管线

    VkCommandPool commandPool;//命令池。命令池管理用于存储缓冲区的内存，并从中分配命令缓冲区
    std::vector<VkCommandBuffer>commandBuffers;//命令缓冲区。命令 缓冲区将在其命令池被销毁时自动施放，因此不需要显式清理
    
    //存储这些信号量对象和栅栏对象
    std::vector<VkSemaphore>imageAvailableSemaphores;
    std::vector<VkSemaphore>renderFinishedSemaphores;
    std::vector<VkFence>inFlightFences;

    uint32_t currentFrame = 0;//帧索引。要在每一帧都使用正确的对象需要跟踪当前帧

    //初始化glfw，创建窗口
    void initWindow() {
        glfwInit();//初始化glfw库
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//告诉glfw不创建opengl上下文
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//resizable可调整大小的
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//创建窗口(高度,宽度,标题,允许指定打开窗口的屏幕,仅与OpenGL有关的参数)
    }
    //initVk中调用所有的 启动vk对象的函数
    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurace();//窗口表面在instance创建后立即创建，它会影响物理设备的旋转
        pickPhysicalDevice();
        //选择好物理设备后，需要设置一个逻辑设备与其交互
        createLogicalDevice();
        createSwapChain();
        createImageViews();//
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
    }
    //mainLoop
    void mainLoop() {
        //该循环会一直迭代直到窗口关闭
        while (!glfwWindowShouldClose(window)) {//检查关闭事件
            glfwPollEvents();
            //std::cout << "全局验证层数量：" << validationLayers.size() << std::endl;
            std::cout << "currentFrame:" << currentFrame << std::endl;
            drawFrame();//绘图
        }
        vkDeviceWaitIdle(device);
    }
    //当窗口关闭并mainLoop返回，在cleanup中释放资源
    void cleanup() {
        //当程序结束，所有命令完成不再需要同步时清理信号量和围栏
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        
        vkDestroyCommandPool(device, commandPool, nullptr);
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        for (auto imageView : swapChainImageViews) {//图像视图是明确创建的，需要添加一个类似的循环销毁
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);//销毁逻辑设备
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);//销毁messenger
        }
        vkDestroySurfaceKHR(instance, surface, nullptr);
        //在程序退出前销毁instance。所有其他vk资源都应该在instance被销魂前清理
        vkDestroyInstance(instance,nullptr);//vk中的分配和释放函数都有一个Allocator回调，可以通过nullptr忽略
        glfwDestroyWindow(window);
        glfwTerminate();//glfw终止
    }
    //创建instance，初始化vk库，链接app和vk库
    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {//app在调试模式下，所有验证层都可用
            throw std::runtime_error("validation layers requested,but not available!");
        }

        //在结构体中填写关于app的信息以创建实例。vk中许多信息是通过结构体而不是函数参数来传递的
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;//vk中许多结构体都要求在sType中显式指定类型
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);//version版本
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};//告诉vk驱动使用哪些extension和validation
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        //app所需的extensions(包括validation、swapChain等)
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};//该debugCreateInfo变量放在if语句之外是为了在vkCreateInstance调用前它不会被破坏。创建一个额外的调试信使，它将在此期间自动使用vkCreateInstance之后用vkDestroyInstance清理
        //结构体的最后两个成员确定要启用的全局验证层
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();//包含启用的验证层名称
            //虽然app中已经加了验证消息调试回调，但是由于调用的关系 vkCreateInstance和vkDestroyInstance没有被调试。扩展文档中有专门为这两个函数创建单独调试信使的方法，只要在VkInstanceCreateInfo的pNext扩展字段中传递一个指向VkDebugUtilsMessengerCreateInfoEXT调试信使创建信息的指针
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        //创建实例。vk中创建对象的函数一般都是(指向创建信息的结构体指针，指向自定义分配器回调的指针，指向存储新对象句柄的指针)
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {/*像这样的，Vulkan中的对象创建函数参数遵循的一般模式是：指向带有创建信息的结构体的指针；指向自定义分配器回调的指针，本项目都是nullptr；指向储存新对象句柄的变量的指针。*///几乎所有的Vulkan函数都会返回一个类型值VkResult，要么是VK_SUCCESS要么是错误代码
            throw std::runtime_error("failed to create instance!");
        }
    }
    //将信使创建的入口提取到单独的函数中
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;//指定希望调用回调的所有严重性类型
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;//过滤通知回调的消息类型
        createInfo.pfnUserCallback = debugCallback;//指定指向回调函数的指针
    }
    //设置validaiton消息调试回调messenger
    void setupDebugMessenger() {
        if (!enableValidationLayers)return;//不在调试模式下就没必要使用validation
        VkDebugUtilsMessengerCreateInfoEXT createInfo;//在结构体中填写有关信使及其调试信息
        populateDebugMessengerCreateInfo(createInfo);//填充messenger创建信息
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    //创建窗口表面
    void createSurace() {
        //vkCreateWin32SurfaceKHR 创建表面(实例的参数，表面创建袭击，自定义分配器，要产出的表面具备) 这是一个WS扩展函数非常常用以至于vk加载器包含它，不需要显示加载
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create windwo surface!");
        }
    }
    //选择物理设备
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);//列举显卡
        if (deviceCount == 0)throw std::runtime_error("failed to find GPUs with Vulkan support!");//如果没有显卡支持vulkan就没有必要继续走了
        std::vector<VkPhysicalDevice>devices(deviceCount);//保存所有VkPhysicalDevice句柄d 数组
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());//列举显卡
        for (const auto& device : devices) {//评估每张显卡是否合适满足要求
            if (isDeviceSuitable(device)) {
                physicalDevice = device;//找到了满足要求的显卡
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE)throw std::runtime_error("failed to find a suitable GPU!");//遍历所有的显卡都不合适
    }
    //创建逻辑设备
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        //我们需要有多个VkDeviceQueueCreateInfo结构来从两个系列创建一个队列。方法是创建一组所需队列 所必需的 所有唯一队列族
        std::vector<VkDeviceQueueCreateInfo>queueCreateInfos;
        std::set<uint32_t>uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value()};

        //创建queueFamily的info
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};//结构体VkDeviceQueueCreateInfo描述了我们想要的单个队列族的队列数
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;//当前驱动只可为每个queueFamily创建少量queue，实际不需要多个queue因为可以多线程创建所有命令缓冲区，通过一个低开销调用在主线程一次提交
            
            float queuePriority = 1.0f;//使用float(0-1)为队列分配优先级 影响命令缓冲区执行的调度
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};//指定将要使用的设备功能集

        //创建逻辑设备
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();//指向队列创建信息的指针
        createInfo.pEnabledFeatures = &deviceFeatures;//指向设备功能结构体的指针
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());//指定特定于设备的扩展
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();//启用设备扩展

        if (enableValidationLayers) {//特定于设备的验证层
            createInfo.enabledLayerCount = static_cast<int32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }else {
            createInfo.enabledLayerCount = 0;
        }
        //实例化逻辑设备(与之交互的物理设备，创建信息(队列等)，分配器回调指针，指向逻辑设备的句柄的指针)
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        //检索每个队列族的队列句柄。参数是逻辑设备、队列族、队列索引和指向存储队列句柄的指针
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }
    //创建交换链//交换链的本质是一个等待呈现到屏幕上的图像队列。app获取该图像并绘制他，再将其返回队列中。队列的具体工作方式以及从队列中呈现图像的方式取决于交换链的设置。一般交换链的目的是使图像的呈现与屏幕的刷新率同步
    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport=querySwapChainSupport(physicalDevice);//查询物理设备的交换链支持情况
        //为交换链选择最佳设置
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        //设置交换链图像数量
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount+1;//交换链运行所需的最小图像数量//直接用最小值可能有必须等待驱动程序完成内部材质然后才能获取另一个要渲染的图像的情况，一次至少请求比最小值多的图像
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {//确保不超过最大图像数量，0是一个特殊值表示没有最大值
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        //交换链对象创建信息
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;//指定交换链应绑点到哪个表面
        //指定交换链图像的详细信息
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;//指定每个图像包含的层数。触发开发立体3Dapp，否则=1
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//位字段指定将交换链中的图像进行何种操作
        //指定如何处理 将跨多个队列使用的 交换链图像。我们将从图形队列中的交换链中绘制图形，如何将他们提交到演示队列中。
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamulyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//图像可以在多个队列族中使用，无需显式所有权转移。
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamulyIndices;
        }else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//图像一次由一个队列族拥有，并且在将其用于另一个队列族之前，必须明确转移所有权。此选项提供最佳性能。
            createInfo.queueFamilyIndexCount = 0;//optional
            createInfo.pQueueFamilyIndices = nullptr;//optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;//我们可以指定，如果支持某个变换，则应将其应用于交换链中的图像（功能上支持该变换），如顺时针旋转90度或水平翻转。要指定不需要任何转换，只需指定当前转换。
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//该字段指定是否使用alpha通道与窗口系统中的其他窗口混合。这里忽略
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;//该字段设置为true意味着不关心被遮挡的像素的颜色

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        //创建交换链(逻辑设备，创建信息，可选的分配器，执行存储句柄的指针)
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        //检索交换链图像
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        //将选择的交换链格式和范围存储在成员变量中
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }
    //为交换链中的每个图像创建一个基本的图像视图，方便用户可以将他们用作颜色目标
    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());//调整列表的大小以适应将要创建的所有图像视图
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};//创建图像视图的参数
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            //viewType和format指定如何解释图像数据
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;//允许将图像视为1D纹理、2D纹理、3D纹理和立方体贴图
            createInfo.format = swapChainImageFormat;
            //components允许调整周围的颜色通道，这里使用默认映射
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            //subresourceRange描述图像的用途以及应该访问图像的哪一部分。我们的图像将 用作没有任何mipmapping级别或多层的颜色目标
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            //如果开发立体3Dapp，那么将创建具有多个层的交换链。然后通过访问不同的层为每个表示左眼和右眼视图的图像创建多个图像视图
            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {//创建图像视图
                throw std::runtime_error("failed to create image views!");
            }
        }
    }
    //创建渲染通道。在创建pipeline之前，告诉vk渲染时将使用的帧缓冲区附件。指定将有多少颜色和深度缓冲区，每个缓冲区使用多少样本，以及在整个渲染操作中应该如何处理他们的内容
    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;//颜色附件的format应该与交换链图像的格式相匹配
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//暂时没有对多重采样做任何事情，所以使用1个样本
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//确定渲染前如何处理附件中的数据。VK_ATTACHMENT_LOAD_OP_LOAD-保留附件的现有内容；VK_ATTACHMENT_LOAD_OP_CLEAR-在开始时将值清除为常量；VK_ATTACHMENT_LOAD_OP_DONT_CARE-现有内容未定义，不在乎他们。在这里使用清除操作，在绘制新帧之前将缓冲区清除为黑色
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//soreOp有两种可能性：VK_ATTACHMENT_STORE_OP_STORE-渲染的内容将存储在内存中，稍后可以读取；VK_ATTACHMENT_STORE_OP_DONT_CARE-渲染操作后将未定义帧缓冲区的内容。这里进行存储操作
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//loadOp和storeOp应用于颜色和深度数据，stencilLoad/stencilStoreOp适用于模具数据，app不会对模板缓冲区做任何事情，因此加载和存储的结果是无关的。
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //常见的布局：VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL-用作颜色附件的图像；VK_IMAGE_LAYOUT_PRESENT_SRC_KHR-交换链中显示的图像；VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL-用作内存复制操作目标的图像。
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//指定在渲染过程之前图像的布局。使用VK_IMAGE_LAYOUT_UNDEFINED就是我们不关心图像以前的布局，但是图像的不能不能保证被保存，这不重要因为我们无论如何都要清除它。
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//指定在渲染过程完成时自动过渡到的布局。我们希望图像在渲染后可以使用交换链进行显示，这就是为什么使用VK_IMAGE_LAYOUT_PRESENT_SRC_KHR作为最终布局

        /*单个renderPass可以包含多个subpass。subpass是后续渲染操作，它依赖于先前通道中帧缓冲区的内容，例如后处理。如果将这些渲染操作分组到一个渲染过程中，那么vk能够重新排序操作并节省内存带宽以获得更好的性能。这里检查使用单个subpass*/
        VkAttachmentReference colorAttachmentRef{};//每个subpass都引用一个或多个前面的结构体描述的附件
        colorAttachmentRef.attachment = 0;//该参数通过附件描述数值中的所有指定要引用的附件。数组由一个VkAttachmentDescription组成，所以他的索引是0
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//指定附件在使用此引用的subpass期间的布局。当subpass启动时vk会自动将附件转换到此布局。VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL顾名思义，我们打算将附件用作颜色缓冲区，布局将为我们提供最佳性能。

        VkSubpassDescription subpass{};//描述subpass
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        //vk将来也可能支持计算subpass，因此必须说明这是一个图形subpass。指定对颜色附件的引用：
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;//该数组中附件的所有是直接从fragShader中layout(location=0)out vec4 outColor指令引用的
        //subpass可以引用以下其他类型的附件：pInputAttachments-从着色器中读取的附件；pResolveAttachments-用于多重采样颜色附件的附件；pDepthStencilAttachment-深度和模板数据的附件；pPreserveAttachments-此子通道未使用但必须保留其数据的附件。

        //子通道依赖项。渲染管线中的subpass会自动处理图像布局变换。这些转换由subpass依赖项控制，它指定subpass之间的内存和执行依赖关系，现在只有一个subpass，但是这个subpass之前和之后的操作也算隐式"subpass"
        //在renderPass开始和renderPass结束时，有两个内置依赖项负责过度，但前者不会在正确的时间发生。它假设转换发生在管线的开始，但是我们还没有获得该点的图像！有两种方法解决。我们可以将iamgeAvailableSemaphore的waitStages更改为VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT，以确保渲染过程在图像可用之前不会开始，或者我们可以使渲染过程等待VK_PPIELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT阶段。这里使用第二个选项，因为这是个很好的查看subpass依赖项及其工作方式的借口
        VkSubpassDependency dependency{};
        //前两个字段指定依赖和依赖subpass的索引。VK_SUBPASS_EXTERNAL是指渲染管线之前或之后的隐式subpass，取决于它是否在srcSubpass或dstSubpass中。索引0指的是subpass，这是第一个也是唯一一个。dstSubpass必须始终高于srcSubpass以防止依存关系图中出现循环(除非其中一个subpass是VK_SUBPASS_EXTERNAL)
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        //接下来的两个字段指定要等待的操作以及这些操作发生的阶段。我们需要等待交换链完成对图像的读取然后才能访问它。这可以通过等待颜色附件输出级本身来完成
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        //通过VkRenderPassCreateInfo使用附件和subpass数组填充结构来创建renderPass对象。VkAttachmentReference对象使用此数组的索引引用附件
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("faided to create render pass!");
        }
    }
    /*图形管线是一系列操作，将网格的定点和纹理一直带到渲染目标中的像素:
    Vertex/Index Buffer->InputAssembler输入汇编程序->VertexShader->Tessellation细分->GeometryShader->Rasterization光栅化->FragmentShader->ColorBlending->FrameBuffer
    输入汇编器 从指定缓冲区收集原始顶点数据，也可以用索引缓冲区重复某些元素不用复制顶点数据本身。
    顶点着色器 每个顶点运行，应用转换将顶点位置从模型空间转换到屏幕空间。将每个顶点的数据传递到管线中。
    细分着色器 根据某些规则细分几何体以提高网格质量。通常用于使砖墙和楼梯等表面在附近时看起来不那么平坦。
    几何着色器 在每个图元(三角形、线、点)上运行，并且可以丢弃它或输出比输入更多的图元。这类似于曲面细分着色器，但更灵活。但是，它在当今的应用程序中使用得并不多，因为除了Intel的集成GPU之外，大多数显卡的性能都不是那么好。
    光栅化阶段 将图元离散为片段。这是它们在帧缓存区中填充的像素元素。任何落在屏幕外的片段都会被丢弃，顶点着色器输出的属性会被插值到片段中。通常其他原始片段后面的片段也会因为深度测试而在这里被丢弃。
    片元着色器 为每个幸存的片段调用，并确定将片段写入哪些帧缓存区以及使用哪些颜色和深度值。它可以使用来自顶点着色器的插值数据来执行此操作，其中可以包括纹理坐标和光照法线等内容。
    颜色混合阶段 应用操作来混合映射到帧缓存区中相同像素的不同片段。片段可以简单地相互覆盖、叠加或基于透明度混合。
    输入汇编、光栅化和颜色混合阶段是固定功能阶段。这些阶段允许您使用参数调整它们的操作，但它们的工作方式是预定义的。
    其他阶段是programable可编程的，可以将自己的代码上传到显卡以准确应用自定义操作。例如，这允许您使用片段着色器来实现从纹理和照明到光线追踪器的任何内容。这些程序同时在许多GPU内核上运行，以并行处理许多对象，例如顶点和片元
    如果您之前使用过OpenGL和Direct3D等较旧的API，那么您将习惯于使用glBlendFunc和OMSetBlendState等调用随意更改任何管线设置。vk中的图形管线几乎是完全不可变的，因此如果要更改着色器、绑定不同的帧缓存区或更改混合函数，必须从头开始重新创建管线。缺点是您必须创建许多管线，这些管线表示要在渲染操作中使用的所有不同状态混合。然而，因为您将在管线中执行的所有操作都是预先知道的，所有驱动程序可以更好的进行优化。
    一些可编程阶段是可选的。例如如果只是绘制简单的几何图形，则可以禁用曲面细分和几何图形阶段。如果您只对深度值感兴趣，则可以禁用片段着色器，这对于生成阴影贴图很有用。*/
    /*与早期的API不同，vk中的着色器代码必须以字节码格式指定，而不是像glsl和hlsl这样的可读性语法。这种字节码格式称为SPIR-V，旨在与vk和opgl以前使用。它是一种可用于编写图形和计算着色器的格式
    使用字节码格式的优势在于GPU供应商编写的讲着色器代码转换为本机代码的编译器的复杂性要低得多。过去表面，对像glsl在于的人类可读语法，一些GPU供应商对标准的解释相当灵活。如果使用他们的GPU编写了重要的着色器，将面临其他硬件驱动不兼容的风险。
    然而我们也不需要手动编写字节码，Khronos发布了编译器可将glsl编译为SPIR-V。此编译器旨在验证着色器代码是否完全符合标准，并生成一个SPIR-V二进制文件。也可以将此编译器作为库包含在运行时生成SPIR-V，但是本教程不这样做。虽然可以通过glslangValidator.exe直接使用编译器，但我们将glslc.exe改为由Google使用。glslc的优点是它使用普通编译器(如gcc和clang)相同的参数格式，并包含一些额外的功能，如include。它们都已经包含在了vkSDK中
    glsl是一种具有c风格语法的着色器语言。用它编写的程序有一个main为每个对象调用的函数。glsl不使用输入参数和返回值作为输出，而是使用全局变量来处理输入和输出。该语言包括许多有助于图像编程的功能，例如内置向量和矩阵济源。包括叉积、矩阵向量积个向量周围的反射等运算的函数。vec使用表示元素数量的数字调用向量类型。例如3D位置将存储在vec3，可以通过类似.x访问分量，但也可以同时从多个组件创建一个新向量，例如表达式vec3(1.0,2.0,2.0).xy会导致vec2。向量的构造函数也可以采用向量对象和标量值的组合，例如vec3可以用来构造vec3(vec2(1.0,2.0),3.0)*/
    //创建图形管线
    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("Shaders/vert.spv");//加载着色器的字节码
        auto fragShaderCode = readFile("Shaders/frag.spv");
        /*着色器模块只是着色器字节码和其中定义的函数的一个薄包装器。在创建图形管线之前，不会把SPIR-V字节码编译机器码和链接让GPU执行。所以一旦管线创建完成就可以再次销毁着色器模块，这就是为什么在createGraphicsPipeline中将它们设为局部变量而不是类成员的原因*/
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        //着色器阶段创建。要实际使用着色器需要通过结构体将着色器分配给特定的管线阶段，作为实际管线创建过程的一部分
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;//告诉vk这个shader在哪个阶段使用
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;//指定包含代码的着色器模块
        vertShaderStageInfo.pName = "main";//指定函数入口
        /*这里还有一个可选成员pSpecializationInfo，这里不用。它允许指定着色器的常量的值。可以使用单个着色器模块通过为其中使用的常量指定不同的值，可以在创建管线时配置其行为。这比在渲染时使用变量配置着色器更有效，因为编译器可以进行优化，例如消除if等。默认nulptr*/
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};//片元
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;//告诉vk这个shader在哪个阶段使用
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;//指定包含代码的着色器模块
        fragShaderStageInfo.pName = "main";//可以将多个fragShader组合到一个shaderModule中，并使用不同的入口点来区分它们的行为。但是我们坚持使用标准main

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };

        //较旧的图形API为图形管线的大多数阶段提供了默认阶段。在vk中必须明确大多数管线状态，因为它将被烘焙到不可变的管线状态对象中
        
        //顶点输入
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};//该结构体描述了将传递给顶点着色器的顶点数据的格式
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;//绑定：数据之间的间距以及数据是逐顶点还是逐实例
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;//属性描述：传递给顶点着色器的属性的类型，从哪个绑定加载它们以及在哪个偏移量
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
        //因为我们之间在顶点着色器中对顶点数据进行硬编码，所以暂时没有要加载的顶点数据
        //pVertexBindingDescriptions和pVertex AttributeDescriptions成员指向一个结构数组，该数组描述了加载顶点数据的上述详细信息。将此结构添加到shaderStages数组后面的createGraphicsPipeline函数中。

        //输入装配，输入汇编
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;//将从顶点绘制什么样的几何图形。VK_PRIMITIVE_TOPOLOGY_POINT_LIST: 来自顶点的点；VK_PRIMITIVE_TOPOLOGY_LINE_LIST-每2个顶点的线，不重复使用；VK_PRIMITIVE_TOPOLOGY_LINE_STRIP-每行的结束顶点用作下一行的开始顶点；VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST-来自每 3 个顶点的三角形，不重复使用；VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP-每个三角形的第二个和第三个顶点用作下一个三角形的前两个顶点。这里绘制三角形
        inputAssembly.primitiveRestartEnable = VK_FALSE;//是否应该启用图元重启
        //通常，顶点是按索引顺序从顶点缓冲区加载的，但使用元素缓冲区，您可以指定要使用的索引。这允许您执行优化，如重用顶点。如果将primitiveRestartEnable成员设置为VK_TRUE，则可以通过使用特殊索引0xFFFF或0xFFFFFF，在_STRIP拓扑模式中分割直线和三角形。
        
        //视口状态
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;//没有使用动态状态。在pipeline中设置viewport和scissorRectangle。这会让该pipeline的viewport和scissorRectangle不可变，对其任何更改都要重新创建pipeline
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        //无论如何设置都可以在某些显卡上使用多个viewport和scissorRectangle，通过结构体数组引用。但是使用多个需要启用GPU功能(参阅逻辑设备创建)

        //视口和剪刀。视口基本上描述了输出将被渲染到的帧缓存区区域
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;//交换链的大小及其图像可能与窗口的WIDTH和HEIGHT不同。交换链图像稍后将用作帧缓冲区，因此我们应该坚持它们的大小.
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;//minDepth和maxDepth值指定用于帧缓冲区的深度值范围。这些值必须在[0.0f, 1.0f]范围内，但 minDepth可能高于maxDepth。如果你没有做任何特别的事情，那么你应该坚持0.0fand的标准值1.0f。
        viewport.maxDepth = 1.0f;
        //视口定义了从图像到帧缓冲区的转换，而裁切矩形定义了实际存储像素的区域。裁切矩形之外的任何像素都将被光栅化器丢弃。他们的功能类似过滤器
        VkRect2D scissor{};//如果想绘制整个帧缓冲区，要指定一个完全覆盖它的裁切矩形
        scissor.offset = { 0,0 };
        scissor.extent = swapChainExtent;
        //视口和剪裁矩形既可以指定为管线的静态部分，也可以指定为命令缓冲区中的动态状态集。虽然前者更符合其他状态，但使视口和剪刀状态动态化通常很方便，因为它为您提供了更多的灵活性。这很常见，所有实现都可以处理这种动态状态而不会降低性能。
        
        //光栅化器。光栅化器获取vertShader中顶点形成的几何图形，将其转换为片元让fragmentShader着色。它还执行深度测试、表面剔除和裁剪测试，并且配置 为输出填充整个多边形或者仅边缘的 片元(线框渲染)
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;//如果设置为VK_TRUE，则将超出近平面和远平面的片元夹在它们上，而不是丢弃它们。这在一些特殊情况下很有用 比如阴影贴图。使用它需要启用GPU功能。
        rasterizer.rasterizerDiscardEnable = VK_FALSE;//如果设置为VK_TRUE，则几何图形永远不会通过光栅化阶段
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//确定如何为几何体生成片元。可以使用以下模式：VK_POLYGON_MODE_FILL-用片段填充多边形区域；VK_POLYGON_MODE_LINE-多边形边被画成线；VK_POLYGON_MODE_POINT-多边形顶点被绘制为点。
        rasterizer.lineWidth = 1.0f;//根据片元数描述线条的粗细。支持的最大线宽取决于硬件，任何大于1.0f的线都需要启用wideLines GPU功能。
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;//确定要使用的面剔除类型。可以禁用剔除、剔除正面、剔除背面或两者
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;//指定被视为正面的面的顶点顺序，可以是顺时针或逆时针
        rasterizer.depthBiasEnable = VK_FALSE;//添加常量值或基于碎片的斜率偏移深度值来更改深度值。这有时用于阴影贴图，但这里不会使用它。只需将depthBiasEnable设置为VK_FALSE。
        rasterizer.depthBiasConstantFactor = 0.0f;//optional
        rasterizer.depthBiasClamp = 0.0f;//optional
        rasterizer.depthBiasSlopeFactor = 0.0f;//

        //多重采样。是执行抗锯齿的方法之一。通过将光栅化到同一像素的多个多边形的fragShader结果组合在一起来工作。主要发生在边缘，也是最明显的锯齿伪影发生的地方。因为如果只有一个多边形映射到一个像素，它不需要多次运行fragShader。启动它需要启动GPU功能
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional禁用多重采样

        //深度和模板测试
        //如果使用深度或模板缓冲区，则还需要使用VkPipelineDepthStencilStateCreateInfo配置深度和模具测试。我们现在还没有，所以我们可以简单地传递一个nullptr，而不是指向这样一个结构的指针。我们将在深度缓冲一章中讨论它。

        //颜色混合。fragShader返回颜色后，需要将其与帧缓冲区中已有的颜色组合。这种转换称为颜色混合，有两种方法可以做到：混合旧值和新值以产生最终颜色/使用按位运算组合旧值和新值。
        //有两种类型的结构来配置颜色混合。第一个结构 VkPipelineColorBlendAttachmentState包含每个附加帧缓冲区的配置，第二个结构VkPipelineColorBlendStateCreateInfo 包含全局颜色混合设置。在我们的例子中，我们只有一个帧缓冲区：
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;//如果设置为VK_FALSE，则来自fragShader的新颜色会未经修改地通过
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        //第二个结构体引用所以帧缓冲区的结构体数值，并允许设置混合常量，用作混合因子
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;//为true时自动禁用第一种方法
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional//指定按位运算
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        
        //虽然大多数管线状态都需要被烘焙到管线状态中，但实际上可以更改有限的状态，而无需在绘制时重新创建管线。例如视口的大小线宽和混合常量。如果要使用动态状态并保留这些属性那必须填写必须填写VkPipelineDynamicStateCreateInfo结构
        std::vector<VkDynamicState>dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };
        //当选择动态视口和剪切矩形时，您需要为管线启用相应的动态状态：
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
        //这将导致这些值的配置被忽略，能够在绘制时指定数据。这会产生更灵活的设置，并且对于视口和裁剪状态之类的东西翡翠城就，当被烘焙到管线状态时hi导致更复杂的设置

        //管线布局，可以配置统一值uniform。uniform类似动态状态变量的全局值，可以在drawFrame的时候更改它们以改变shader的行为而无需重新创建shader。通常用于将变换矩阵传递给vertShader或在fragShader中创建纹理采样器。这里创建一个空的pipelineLayout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional//该结构还指定了push constants，这是将动态值传递给着色器的另一种方式，我们可能会在以后的章节中介绍。
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        /*现在拥有的对象类型：
        着色器阶段：定义图像管线可编程阶段功能的着色器模块；
        固定功能状态：定义管线固定功能阶段的所有结构，如输入组件、光栅化器、视口和颜色混合；
        管线布局：着色器引用的统一和推送值，可以在绘制时更新；
        渲染通道：管线阶段引用的附件及其用法。
        这些组合定义了图像管线的功能*/
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        //固定功能阶段
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        //管线布局，他是vk句柄而不是结构体指针
        pipelineInfo.layout = pipelineLayout;
        //渲染管线的引用和其subpass的索引
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        //vk允许您通过现有管线派生来创建新的图形管线。原理是，当管线与现有管线具有许多共同功能时，建立管线的成本较低，并且可以更快地在来自同一父管线的管线之间切换。
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;//指定现有管线的句柄
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {//第二个参数引用了一个可选VkPipelineCache对象。pipelineCache可用于存储和重用 与多个vkCreateGraphicPipelines调用 之间的管线创建相关的数据。如果缓存到文件中，则还可以用于跨程序执行之间的数据。这使得以后可以显著加快管线创建速度
            throw std::runtime_error("failed to create graphic pipeline!");
        }

        //最后删除着色器模块
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }
    //创建帧缓冲区。在渲染过程创建期间指定的附件包装到一个VkFramebuffer对象来绑定。帧缓冲区对象引用所以VkImageView表示附件的对象。必须为附件使用的图像取决于检索用于显示的图像时交换链返回的图像。所以必须为交换链中的所有图像创建一个帧缓冲区，并使用与drawFrame时检索到的图像相对应的帧缓冲区
    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());//调整容器的大小以容纳所有帧缓冲区

        //遍历图像视图并从中创建帧缓冲区
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = { swapChainImageViews[i] };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;//指定帧缓冲区与哪个renderPass兼容。只能将帧缓冲区与它兼容的渲染管线一起使用，所以它们使用相同数量和类型的附件
            framebufferInfo.attachmentCount = 1;//attachmentCount和pAttachments参数指定应绑定到渲染过程pAttachment数组中相应附件描述的VkImageView对象
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;//指图像阵列中的层数。我们的交换链图像是单个图像所以层数为1

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }
    //vk中的命令如绘图操作和内存传输，不是直接使用函数调用执行的。必须在命令缓冲区对象中记录要执行的所有操作。好处是当准备好让vk做什么时，所有命令会一起提交，vk可以更有效地处理这些命令因为他们都可以一起使用。此外 如果需要 允许在多个线程中进行命令记录
    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//命令池有两个可能的flag：VK_COMMAND_POOL_CREATE_TRANSIENT_BIT-提示命令缓冲区经常用新命令重新记录（可能会改变内存分配行为）；VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT-允许单独重新记录命令缓冲区，如果没有此标志，它们都必须一起重置。这里将每帧记录一个命令缓冲区，所以希望能够重置并重新记录它，所以用这个宏
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        //命令缓冲区通过将它们提交到设备队列之一来执行。每个命令池只能分配在单一类型队列上提交的命令缓冲区。我们将记录绘图命令，这就是选择图形队列的原因
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    //从cmdPool分配cmdBuffer
    void createCommandBuffers() {
        /*FrameInFlight：渲染循环有个缺陷：需要等待前一帧完成才能开始下一帧渲染，这会导致主机不必要的空闲
        解决方法是多个帧同时进行，即允许一帧的渲染不干扰下一帧的记录。做到这个，在渲染期间访问和修改的任何资源必须负责。因此需要多个cmdBuffer、semaphore、fence。
        定义同时处理的帧数为2是因为不希望CPU远远领先于GPU。有2帧在飞行中，CPU和GPU可以同时处理各自的任务。如果CPU提前完成则会等到GPU完成渲染后再提交更多工作。如果3帧或更多则CPU可能会领先GPU从而增加帧延迟。
        每个帧都应该有自己的cmdBuffer、semaphore和fence*/
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);//修改cmdBuffers的大小
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;//指定命令池
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//指定分配的命令缓冲区是主命令缓冲区还是辅助命令缓冲区。VK_COMMAND_BUFFER_LEVEL_PRIMARY-可以提交到队列执行，但不能从其他命令缓冲区调用。；VK_COMMAND_BUFFER_LEVEL_SECONDARY-不能直接提交，但可以从主命令缓冲区调用。
        allocInfo.commandBufferCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;//指定要分配的缓冲区数量
        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    //命令缓冲区记录。将我们想要执行的命令写入命令缓冲区的函数。使用的VkCommandBuffer将作为参数传入，以及要写入的当前swapchain映像的索引。
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //beginInfo.flags = 0;//指定如何使用命令缓冲区。可以使用以下值：VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT-命令缓冲区将在执行一次后立即重新记录；VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT-这是一个辅助命令缓冲区，将完全在单个渲染过程中；VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT-命令缓冲区可以在它也已经等待执行时重新提交。
        //beginInfo.pInheritanceInfo = nullptr;//仅与辅助命令缓冲区相关。指定从调用主命令缓冲区继承的状态。
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {//如果命令缓冲区已经记录过一次，那么调用vkBeginCommandBuffer将隐式重置它。以后不可能将命令附加到缓冲区
            throw std::runtime_error("failed to begin recoding command buffer!");
        }

        //绘制从开始渲染通道开始
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;//渲染过程本身
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];//要绑定的附件。我们为每个被指定为颜色附件的交换链图像创建了一个帧缓冲区。因此，我们需要为要绘制的交换链图像绑定帧缓冲区。使用传入的imageIndex参数，可以为当前交换链图像选择正确的帧缓冲区
        //指定渲染区域的大小。渲染区域定义着色器加载和存储即将发生的位置。此区域之外的像素将具有未定义的值。它应该与附件的大小相匹配以获得最佳性能
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = swapChainExtent;
        //最后两个参数定义用于VK_ATTACHMENT_LOAD_OP_clear的清除值，我们将其用作颜色附件的加载操作。我们已经将清晰的颜色定义为100%不透明度的黑色
        VkClearValue clearColor = { {{0.0f,0.0f,0.0f,1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        //开始渲染过程了。所有记录命令的函数都有vkCmd前缀。他们都返回void
        //每个命令的第一个参数始终是记录命令的命令缓冲区。第二个参数指定刚刚提供的渲染管线的信息，最后一个参数控制如何提供渲染过程中的绘图命令，它可以有两个值：VK_SUBPASS_CONTENTS_INLINE-渲染通道命令将嵌入到主命令缓冲区本身中并且不会执行辅助命令缓冲区；VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS-渲染通道命令将从辅助命令缓冲区执行。
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            //绑定图形管线。第二个参数指定管线对象是图形还是计算管线。我们已经告诉vk在图形管线中执行哪些操作以及在fragShader中使用哪个附件
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
            //正如在固定功能中提到的，我们为这个管线指定了视口和裁切状态为动态的。所以需要在发出绘图命令之前将他们设置在命令缓冲区中：
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChainExtent.width);
            viewport.height = static_cast<float>(swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0,0 };
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
            
            //现在准备发出三角形的绘制命令
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);//实际的vkCmdDraw功能虎头蛇尾，因为提前指定了所有信息。除了命令缓冲区外，还有以下参数：vertexCount：即使我们没有顶点缓冲区，但从技术上讲，我们仍然需要绘制 3 个顶点。；instanceCount-用于实例渲染，如果你不这样做，请使用1；firstVertex-用作顶点缓冲区的偏移量，定义_VertexIndex的最小值gl；firstInstance-用作实例渲染的偏移量，定义gl_InstanceIndex的最小值。

        //结束渲染过程
        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    /*同步：
    vk的核心设计理念是GPU上的执行同步是明确的。操作顺序取决于使用不同的同步原语来定义，这些原语告诉驱动程序我们希望运行的顺序。所以许多GPU上执行工作的vkAPI调用是异步的，函数会在操作完成之前返回
    在本章中，需要明确排序发生在GPU上的许多事件：从交换链中获取图像，执行在获取的图像上绘制的命令，将该图像呈现到屏幕上进行呈现并将其返回到交换链。
    这些事件都使用单个函数调用来启动而且都是异步执行的。函数调用在操作实际完成前返回，执行顺序也未定义。这是不幸的，因为每个操作都依赖之前操作完成。所以我们需要探索可以使用哪些原语来实现所需的排序
    信号量：信号量用于在队列操作之间添加顺序。队列操作是我们提交给队列的工作，可以是在命令缓冲区中，也可以是在函数中。队列的示例是图形队列和演示队列。信号量用于对同一队列内的工作进行排序
        vk有两种信号量，二进制和时间线。这里只使用二进制信号量不讨论时间线信号量
        信号量是无信号或者有信号的。它以无信号开始，使用信号量对队列进行排序的方法是，在一个队列操作中提供与"信号"信号量相同的信号量，在另一个队列运行中提供相同的"等待"信号量
        例如：假设有信号量S和队列操作A和B，我们想按顺序执行它们。我们想告诉vk的是，当操作A完成执行时，它将"向"信号量S发出信号，而操作B将在信号量S上"等待"然后再开始执行。当操作A结束时，信号量S将被发送信号，而操作B直到S被发送"信号"才开始。操作B开始执行后，信号量S自动重置为未签名，允许再次使用
        注意：等待只发生在GPU上，CPU继续运行不会阻塞。为了让CPU等待，需要使用一个不同的同步原语
    围栏：围栏有相似的用途，用于同步执行，但它用于在CPU(也称为主机)上排序执行。简单的说，如果主机需要知道GPU什么时候完成了什么，就使用一个围栏
        和信号量相似，围栏也处于有或无信号状态。无论何时提交要执行的工作，都可以在该工作上附加一个围栏。当工作完成时，围栏发出信号。然后我们可以让主机等待围栏发出信号，确保在主机继续前完成工作
        例如：截屏。假设已经在GPU上完成了工作。现在要将图像从GPU传输到主机，然后将内存保存到文件。我们有执行传输和围栏F的命令缓冲区A。我们提交带有围栏F的命令缓冲区A，然后立即通知主机等待F发出信号。这会导致主机阻塞，知道命令缓冲区A完成执行。所以当内存传输完成时，可以安全地让主机将文件保存到磁盘
        一般来说，除非必要否则最好不要阻止主机。我们想为GPU和主机提供有用的工作，在围栏上等待信号并不是一件有用的工作。所以我们更喜欢用信号量火其他尚未涉及的同步原语来同步工作
        必须手动重置围栏，使其恢复未签名状态。因为围栏用于控制主机的执行，所以主机可以决定何时重置围栏。相反，信号量用于在GPU上命令工作而不涉及主机
    总之，信号量用于指定GPU上操作的执行顺序，而围栏用于保存CPU和GPU彼此同步。
    选择什么？有两个同步原语可以用，并且有两个地方可以方便地应用同步：交换链操作和等待前一帧完成。我们将信号量用于交换链操作，因为他们发生在GPU上，我们不想让主机等待。对于等待前一帧完成我们使用围栏，因为我们想让主机等待。这样我们不会一次绘制多个帧。因为每一帧都重新记录命令缓冲区，所以在当前帧执行完毕前，无法将下一帧的工作记录到命令缓冲区中，因为我们不想在GPU使用蛮力缓冲区时覆盖其当前内容。*/

    //创建同步对象
    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{}; //信号量
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};//围栏
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        //创建信号量和围栏
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }
    //绘图
    void drawFrame() {
        /*概况vk中渲染帧常见步骤：等待上一帧完成->从交换链中获取图像->记录一个将场景绘制到该图像上的命令缓冲区->提交记录的命令缓冲区->显示交换链图像*/
        
        //在帧开始时等待前一帧结束，以便命令缓冲区和信号量可供使用。为此调用等待围栏vkWaitForFences，该函数接受一个围栏数组，并在主机上等待任何或所有围栏在返回之前翻出信号。VK_TRUE表示我们要等待所有的栅栏，但在单个栅栏的情况下没关系。该函数还有个超时参数，将其设置为64位无符号整数的最大值UINT64_MAX，它有效地禁用了超时。
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        //等待之后，手动将围栏重置为无信号状态
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        //在继续之前，我们的设计中有个小问题。在第一帧我们调用drawFrame()，它会等待inFlightFence发出信号，但是它只会在一帧完成渲染后发出信号，但是这是第一帧，所以没有以前的帧可以发出围栏信号，所以vkWaitForFences()无限期地阻塞。
        //API内置了一个巧妙的解决方法。在信号状态下创建围栏，以便第一次调用vkWaitForFences()立即返回，因为栅栏已经发出信号。
        
        //从交换链获取图像。交换链是一个扩展功能，所以我们必须使用vk*KHR命名约定的函数
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);//前两个参数是逻辑设备和交换链。第三个参数指定图像可用的超时时间(单位纳秒)。使用64位无符号整数的最大值意味着禁止超时。接下来两个参数指定当表示引擎使用图像完成时要向其发送信号的同步对象。这是可以开始绘制的时间点，可以指定信号量围栏或both，为此使用imageAvailableSemaphore。最后一个参数指定一个变量，用于输出可用的交换链映像的索引。索引引用swapChainImages数组中的VkImage，我们使用该索引现在VkFrameBuffer

        //记录命令缓冲区。使用imageIndex指定要使用的交换链图像，现在可用使用命令缓冲区。
        vkResetCommandBuffer(commandBuffers[currentFrame], 0);//对命令缓冲区调用此函数以确保它能够被记录。
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        //提交命令缓冲区。有了完整记录的命令缓冲区，就可以提交它
        VkSubmitInfo submitInfo{};//队列提交和同步是通过此结构体中的参数来配置的
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        //前三个参数指定在执行开始前等待哪些信号量，以及在管线的哪个阶段等待。我们希望等到图像可用时再向其写入颜色，所以我们要指定写入颜色附件的图形管线的阶段。这意味着理论上，在图像尚不可用的情况下，实现可以开始执行顶点着色器等。waitStages数组中的每个条目对应于pWaitSemaphores中具有相同索引的信号量。
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        //接下来的两个参数指定实际提交执行的命令缓冲区。我们只需要提交我们拥有的单个命令缓冲区
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        //signalSemaphoreCount和pSignalSemaphores参数指定命令缓冲区完成执行后要发出信号的信号量，我们使用renderFinishedSemaphore来完成此目的
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        //现在使用vkQueueSubmit将命令缓冲区提交到图形队列。当工作负载大得多时，该函数采用VkSubmitInfo结构体数组作为效率参数。最后一个参数引用了一个可选围栏，当命令缓冲区完成执行时，会发出信号。这让我们知道何时可以安全地重用命令缓冲区，所以我们希望在FlightFence中提供他。现在下一帧中，CPU将等待此命令缓冲区完成执行，然后将新命令记录到其中
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer!");
        }

        //将结果提交回交换链，使其最终显示到屏幕上。
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        //前两个参数指定在呈现之前要等待哪些信号量，就像vkSubmitInfo。由于我们想等待命令缓冲区完成执行，因此绘制三角形，我们获取将被发送信号的信号量并等待他们，因此我们使用signalSemaphores
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        //接下来的两个参数指定将图像呈现到交换链以及每个交换链的图像索引。这几乎总是一个
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        //最后一个可选参数，如果演示成功，它允许指定一组vkResult值来检查每个单独的交换链。如果只使用单个交换链，则没必要，因为可以简单的使用当前函数的返回值
        presentInfo.pResults = nullptr;
        //vkQueuePresentKHR函数提交向交换链呈现图像的请求
        vkQueuePresentKHR(presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;//使用模%运算符，确保循环
    }
    //在将代码传递给管线之前，必须将它包装在一个VKShaderModule对象中
    VkShaderModule createShaderModule(const std::vector<char>& code) {//该函数将使用字节码 作为参数的缓冲区，并从中创建一个VKShaderModule对象。如果缓存存储到文件中，则管线缓存可用于跨多次调用vkCreateGraphicsPipelines甚至跨持续执行存储和重用与管线创建相关的数据。这使得以后可以显著加快管线创建速度
        //创建着色器模块很简单，只需要用字节码和长度指定一个指向缓冲区的指针
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        /*一个问题是字节码的大小以字节为单位指定，但字节码指针是uint32_t指针而不是char指针。因此，我们需要将指针转换 reinterpret_cast为如下所示。当您执行这样的强制转换时，您还需要确保数据满足uint32_t. 幸运的是，数据存储在std::vector默认分配器已经确保数据满足最坏情况对齐要求的位置。*/
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderMoudle;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderMoudle) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderMoudle;
    }
    //为交换链选择最佳表面格式设置
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {//每个VkSurfaceFormatKHR都包含一个format和一个colorSpace。格式指定颜色通道和类型(例如VK_FORMAT_B8G8R8A8_SRGB意味着以8位无符号整数的顺序产出BGR和alpha通道，每个像素共32位)色彩空间指示是否支持srgb颜色空间，或者是否使用VK_color_space_SRGB_NONLINEAR_KHR标志
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {//颜色空间尽量使用srgb，它产生更精确的感知颜色，它也是图像的标准颜色空间
                return availableFormat;
            }
        }
        return availableFormats[0];//如果这也失败了，可以根据它们的程度对可用格式排名，大多数情况下第一种就可以了
    }
    //为交换链选择最佳演示模式设置
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        /*演示模式是交换链最重要的设置，它代表了将图像显示到屏幕的实际条件，vk有四种可能的模式：
            VK_PRESENT_MODE_IMMEDIATE_KHR：app提交的图像会立即转移到屏幕上，可能会导致撕裂。
            VK_PRESENT_MODE_FIFO_KHR：交换链是一个队列，当显示器刷新时，显示器从队列的前面获取图像，程序将渲染的图像插入到队列的后面。如果队列已满，则程序必须等待。这与现代游戏中的垂直同步最为相似。刷新显示的那一刻称为“垂直空白”。
            VK_PRESENT_MODE_FIFO_RELAXED_KHR：此模式仅在应用程序迟到并且队列在最后一个垂直空白处为空的情况下与前一种模式不同。图像最终到达时立即传输，而不是等待下一个垂直空白。这可能会导致明显的撕裂。
            VK_PRESENT_MODE_MAILBOX_KHR: 这是第二种模式的另一种变体。队列已满时不会阻塞应用程序，而是将已排队的图像简单地替换为较新的图像。此模式可用于尽可能快地渲染帧，同时仍避免撕裂，与标准垂直同步相比，延迟问题更少。这就是俗称的“三重缓冲”，虽然单独存在三个缓冲并不一定意味着帧率是解锁的。*/
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {//能三重缓冲就三重缓冲，不能就双重缓冲
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    //为交换链选择最佳交换范围设置
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        //交换范围是交换链图像的分辨率，几乎等于正在绘制的窗口的分辨率。分辨率范围在VkSurfaceCapabilitiesKHR结构中定义
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            VkExtent2D actualExtent = { static_cast<uint32_t>(width),static_cast<uint32_t>(height) };
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }
    //查询交换链支持
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);//表面功能，交换链的核心组件
        //查询支持的表面格式
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);//表面格式
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        //查询支持的演示模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }
    //评估该显卡适合工作
    bool isDeviceSuitable(VkPhysicalDevice device) {
        //VkPhysicalDeviceProperties deviceProperties;
        //VkPhysicalDeviceFeatures deviceFeatures;
        //vkGetPhysicalDeviceProperties(device, &deviceProperties);//获得名称、类型、支持的vk版本等设备基础属性
        //vkGetPhysicalDeviceFeatures(device, &deviceFeatures);//查询对纹理压缩、64位浮点数和多视口渲染(VR相关)等可选功能的支持
        
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        //查询交换链支持
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }
    //检查设备扩展支持
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties>availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string>requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());//用一组字符串来表示未确认的所需扩展

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    //vk所有操作，从绘图到上传纹理都要将命令提交到队列
    //查找需要的所有队列族。检查设备支持哪些队列族以及其中哪一个支持想使用的命令
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);//检索设备队列族列表
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        //该VkQueueFamilyProperties结构体包含有关队列系列的一些详细信息，包括支持的操作类型以及基于该系列创建的队列数量
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {//遍历每一个队列族
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);//查找能够呈现到窗口表面的队列族(物理设备，队列族索引，窗口表面句柄)
            if (presentSupport) {
                indices.presentFamily = i;//查到了能呈现到surface的queueFamily，并把他保存到QueueFamilyIndices
            }
            if (indices.isComplete()) break;
            i++;
        }
        return indices;
    }
    //根据验证层是否启用 返回创建messenger所需的扩展列表。默认验证层会将调试消息打印，但也可在程序中显式回调来处理 因为有的错误不致命
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*>extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//使用VK_EXT_debug_utils扩展设置带有回调的调试信使。这个宏等于文字字符串VK_EXT_debug_utils
        }
        return extensions;
    }
    //检查所有请求的验证图层是否都可用
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);//列出所有可用验证层
        std::vector<VkLayerProperties>availableLayers(layerCount);//用来存储可用验证层
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        //检查要启动的验证层是否可用(availableLayers是否包含所有validationLayers)
        for (const char* layerName : validationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {return false;}
        }
        return true;
    }
    //从文件中加载二进制数据：从指定文件中读取所有字节并将它们返回到std::vector管理的字节数组中
    static std::vector<char>readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);//ate:从文件末尾开始读取，binary:将文件读取为二进制文件(避免文本转换)
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();//从末尾开始读取的好处是可以使用读取位置来确定文件大小并分配缓冲区
        std::vector<char>buffer(fileSize);
        //之后回到文件的开头并一次性读取所有字节：
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        //关闭文件并返回字节
        file.close();
        return buffer;
    }
    //validation消息回调函数。使用PFN_vkDebugUtilsMessengerCallbackEXT原型添加一个名为debugCallback的静态成员函数。
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(//VKAPI_ATTR和VKAPI_CALL确保函数签名正确以供vk调用。
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,//指定消息的严重性。它是以下标志之一：VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT-诊断信息；VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT-信息性消息例如创建资源；VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT-关于行为的消息,不一定是错误,但很可能是您的应用程序中的错误；VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT-有关无效且可能导致崩溃的行为的消息。
        VkDebugUtilsMessageTypeFlagsEXT messageType,//可以具有以下值：VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT-发生了一些与规格或性能无关的事件；VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT-发生了违反规范或表明可能存在错误的事情；VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT-Vulkan 的潜在非最佳使用
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,//包含回调消息详细信息的VkDebugUtilsMessengerCallbackDataEXT结构体，其中最重要的是：pMessage-调试消息作为一个以空字符结尾的字符串；pObjects-与消息相关的vk对象句柄数组；objectCount-数组中的对象数。
        void* pUserData) {//该pUserData参数包含一个在回调设置期间指定的指针，并允许您将自己的数据传递给它。
        std::cerr << "***ValidationLayer: " << pCallbackData->pMessage << std::endl;//将会把validation消息打印
        return VK_FALSE;//回调返回一个布尔值，是否中止触发验证层消息的vk调用。如果返回true，则调用将中止并返回VK_ERROR_VALIDATION_FAILED_EXT错误。这通常仅用于测试验证层本身，因此应始终返回VK_FALSE。
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}