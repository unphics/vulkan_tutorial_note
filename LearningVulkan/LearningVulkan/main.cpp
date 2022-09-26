#define GLFW_INCLUDE_VULKAN//GLFW�������Լ��Ķ��岢�Զ�����Vulkan��ͷ
#include <GLFW/glfw3.h>
//std
#include <iostream>//������ͷ�ļ����ڱ���ʹ���error
#include <stdexcept>
#include <cstdlib>//�ṩEXIT_SUCCESS��EXIT_FAILURE��
#include<vector>
#include<cstring>
#include<optional>
#include<set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include<fstream>
/*
vkCreateXXX ��������
vkAllocateXXX �������
hint ��ʾ����ʾ
actual ʵ�ʵ�
resizable �ɵ�����С��
terminate ��ֹ
enumerate �о١�ö��
feature ��ɫ����
capability ����������
scissor ����
uniform ͳһ���Ʒ�

validation ��֤��
allocator ������
messenger ��ʹ���������еĶ���ʾ��֤��Ϣ������ʹ��ͬdebugMessenger
layer �㡢ͼ�㡣�������еĶ���ʾ��֤�㣬ͬvalidationLayer��validation
extension ��չ
physicalDevice �����豸-�Կ�
device �豸���������еĶ���ʾ�߼��豸��ͬlogicalDevice
queue ����
queueFamily ������
surface ���档�������еĶ���ʾ���ڱ���windowSurface��surface��һ��window��ȾĿ��ĳ���
swapChain ������
imageView ͼ����ͼ��
pipeline ���ߡ��������еĶ���ʾͼ�ι��ߣ�ͬgraphicPipeline
scissorRectangle �ü�����
rasterizer ��դ����
renderPass ��Ⱦͨ��
attachment ����
drawFrame ����
commandPool �����(�������ڴ洢���������ڴ棬�����з����������)
semaphore �ź���
fence Χ��
*/
const uint32_t WIDTH = 800;//���� ���ڵĿ��
const uint32_t HEIGHT = 600;//���� ���ڵĸ߶�
const int MAX_FRAMES_IN_FLIGHT = 2;//����ͬʱ�����֡����
//����vkSDK�ṩ�ı�׼��֤�㡣��֤���п��ԣ����ݹ淶������ֵ�Լ�����ã����ٶ���Ĵ����������Բ�����Դй©��ͨ�����ٵ���Դ�Ե��߳�������̰߳�ȫ����ÿ�����ü��������¼����׼��������� Vulkan �����Խ��з������طš�
const std::vector<const char*> validationLayers = { //�洢Ҫʹ�õ�ȫ����֤��
    "VK_LAYER_KHRONOS_validation"//��֤����Ҫͨ��ָ�����������ã����б�׼��֤��������SDK�е�һ�������ΪVK_LAYER_KHRONOS_validation
};
//��������չ��
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME//vk�ṩ�ĺ꣬����ΪVK_KHR_SWAPCHAIN,�������ĺô��Ǳ������ᷢ��ƴд����
};
//���ñ���-�Ƿ�����validationLayer
#ifdef NDEBUG//�ǵ���
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
//������ʹ������ϢVkDebugUtilsMessengerCreateInfoEXT�Դ���vkCreateDebugUtilsMessengerEXT��ʹ����ĺ�����
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");//vkCreateDebugUtilsMessengerEXT������ʹ��������һ����չ(�����Զ�����)������ʹ��vkGetInstanceProcAddr�������ĵ�ַ������Ӧ��ʹ�ô������ں�̨����������
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
//����messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");//������������������ʹ�ĺ��� �ĵ�ַ
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
//����֧��ͼ������Ķ���������
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t>presentFamily;//�ҵ�֧�ֳ��ֵ�����Ķ�����
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();//��ⲻΪnull
    }
};
//����֧�ֽ�����������
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;//�������湦��(��������ͼ�����С/�������,ͼ�����С/����Ⱥ͸߶�)
    std::vector<VkSurfaceFormatKHR>formats;//�����ʽ(���ظ�ʽ,ɫ�ʿռ�)
    std::vector<VkPresentModeKHR>presentModes;//���õ���ʾģʽ
};
//��������װ��һ������
class HelloTriangleApplication {
public:
    void run() {//��ִ�г���
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
private:
    GLFWwindow* window;//ʵ�ʵĴ���

    VkInstance instance;//vkʵ���ľ��
    VkDebugUtilsMessengerEXT debugMessenger;//������ʹ�������֤��Ϣ�ص��ǵ�����ʹ��һ����
    VkSurfaceKHR surface;//vk�봰��ϵͳ����������//����glfwGetRequiredInstanceExtensions�����б����Ѿ����ù�
    
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;//ѡ����Կ��ľ������vkʵ��������ʱ���������ᱻ��ʽ���٣���˲���Ҫ��cleanUp��ִ���κβ���
    VkDevice device;//�߼��豸���
   
    VkQueue graphicsQueue;//ͼ�ζ��еľ��(�������߼��豸һ���Զ����� ������Ҫ�����ǽ����ľ��)�豸������ʱ�ᱻ��ʽ����
    VkQueue presentQueue;//��ʾ���еľ��
    
    VkSwapchainKHR swapChain;//�洢VkSwapchainKHR����
    std::vector<VkImage>swapChainImages;//�洢��������vkImages�ľ��
    VkFormat swapChainImageFormat;//�洢Ϊ������ѡ��ĸ�ʽ
    VkExtent2D swapChainExtent;//�洢Ϊ������ѡ��ķ�Χ
    std::vector<VkImageView>swapChainImageViews;//�洢ͼ����ͼ,ͼ����ͼʵ�����Ƕ�ͼ�����ͼ����������η���ͼ���Լ�����ͼ����ĸ�����
    std::vector<VkFramebuffer>swapChainFramebuffers;//֡������
    
    VkRenderPass renderPass;//��Ⱦͨ��
    VkPipelineLayout pipelineLayout;//���߲���
    VkPipeline graphicsPipeline;//ͼ�ι���

    VkCommandPool commandPool;//����ء�����ع������ڴ洢���������ڴ棬�����з����������
    std::vector<VkCommandBuffer>commandBuffers;//������������� ����������������ر�����ʱ�Զ�ʩ�ţ���˲���Ҫ��ʽ����
    
    //�洢��Щ�ź��������դ������
    std::vector<VkSemaphore>imageAvailableSemaphores;
    std::vector<VkSemaphore>renderFinishedSemaphores;
    std::vector<VkFence>inFlightFences;

    uint32_t currentFrame = 0;//֡������Ҫ��ÿһ֡��ʹ����ȷ�Ķ�����Ҫ���ٵ�ǰ֡

    //��ʼ��glfw����������
    void initWindow() {
        glfwInit();//��ʼ��glfw��
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//����glfw������opengl������
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//resizable�ɵ�����С��
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//��������(�߶�,���,����,����ָ���򿪴��ڵ���Ļ,����OpenGL�йصĲ���)
    }
    //initVk�е������е� ����vk����ĺ���
    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurace();//���ڱ�����instance��������������������Ӱ�������豸����ת
        pickPhysicalDevice();
        //ѡ��������豸����Ҫ����һ���߼��豸���佻��
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
        //��ѭ����һֱ����ֱ�����ڹر�
        while (!glfwWindowShouldClose(window)) {//���ر��¼�
            glfwPollEvents();
            //std::cout << "ȫ����֤��������" << validationLayers.size() << std::endl;
            std::cout << "currentFrame:" << currentFrame << std::endl;
            drawFrame();//��ͼ
        }
        vkDeviceWaitIdle(device);
    }
    //�����ڹرղ�mainLoop���أ���cleanup���ͷ���Դ
    void cleanup() {
        //���������������������ɲ�����Ҫͬ��ʱ�����ź�����Χ��
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
        for (auto imageView : swapChainImageViews) {//ͼ����ͼ����ȷ�����ģ���Ҫ���һ�����Ƶ�ѭ������
            vkDestroyImageView(device, imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);//�����߼��豸
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);//����messenger
        }
        vkDestroySurfaceKHR(instance, surface, nullptr);
        //�ڳ����˳�ǰ����instance����������vk��Դ��Ӧ����instance������ǰ����
        vkDestroyInstance(instance,nullptr);//vk�еķ�����ͷź�������һ��Allocator�ص�������ͨ��nullptr����
        glfwDestroyWindow(window);
        glfwTerminate();//glfw��ֹ
    }
    //����instance����ʼ��vk�⣬����app��vk��
    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {//app�ڵ���ģʽ�£�������֤�㶼����
            throw std::runtime_error("validation layers requested,but not available!");
        }

        //�ڽṹ������д����app����Ϣ�Դ���ʵ����vk�������Ϣ��ͨ���ṹ������Ǻ������������ݵ�
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;//vk�����ṹ�嶼Ҫ����sType����ʽָ������
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);//version�汾
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};//����vk����ʹ����Щextension��validation
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        //app�����extensions(����validation��swapChain��)
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};//��debugCreateInfo��������if���֮����Ϊ����vkCreateInstance����ǰ�����ᱻ�ƻ�������һ������ĵ�����ʹ�������ڴ��ڼ��Զ�ʹ��vkCreateInstance֮����vkDestroyInstance����
        //�ṹ������������Աȷ��Ҫ���õ�ȫ����֤��
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();//�������õ���֤������
            //��Ȼapp���Ѿ�������֤��Ϣ���Իص����������ڵ��õĹ�ϵ vkCreateInstance��vkDestroyInstanceû�б����ԡ���չ�ĵ�����ר��Ϊ������������������������ʹ�ķ�����ֻҪ��VkInstanceCreateInfo��pNext��չ�ֶ��д���һ��ָ��VkDebugUtilsMessengerCreateInfoEXT������ʹ������Ϣ��ָ��
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        //����ʵ����vk�д�������ĺ���һ�㶼��(ָ�򴴽���Ϣ�Ľṹ��ָ�룬ָ���Զ���������ص���ָ�룬ָ��洢�¶�������ָ��)
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {/*�������ģ�Vulkan�еĶ��󴴽�����������ѭ��һ��ģʽ�ǣ�ָ����д�����Ϣ�Ľṹ���ָ�룻ָ���Զ���������ص���ָ�룬����Ŀ����nullptr��ָ�򴢴��¶������ı�����ָ�롣*///�������е�Vulkan�������᷵��һ������ֵVkResult��Ҫô��VK_SUCCESSҪô�Ǵ������
            throw std::runtime_error("failed to create instance!");
        }
    }
    //����ʹ�����������ȡ�������ĺ�����
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;//ָ��ϣ�����ûص�����������������
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;//����֪ͨ�ص�����Ϣ����
        createInfo.pfnUserCallback = debugCallback;//ָ��ָ��ص�������ָ��
    }
    //����validaiton��Ϣ���Իص�messenger
    void setupDebugMessenger() {
        if (!enableValidationLayers)return;//���ڵ���ģʽ�¾�û��Ҫʹ��validation
        VkDebugUtilsMessengerCreateInfoEXT createInfo;//�ڽṹ������д�й���ʹ���������Ϣ
        populateDebugMessengerCreateInfo(createInfo);//���messenger������Ϣ
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    //�������ڱ���
    void createSurace() {
        //vkCreateWin32SurfaceKHR ��������(ʵ���Ĳ��������洴��Ϯ�����Զ����������Ҫ�����ı���߱�) ����һ��WS��չ�����ǳ�����������vk������������������Ҫ��ʾ����
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create windwo surface!");
        }
    }
    //ѡ�������豸
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);//�о��Կ�
        if (deviceCount == 0)throw std::runtime_error("failed to find GPUs with Vulkan support!");//���û���Կ�֧��vulkan��û�б�Ҫ��������
        std::vector<VkPhysicalDevice>devices(deviceCount);//��������VkPhysicalDevice���d ����
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());//�о��Կ�
        for (const auto& device : devices) {//����ÿ���Կ��Ƿ��������Ҫ��
            if (isDeviceSuitable(device)) {
                physicalDevice = device;//�ҵ�������Ҫ����Կ�
                break;
            }
        }
        if (physicalDevice == VK_NULL_HANDLE)throw std::runtime_error("failed to find a suitable GPU!");//�������е��Կ���������
    }
    //�����߼��豸
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        //������Ҫ�ж��VkDeviceQueueCreateInfo�ṹ��������ϵ�д���һ�����С������Ǵ���һ��������� ������� ����Ψһ������
        std::vector<VkDeviceQueueCreateInfo>queueCreateInfos;
        std::set<uint32_t>uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value()};

        //����queueFamily��info
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};//�ṹ��VkDeviceQueueCreateInfo������������Ҫ�ĵ���������Ķ�����
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;//��ǰ����ֻ��Ϊÿ��queueFamily��������queue��ʵ�ʲ���Ҫ���queue��Ϊ���Զ��̴߳����������������ͨ��һ���Ϳ������������߳�һ���ύ
            
            float queuePriority = 1.0f;//ʹ��float(0-1)Ϊ���з������ȼ� Ӱ���������ִ�еĵ���
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};//ָ����Ҫʹ�õ��豸���ܼ�

        //�����߼��豸
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();//ָ����д�����Ϣ��ָ��
        createInfo.pEnabledFeatures = &deviceFeatures;//ָ���豸���ܽṹ���ָ��
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());//ָ���ض����豸����չ
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();//�����豸��չ

        if (enableValidationLayers) {//�ض����豸����֤��
            createInfo.enabledLayerCount = static_cast<int32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }else {
            createInfo.enabledLayerCount = 0;
        }
        //ʵ�����߼��豸(��֮�����������豸��������Ϣ(���е�)���������ص�ָ�룬ָ���߼��豸�ľ����ָ��)
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        //����ÿ��������Ķ��о�����������߼��豸�������塢����������ָ��洢���о����ָ��
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }
    //����������//�������ı�����һ���ȴ����ֵ���Ļ�ϵ�ͼ����С�app��ȡ��ͼ�񲢻��������ٽ��䷵�ض����С����еľ��幤����ʽ�Լ��Ӷ����г���ͼ��ķ�ʽȡ���ڽ����������á�һ�㽻������Ŀ����ʹͼ��ĳ�������Ļ��ˢ����ͬ��
    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport=querySwapChainSupport(physicalDevice);//��ѯ�����豸�Ľ�����֧�����
        //Ϊ������ѡ���������
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
        //���ý�����ͼ������
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount+1;//�����������������Сͼ������//ֱ������Сֵ�����б���ȴ�������������ڲ�����Ȼ����ܻ�ȡ��һ��Ҫ��Ⱦ��ͼ��������һ�������������Сֵ���ͼ��
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {//ȷ�����������ͼ��������0��һ������ֵ��ʾû�����ֵ
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        //���������󴴽���Ϣ
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;//ָ��������Ӧ��㵽�ĸ�����
        //ָ��������ͼ�����ϸ��Ϣ
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;//ָ��ÿ��ͼ������Ĳ�����������������3Dapp������=1
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//λ�ֶ�ָ�����������е�ͼ����к��ֲ���
        //ָ����δ��� ����������ʹ�õ� ������ͼ�����ǽ���ͼ�ζ����еĽ������л���ͼ�Σ���ν������ύ����ʾ�����С�
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamulyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//ͼ������ڶ����������ʹ�ã�������ʽ����Ȩת�ơ�
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamulyIndices;
        }else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//ͼ��һ����һ��������ӵ�У������ڽ���������һ��������֮ǰ��������ȷת������Ȩ����ѡ���ṩ������ܡ�
            createInfo.queueFamilyIndexCount = 0;//optional
            createInfo.pQueueFamilyIndices = nullptr;//optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;//���ǿ���ָ�������֧��ĳ���任����Ӧ����Ӧ���ڽ������е�ͼ�񣨹�����֧�ָñ任������˳ʱ����ת90�Ȼ�ˮƽ��ת��Ҫָ������Ҫ�κ�ת����ֻ��ָ����ǰת����
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//���ֶ�ָ���Ƿ�ʹ��alphaͨ���봰��ϵͳ�е��������ڻ�ϡ��������
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;//���ֶ�����Ϊtrue��ζ�Ų����ı��ڵ������ص���ɫ

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        //����������(�߼��豸��������Ϣ����ѡ�ķ�������ִ�д洢�����ָ��)
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        //����������ͼ��
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        //��ѡ��Ľ�������ʽ�ͷ�Χ�洢�ڳ�Ա������
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }
    //Ϊ�������е�ÿ��ͼ�񴴽�һ��������ͼ����ͼ�������û����Խ�����������ɫĿ��
    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());//�����б�Ĵ�С����Ӧ��Ҫ����������ͼ����ͼ
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};//����ͼ����ͼ�Ĳ���
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            //viewType��formatָ����ν���ͼ������
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;//����ͼ����Ϊ1D����2D����3D�������������ͼ
            createInfo.format = swapChainImageFormat;
            //components���������Χ����ɫͨ��������ʹ��Ĭ��ӳ��
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            //subresourceRange����ͼ�����;�Լ�Ӧ�÷���ͼ�����һ���֡����ǵ�ͼ�� ����û���κ�mipmapping����������ɫĿ��
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            //�����������3Dapp����ô���������ж����Ľ�������Ȼ��ͨ�����ʲ�ͬ�Ĳ�Ϊÿ����ʾ���ۺ�������ͼ��ͼ�񴴽����ͼ����ͼ
            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {//����ͼ����ͼ
                throw std::runtime_error("failed to create image views!");
            }
        }
    }
    //������Ⱦͨ�����ڴ���pipeline֮ǰ������vk��Ⱦʱ��ʹ�õ�֡������������ָ�����ж�����ɫ����Ȼ�������ÿ��������ʹ�ö����������Լ���������Ⱦ������Ӧ����δ������ǵ�����
    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;//��ɫ������formatӦ���뽻����ͼ��ĸ�ʽ��ƥ��
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//��ʱû�жԶ��ز������κ����飬����ʹ��1������
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//ȷ����Ⱦǰ��δ������е����ݡ�VK_ATTACHMENT_LOAD_OP_LOAD-�����������������ݣ�VK_ATTACHMENT_LOAD_OP_CLEAR-�ڿ�ʼʱ��ֵ���Ϊ������VK_ATTACHMENT_LOAD_OP_DONT_CARE-��������δ���壬���ں����ǡ�������ʹ������������ڻ�����֮֡ǰ�����������Ϊ��ɫ
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//soreOp�����ֿ����ԣ�VK_ATTACHMENT_STORE_OP_STORE-��Ⱦ�����ݽ��洢���ڴ��У��Ժ���Զ�ȡ��VK_ATTACHMENT_STORE_OP_DONT_CARE-��Ⱦ������δ����֡�����������ݡ�������д洢����
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//loadOp��storeOpӦ������ɫ��������ݣ�stencilLoad/stencilStoreOp������ģ�����ݣ�app�����ģ�建�������κ����飬��˼��غʹ洢�Ľ�����޹صġ�
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        //�����Ĳ��֣�VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL-������ɫ������ͼ��VK_IMAGE_LAYOUT_PRESENT_SRC_KHR-����������ʾ��ͼ��VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL-�����ڴ渴�Ʋ���Ŀ���ͼ��
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;//ָ������Ⱦ����֮ǰͼ��Ĳ��֡�ʹ��VK_IMAGE_LAYOUT_UNDEFINED�������ǲ�����ͼ����ǰ�Ĳ��֣�����ͼ��Ĳ��ܲ��ܱ�֤�����棬�ⲻ��Ҫ��Ϊ����������ζ�Ҫ�������
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//ָ������Ⱦ�������ʱ�Զ����ɵ��Ĳ��֡�����ϣ��ͼ������Ⱦ�����ʹ�ý�����������ʾ�������Ϊʲôʹ��VK_IMAGE_LAYOUT_PRESENT_SRC_KHR��Ϊ���ղ���

        /*����renderPass���԰������subpass��subpass�Ǻ�����Ⱦ����������������ǰͨ����֡�����������ݣ���������������Щ��Ⱦ�������鵽һ����Ⱦ�����У���ôvk�ܹ����������������ʡ�ڴ�����Ի�ø��õ����ܡ�������ʹ�õ���subpass*/
        VkAttachmentReference colorAttachmentRef{};//ÿ��subpass������һ������ǰ��Ľṹ�������ĸ���
        colorAttachmentRef.attachment = 0;//�ò���ͨ������������ֵ�е�����ָ��Ҫ���õĸ�����������һ��VkAttachmentDescription��ɣ���������������0
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;//ָ��������ʹ�ô����õ�subpass�ڼ�Ĳ��֡���subpass����ʱvk���Զ�������ת�����˲��֡�VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL����˼�壬���Ǵ��㽫����������ɫ�����������ֽ�Ϊ�����ṩ������ܡ�

        VkSubpassDescription subpass{};//����subpass
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        //vk����Ҳ����֧�ּ���subpass����˱���˵������һ��ͼ��subpass��ָ������ɫ���������ã�
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;//�������и�����������ֱ�Ӵ�fragShader��layout(location=0)out vec4 outColorָ�����õ�
        //subpass�������������������͵ĸ�����pInputAttachments-����ɫ���ж�ȡ�ĸ�����pResolveAttachments-���ڶ��ز�����ɫ�����ĸ�����pDepthStencilAttachment-��Ⱥ�ģ�����ݵĸ�����pPreserveAttachments-����ͨ��δʹ�õ����뱣�������ݵĸ�����

        //��ͨ���������Ⱦ�����е�subpass���Զ�����ͼ�񲼾ֱ任����Щת����subpass��������ƣ���ָ��subpass֮����ڴ��ִ��������ϵ������ֻ��һ��subpass���������subpass֮ǰ��֮��Ĳ���Ҳ����ʽ"subpass"
        //��renderPass��ʼ��renderPass����ʱ���������������������ȣ���ǰ�߲�������ȷ��ʱ�䷢����������ת�������ڹ��ߵĿ�ʼ���������ǻ�û�л�øõ��ͼ�������ַ�����������ǿ��Խ�iamgeAvailableSemaphore��waitStages����ΪVK_PIPELINE_STAGE_TOP_OF_PIPE_BIT����ȷ����Ⱦ������ͼ�����֮ǰ���Ὺʼ���������ǿ���ʹ��Ⱦ���̵ȴ�VK_PPIELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT�׶Ρ�����ʹ�õڶ���ѡ���Ϊ���Ǹ��ܺõĲ鿴subpass������乤����ʽ�Ľ��
        VkSubpassDependency dependency{};
        //ǰ�����ֶ�ָ������������subpass��������VK_SUBPASS_EXTERNAL��ָ��Ⱦ����֮ǰ��֮�����ʽsubpass��ȡ�������Ƿ���srcSubpass��dstSubpass�С�����0ָ����subpass�����ǵ�һ��Ҳ��Ψһһ����dstSubpass����ʼ�ո���srcSubpass�Է�ֹ�����ϵͼ�г���ѭ��(��������һ��subpass��VK_SUBPASS_EXTERNAL)
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        //�������������ֶ�ָ��Ҫ�ȴ��Ĳ����Լ���Щ���������Ľ׶Ρ�������Ҫ�ȴ���������ɶ�ͼ��Ķ�ȡȻ����ܷ������������ͨ���ȴ���ɫ������������������
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        //ͨ��VkRenderPassCreateInfoʹ�ø�����subpass�������ṹ������renderPass����VkAttachmentReference����ʹ�ô�������������ø���
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
    /*ͼ�ι�����һϵ�в�����������Ķ��������һֱ������ȾĿ���е�����:
    Vertex/Index Buffer->InputAssembler���������->VertexShader->Tessellationϸ��->GeometryShader->Rasterization��դ��->FragmentShader->ColorBlending->FrameBuffer
    �������� ��ָ���������ռ�ԭʼ�������ݣ�Ҳ�����������������ظ�ĳЩԪ�ز��ø��ƶ������ݱ���
    ������ɫ�� ÿ���������У�Ӧ��ת��������λ�ô�ģ�Ϳռ�ת������Ļ�ռ䡣��ÿ����������ݴ��ݵ������С�
    ϸ����ɫ�� ����ĳЩ����ϸ�ּ��������������������ͨ������ʹשǽ��¥�ݵȱ����ڸ���ʱ����������ôƽ̹��
    ������ɫ�� ��ÿ��ͼԪ(�����Ρ��ߡ���)�����У����ҿ��Զ��������������������ͼԪ��������������ϸ����ɫ�������������ǣ����ڵ����Ӧ�ó�����ʹ�õò����࣬��Ϊ����Intel�ļ���GPU֮�⣬������Կ������ܶ�������ô�á�
    ��դ���׶� ��ͼԪ��ɢΪƬ�Ρ�����������֡����������������Ԫ�ء��κ�������Ļ���Ƭ�ζ��ᱻ������������ɫ����������Իᱻ��ֵ��Ƭ���С�ͨ������ԭʼƬ�κ����Ƭ��Ҳ����Ϊ��Ȳ��Զ������ﱻ������
    ƬԪ��ɫ�� Ϊÿ���Ҵ��Ƭ�ε��ã���ȷ����Ƭ��д����Щ֡�������Լ�ʹ����Щ��ɫ�����ֵ��������ʹ�����Զ�����ɫ���Ĳ�ֵ������ִ�д˲��������п��԰�����������͹��շ��ߵ����ݡ�
    ��ɫ��Ͻ׶� Ӧ�ò��������ӳ�䵽֡����������ͬ���صĲ�ͬƬ�Ρ�Ƭ�ο��Լ򵥵��໥���ǡ����ӻ����͸���Ȼ�ϡ�
    �����ࡢ��դ������ɫ��Ͻ׶��ǹ̶����ܽ׶Ρ���Щ�׶�������ʹ�ò����������ǵĲ����������ǵĹ�����ʽ��Ԥ����ġ�
    �����׶���programable�ɱ�̵ģ����Խ��Լ��Ĵ����ϴ����Կ���׼ȷӦ���Զ�����������磬��������ʹ��Ƭ����ɫ����ʵ�ִ����������������׷�������κ����ݡ���Щ����ͬʱ�����GPU�ں������У��Բ��д������������綥���ƬԪ
    �����֮ǰʹ�ù�OpenGL��Direct3D�ȽϾɵ�API����ô����ϰ����ʹ��glBlendFunc��OMSetBlendState�ȵ�����������κι������á�vk�е�ͼ�ι��߼�������ȫ���ɱ�ģ�������Ҫ������ɫ�����󶨲�ͬ��֡����������Ļ�Ϻ����������ͷ��ʼ���´������ߡ�ȱ���������봴�������ߣ���Щ���߱�ʾҪ����Ⱦ������ʹ�õ����в�ͬ״̬��ϡ�Ȼ������Ϊ�����ڹ�����ִ�е����в�������Ԥ��֪���ģ���������������Ը��õĽ����Ż���
    һЩ�ɱ�̽׶��ǿ�ѡ�ġ��������ֻ�ǻ��Ƽ򵥵ļ���ͼ�Σ�����Խ�������ϸ�ֺͼ���ͼ�ν׶Ρ������ֻ�����ֵ����Ȥ������Խ���Ƭ����ɫ���������������Ӱ��ͼ�����á�*/
    /*�����ڵ�API��ͬ��vk�е���ɫ������������ֽ����ʽָ������������glsl��hlsl�����Ŀɶ����﷨�������ֽ����ʽ��ΪSPIR-V��ּ����vk��opgl��ǰʹ�á�����һ�ֿ����ڱ�дͼ�κͼ�����ɫ���ĸ�ʽ
    ʹ���ֽ����ʽ����������GPU��Ӧ�̱�д�Ľ���ɫ������ת��Ϊ��������ı������ĸ�����Ҫ�͵öࡣ��ȥ���棬����glsl���ڵ�����ɶ��﷨��һЩGPU��Ӧ�̶Ա�׼�Ľ����൱�����ʹ�����ǵ�GPU��д����Ҫ����ɫ��������������Ӳ�����������ݵķ��ա�
    Ȼ������Ҳ����Ҫ�ֶ���д�ֽ��룬Khronos�����˱������ɽ�glsl����ΪSPIR-V���˱�����ּ����֤��ɫ�������Ƿ���ȫ���ϱ�׼��������һ��SPIR-V�������ļ���Ҳ���Խ��˱�������Ϊ�����������ʱ����SPIR-V�����Ǳ��̳̲�����������Ȼ����ͨ��glslangValidator.exeֱ��ʹ�ñ������������ǽ�glslc.exe��Ϊ��Googleʹ�á�glslc���ŵ�����ʹ����ͨ������(��gcc��clang)��ͬ�Ĳ�����ʽ��������һЩ����Ĺ��ܣ���include�����Ƕ��Ѿ���������vkSDK��
    glsl��һ�־���c����﷨����ɫ�����ԡ�������д�ĳ�����һ��mainΪÿ��������õĺ�����glsl��ʹ����������ͷ���ֵ��Ϊ���������ʹ��ȫ�ֱ������������������������԰������������ͼ���̵Ĺ��ܣ��������������;����Դ�����������������������������Χ�ķ��������ĺ�����vecʹ�ñ�ʾԪ�����������ֵ����������͡�����3Dλ�ý��洢��vec3������ͨ������.x���ʷ�������Ҳ����ͬʱ�Ӷ���������һ����������������ʽvec3(1.0,2.0,2.0).xy�ᵼ��vec2�������Ĺ��캯��Ҳ���Բ�����������ͱ���ֵ����ϣ�����vec3������������vec3(vec2(1.0,2.0),3.0)*/
    //����ͼ�ι���
    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("Shaders/vert.spv");//������ɫ�����ֽ���
        auto fragShaderCode = readFile("Shaders/frag.spv");
        /*��ɫ��ģ��ֻ����ɫ���ֽ�������ж���ĺ�����һ������װ�����ڴ���ͼ�ι���֮ǰ�������SPIR-V�ֽ������������������GPUִ�С�����һ�����ߴ�����ɾͿ����ٴ�������ɫ��ģ�飬�����Ϊʲô��createGraphicsPipeline�н�������Ϊ�ֲ��������������Ա��ԭ��*/
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        //��ɫ���׶δ�����Ҫʵ��ʹ����ɫ����Ҫͨ���ṹ�彫��ɫ��������ض��Ĺ��߽׶Σ���Ϊʵ�ʹ��ߴ������̵�һ����
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;//����vk���shader���ĸ��׶�ʹ��
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;//ָ�������������ɫ��ģ��
        vertShaderStageInfo.pName = "main";//ָ���������
        /*���ﻹ��һ����ѡ��ԱpSpecializationInfo�����ﲻ�á�������ָ����ɫ���ĳ�����ֵ������ʹ�õ�����ɫ��ģ��ͨ��Ϊ����ʹ�õĳ���ָ����ͬ��ֵ�������ڴ�������ʱ��������Ϊ���������Ⱦʱʹ�ñ���������ɫ������Ч����Ϊ���������Խ����Ż�����������if�ȡ�Ĭ��nulptr*/
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};//ƬԪ
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;//����vk���shader���ĸ��׶�ʹ��
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;//ָ�������������ɫ��ģ��
        fragShaderStageInfo.pName = "main";//���Խ����fragShader��ϵ�һ��shaderModule�У���ʹ�ò�ͬ����ڵ����������ǵ���Ϊ���������Ǽ��ʹ�ñ�׼main

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };

        //�Ͼɵ�ͼ��APIΪͼ�ι��ߵĴ�����׶��ṩ��Ĭ�Ͻ׶Ρ���vk�б�����ȷ���������״̬����Ϊ�������決�����ɱ�Ĺ���״̬������
        
        //��������
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};//�ýṹ�������˽����ݸ�������ɫ���Ķ������ݵĸ�ʽ
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;//�󶨣�����֮��ļ���Լ��������𶥵㻹����ʵ��
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;//�������������ݸ�������ɫ�������Ե����ͣ����ĸ��󶨼��������Լ����ĸ�ƫ����
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
        //��Ϊ����֮���ڶ�����ɫ���жԶ������ݽ���Ӳ���룬������ʱû��Ҫ���صĶ�������
        //pVertexBindingDescriptions��pVertex AttributeDescriptions��Աָ��һ���ṹ���飬�����������˼��ض������ݵ�������ϸ��Ϣ�����˽ṹ��ӵ�shaderStages��������createGraphicsPipeline�����С�

        //����װ�䣬������
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;//���Ӷ������ʲô���ļ���ͼ�Ρ�VK_PRIMITIVE_TOPOLOGY_POINT_LIST: ���Զ���ĵ㣻VK_PRIMITIVE_TOPOLOGY_LINE_LIST-ÿ2��������ߣ����ظ�ʹ�ã�VK_PRIMITIVE_TOPOLOGY_LINE_STRIP-ÿ�еĽ�������������һ�еĿ�ʼ���㣻VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST-����ÿ 3 ������������Σ����ظ�ʹ�ã�VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP-ÿ�������εĵڶ����͵���������������һ�������ε�ǰ�������㡣�������������
        inputAssembly.primitiveRestartEnable = VK_FALSE;//�Ƿ�Ӧ������ͼԪ����
        //ͨ���������ǰ�����˳��Ӷ��㻺�������صģ���ʹ��Ԫ�ػ�������������ָ��Ҫʹ�õ���������������ִ���Ż��������ö��㡣�����primitiveRestartEnable��Ա����ΪVK_TRUE�������ͨ��ʹ����������0xFFFF��0xFFFFFF����_STRIP����ģʽ�зָ�ֱ�ߺ������Ρ�
        
        //�ӿ�״̬
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;//û��ʹ�ö�̬״̬����pipeline������viewport��scissorRectangle������ø�pipeline��viewport��scissorRectangle���ɱ䣬�����κθ��Ķ�Ҫ���´���pipeline
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        //����������ö�������ĳЩ�Կ���ʹ�ö��viewport��scissorRectangle��ͨ���ṹ���������á�����ʹ�ö����Ҫ����GPU����(�����߼��豸����)

        //�ӿںͼ������ӿڻ��������������������Ⱦ����֡����������
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;//�������Ĵ�С����ͼ������봰�ڵ�WIDTH��HEIGHT��ͬ��������ͼ���Ժ�����֡���������������Ӧ�ü�����ǵĴ�С.
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;//minDepth��maxDepthֵָ������֡�����������ֵ��Χ����Щֵ������[0.0f, 1.0f]��Χ�ڣ��� minDepth���ܸ���maxDepth�������û�����κ��ر�����飬��ô��Ӧ�ü��0.0fand�ı�׼ֵ1.0f��
        viewport.maxDepth = 1.0f;
        //�ӿڶ����˴�ͼ��֡��������ת���������о��ζ�����ʵ�ʴ洢���ص����򡣲��о���֮����κ����ض�������դ�������������ǵĹ������ƹ�����
        VkRect2D scissor{};//������������֡��������Ҫָ��һ����ȫ�������Ĳ��о���
        scissor.offset = { 0,0 };
        scissor.extent = swapChainExtent;
        //�ӿںͼ��þ��μȿ���ָ��Ϊ���ߵľ�̬���֣�Ҳ����ָ��Ϊ��������еĶ�̬״̬������Ȼǰ�߸���������״̬����ʹ�ӿںͼ���״̬��̬��ͨ���ܷ��㣬��Ϊ��Ϊ���ṩ�˸��������ԡ���ܳ���������ʵ�ֶ����Դ������ֶ�̬״̬�����ή�����ܡ�
        
        //��դ��������դ������ȡvertShader�ж����γɵļ���ͼ�Σ�����ת��ΪƬԪ��fragmentShader��ɫ������ִ����Ȳ��ԡ������޳��Ͳü����ԣ��������� Ϊ��������������λ��߽���Ե�� ƬԪ(�߿���Ⱦ)
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;//�������ΪVK_TRUE���򽫳�����ƽ���Զƽ���ƬԪ���������ϣ������Ƕ������ǡ�����һЩ��������º����� ������Ӱ��ͼ��ʹ������Ҫ����GPU���ܡ�
        rasterizer.rasterizerDiscardEnable = VK_FALSE;//�������ΪVK_TRUE���򼸺�ͼ����Զ����ͨ����դ���׶�
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//ȷ�����Ϊ����������ƬԪ������ʹ������ģʽ��VK_POLYGON_MODE_FILL-��Ƭ�������������VK_POLYGON_MODE_LINE-����α߱������ߣ�VK_POLYGON_MODE_POINT-����ζ��㱻����Ϊ�㡣
        rasterizer.lineWidth = 1.0f;//����ƬԪ�����������Ĵ�ϸ��֧�ֵ�����߿�ȡ����Ӳ�����κδ���1.0f���߶���Ҫ����wideLines GPU���ܡ�
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;//ȷ��Ҫʹ�õ����޳����͡����Խ����޳����޳����桢�޳����������
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;//ָ������Ϊ�������Ķ���˳�򣬿�����˳ʱ�����ʱ��
        rasterizer.depthBiasEnable = VK_FALSE;//��ӳ���ֵ�������Ƭ��б��ƫ�����ֵ���������ֵ������ʱ������Ӱ��ͼ�������ﲻ��ʹ������ֻ�轫depthBiasEnable����ΪVK_FALSE��
        rasterizer.depthBiasConstantFactor = 0.0f;//optional
        rasterizer.depthBiasClamp = 0.0f;//optional
        rasterizer.depthBiasSlopeFactor = 0.0f;//

        //���ز�������ִ�п���ݵķ���֮һ��ͨ������դ����ͬһ���صĶ������ε�fragShader��������һ������������Ҫ�����ڱ�Ե��Ҳ�������Եľ��αӰ�����ĵط�����Ϊ���ֻ��һ�������ӳ�䵽һ�����أ�������Ҫ�������fragShader����������Ҫ����GPU����
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional���ö��ز���

        //��Ⱥ�ģ�����
        //���ʹ����Ȼ�ģ�建����������Ҫʹ��VkPipelineDepthStencilStateCreateInfo������Ⱥ�ģ�߲��ԡ��������ڻ�û�У��������ǿ��Լ򵥵ش���һ��nullptr��������ָ������һ���ṹ��ָ�롣���ǽ�����Ȼ���һ������������

        //��ɫ��ϡ�fragShader������ɫ����Ҫ������֡�����������е���ɫ��ϡ�����ת����Ϊ��ɫ��ϣ������ַ���������������Ͼ�ֵ����ֵ�Բ���������ɫ/ʹ�ð�λ������Ͼ�ֵ����ֵ��
        //���������͵Ľṹ��������ɫ��ϡ���һ���ṹ VkPipelineColorBlendAttachmentState����ÿ������֡�����������ã��ڶ����ṹVkPipelineColorBlendStateCreateInfo ����ȫ����ɫ������á������ǵ������У�����ֻ��һ��֡��������
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;//�������ΪVK_FALSE��������fragShader������ɫ��δ���޸ĵ�ͨ��
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        //�ڶ����ṹ����������֡�������Ľṹ����ֵ�����������û�ϳ����������������
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;//Ϊtrueʱ�Զ����õ�һ�ַ���
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional//ָ����λ����
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        
        //��Ȼ���������״̬����Ҫ���決������״̬�У���ʵ���Ͽ��Ը������޵�״̬���������ڻ���ʱ���´������ߡ������ӿڵĴ�С�߿�ͻ�ϳ��������Ҫʹ�ö�̬״̬��������Щ�����Ǳ�����д������дVkPipelineDynamicStateCreateInfo�ṹ
        std::vector<VkDynamicState>dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };
        //��ѡ��̬�ӿںͼ��о���ʱ������ҪΪ����������Ӧ�Ķ�̬״̬��
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
        //�⽫������Щֵ�����ñ����ԣ��ܹ��ڻ���ʱָ�����ݡ����������������ã����Ҷ����ӿںͲü�״̬֮��Ķ������Ǿͣ������決������״̬ʱhi���¸����ӵ�����

        //���߲��֣���������ͳһֵuniform��uniform���ƶ�̬״̬������ȫ��ֵ��������drawFrame��ʱ����������Ըı�shader����Ϊ���������´���shader��ͨ�����ڽ��任���󴫵ݸ�vertShader����fragShader�д�����������������ﴴ��һ���յ�pipelineLayout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional//�ýṹ��ָ����push constants�����ǽ���ֵ̬���ݸ���ɫ������һ�ַ�ʽ�����ǿ��ܻ����Ժ���½��н��ܡ�
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        /*����ӵ�еĶ������ͣ�
        ��ɫ���׶Σ�����ͼ����߿ɱ�̽׶ι��ܵ���ɫ��ģ�飻
        �̶�����״̬��������߹̶����ܽ׶ε����нṹ���������������դ�������ӿں���ɫ��ϣ�
        ���߲��֣���ɫ�����õ�ͳһ������ֵ�������ڻ���ʱ���£�
        ��Ⱦͨ�������߽׶����õĸ��������÷���
        ��Щ��϶�����ͼ����ߵĹ���*/
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        //�̶����ܽ׶�
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        //���߲��֣�����vk��������ǽṹ��ָ��
        pipelineInfo.layout = pipelineLayout;
        //��Ⱦ���ߵ����ú���subpass������
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        //vk������ͨ�����й��������������µ�ͼ�ι��ߡ�ԭ���ǣ������������й��߾�����๲ͬ����ʱ���������ߵĳɱ��ϵͣ����ҿ��Ը����������ͬһ�����ߵĹ���֮���л���
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;//ָ�����й��ߵľ��
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {//�ڶ�������������һ����ѡVkPipelineCache����pipelineCache�����ڴ洢������ ����vkCreateGraphicPipelines���� ֮��Ĺ��ߴ�����ص����ݡ�������浽�ļ��У��򻹿������ڿ����ִ��֮������ݡ���ʹ���Ժ���������ӿ���ߴ����ٶ�
            throw std::runtime_error("failed to create graphic pipeline!");
        }

        //���ɾ����ɫ��ģ��
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }
    //����֡������������Ⱦ���̴����ڼ�ָ���ĸ�����װ��һ��VkFramebuffer�������󶨡�֡������������������VkImageView��ʾ�����Ķ��󡣱���Ϊ����ʹ�õ�ͼ��ȡ���ڼ���������ʾ��ͼ��ʱ���������ص�ͼ�����Ա���Ϊ�������е�����ͼ�񴴽�һ��֡����������ʹ����drawFrameʱ��������ͼ�����Ӧ��֡������
    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());//���������Ĵ�С����������֡������

        //����ͼ����ͼ�����д���֡������
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = { swapChainImageViews[i] };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;//ָ��֡���������ĸ�renderPass���ݡ�ֻ�ܽ�֡�������������ݵ���Ⱦ����һ��ʹ�ã���������ʹ����ͬ���������͵ĸ���
            framebufferInfo.attachmentCount = 1;//attachmentCount��pAttachments����ָ��Ӧ�󶨵���Ⱦ����pAttachment��������Ӧ����������VkImageView����
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;//ָͼ�������еĲ��������ǵĽ�����ͼ���ǵ���ͼ�����Բ���Ϊ1

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }
    //vk�е��������ͼ�������ڴ洫�䣬����ֱ��ʹ�ú�������ִ�еġ�������������������м�¼Ҫִ�е����в������ô��ǵ�׼������vk��ʲôʱ�����������һ���ύ��vk���Ը���Ч�ش�����Щ������Ϊ���Ƕ�����һ��ʹ�á����� �����Ҫ �����ڶ���߳��н��������¼
    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;//��������������ܵ�flag��VK_COMMAND_POOL_CREATE_TRANSIENT_BIT-��ʾ����������������������¼�¼�����ܻ�ı��ڴ������Ϊ����VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT-���������¼�¼������������û�д˱�־�����Ƕ�����һ�����á����ｫÿ֡��¼һ���������������ϣ���ܹ����ò����¼�¼���������������
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        //�������ͨ���������ύ���豸����֮һ��ִ�С�ÿ�������ֻ�ܷ����ڵ�һ���Ͷ������ύ��������������ǽ���¼��ͼ��������ѡ��ͼ�ζ��е�ԭ��
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    //��cmdPool����cmdBuffer
    void createCommandBuffers() {
        /*FrameInFlight����Ⱦѭ���и�ȱ�ݣ���Ҫ�ȴ�ǰһ֡��ɲ��ܿ�ʼ��һ֡��Ⱦ����ᵼ����������Ҫ�Ŀ���
        ��������Ƕ��֡ͬʱ���У�������һ֡����Ⱦ��������һ֡�ļ�¼���������������Ⱦ�ڼ���ʺ��޸ĵ��κ���Դ���븺�������Ҫ���cmdBuffer��semaphore��fence��
        ����ͬʱ�����֡��Ϊ2����Ϊ��ϣ��CPUԶԶ������GPU����2֡�ڷ����У�CPU��GPU����ͬʱ������Ե��������CPU��ǰ������ȵ�GPU�����Ⱦ�����ύ���๤�������3֡�������CPU���ܻ�����GPU�Ӷ�����֡�ӳ١�
        ÿ��֡��Ӧ�����Լ���cmdBuffer��semaphore��fence*/
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);//�޸�cmdBuffers�Ĵ�С
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;//ָ�������
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;//ָ����������������������������Ǹ������������VK_COMMAND_BUFFER_LEVEL_PRIMARY-�����ύ������ִ�У������ܴ���������������á���VK_COMMAND_BUFFER_LEVEL_SECONDARY-����ֱ���ύ�������Դ�������������á�
        allocInfo.commandBufferCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;//ָ��Ҫ����Ļ���������
        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    //���������¼����������Ҫִ�е�����д����������ĺ�����ʹ�õ�VkCommandBuffer����Ϊ�������룬�Լ�Ҫд��ĵ�ǰswapchainӳ���������
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //beginInfo.flags = 0;//ָ�����ʹ���������������ʹ������ֵ��VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT-�����������ִ��һ�κ��������¼�¼��VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT-����һ�������������������ȫ�ڵ�����Ⱦ�����У�VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT-���������������Ҳ�Ѿ��ȴ�ִ��ʱ�����ύ��
        //beginInfo.pInheritanceInfo = nullptr;//���븨�����������ء�ָ���ӵ�������������̳е�״̬��
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {//�����������Ѿ���¼��һ�Σ���ô����vkBeginCommandBuffer����ʽ���������Ժ󲻿��ܽ�����ӵ�������
            throw std::runtime_error("failed to begin recoding command buffer!");
        }

        //���ƴӿ�ʼ��Ⱦͨ����ʼ
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;//��Ⱦ���̱���
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];//Ҫ�󶨵ĸ���������Ϊÿ����ָ��Ϊ��ɫ�����Ľ�����ͼ�񴴽���һ��֡����������ˣ�������ҪΪҪ���ƵĽ�����ͼ���֡��������ʹ�ô����imageIndex����������Ϊ��ǰ������ͼ��ѡ����ȷ��֡������
        //ָ����Ⱦ����Ĵ�С����Ⱦ��������ɫ�����غʹ洢����������λ�á�������֮������ؽ�����δ�����ֵ����Ӧ���븽���Ĵ�С��ƥ���Ի���������
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = swapChainExtent;
        //�������������������VK_ATTACHMENT_LOAD_OP_clear�����ֵ�����ǽ���������ɫ�����ļ��ز����������Ѿ�����������ɫ����Ϊ100%��͸���ȵĺ�ɫ
        VkClearValue clearColor = { {{0.0f,0.0f,0.0f,1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        //��ʼ��Ⱦ�����ˡ����м�¼����ĺ�������vkCmdǰ׺�����Ƕ�����void
        //ÿ������ĵ�һ������ʼ���Ǽ�¼���������������ڶ�������ָ���ո��ṩ����Ⱦ���ߵ���Ϣ�����һ��������������ṩ��Ⱦ�����еĻ�ͼ���������������ֵ��VK_SUBPASS_CONTENTS_INLINE-��Ⱦͨ�����Ƕ�뵽��������������в��Ҳ���ִ�и������������VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS-��Ⱦͨ������Ӹ����������ִ�С�
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            //��ͼ�ι��ߡ��ڶ�������ָ�����߶�����ͼ�λ��Ǽ�����ߡ������Ѿ�����vk��ͼ�ι�����ִ����Щ�����Լ���fragShader��ʹ���ĸ�����
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
            //�����ڹ̶��������ᵽ�ģ�����Ϊ�������ָ�����ӿںͲ���״̬Ϊ��̬�ġ�������Ҫ�ڷ�����ͼ����֮ǰ��������������������У�
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
            
            //����׼�����������εĻ�������
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);//ʵ�ʵ�vkCmdDraw���ܻ�ͷ��β����Ϊ��ǰָ����������Ϣ��������������⣬�������²�����vertexCount����ʹ����û�ж��㻺���������Ӽ����Ͻ���������Ȼ��Ҫ���� 3 �����㡣��instanceCount-����ʵ����Ⱦ������㲻����������ʹ��1��firstVertex-�������㻺������ƫ����������_VertexIndex����Сֵgl��firstInstance-����ʵ����Ⱦ��ƫ����������gl_InstanceIndex����Сֵ��

        //������Ⱦ����
        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
    /*ͬ����
    vk�ĺ������������GPU�ϵ�ִ��ͬ������ȷ�ġ�����˳��ȡ����ʹ�ò�ͬ��ͬ��ԭ�������壬��Щԭ�����������������ϣ�����е�˳���������GPU��ִ�й�����vkAPI�������첽�ģ��������ڲ������֮ǰ����
    �ڱ����У���Ҫ��ȷ��������GPU�ϵ�����¼����ӽ������л�ȡͼ��ִ���ڻ�ȡ��ͼ���ϻ��Ƶ��������ͼ����ֵ���Ļ�Ͻ��г��ֲ����䷵�ص���������
    ��Щ�¼���ʹ�õ��������������������Ҷ����첽ִ�еġ����������ڲ���ʵ�����ǰ���أ�ִ��˳��Ҳδ���塣���ǲ��ҵģ���Ϊÿ������������֮ǰ������ɡ�����������Ҫ̽������ʹ����Щԭ����ʵ�����������
    �ź������ź��������ڶ��в���֮�����˳�򡣶��в����������ύ�����еĹ���������������������У�Ҳ�������ں����С����е�ʾ����ͼ�ζ��к���ʾ���С��ź������ڶ�ͬһ�����ڵĹ�����������
        vk�������ź����������ƺ�ʱ���ߡ�����ֻʹ�ö������ź���������ʱ�����ź���
        �ź��������źŻ������źŵġ��������źſ�ʼ��ʹ���ź����Զ��н�������ķ����ǣ���һ�����в������ṩ��"�ź�"�ź�����ͬ���ź���������һ�������������ṩ��ͬ��"�ȴ�"�ź���
        ���磺�������ź���S�Ͷ��в���A��B�������밴˳��ִ�����ǡ����������vk���ǣ�������A���ִ��ʱ������"��"�ź���S�����źţ�������B�����ź���S��"�ȴ�"Ȼ���ٿ�ʼִ�С�������A����ʱ���ź���S���������źţ�������Bֱ��S������"�ź�"�ſ�ʼ������B��ʼִ�к��ź���S�Զ�����Ϊδǩ���������ٴ�ʹ��
        ע�⣺�ȴ�ֻ������GPU�ϣ�CPU�������в���������Ϊ����CPU�ȴ�����Ҫʹ��һ����ͬ��ͬ��ԭ��
    Χ����Χ�������Ƶ���;������ͬ��ִ�У�����������CPU(Ҳ��Ϊ����)������ִ�С��򵥵�˵�����������Ҫ֪��GPUʲôʱ�������ʲô����ʹ��һ��Χ��
        ���ź������ƣ�Χ��Ҳ�����л����ź�״̬�����ۺ�ʱ�ύҪִ�еĹ������������ڸù����ϸ���һ��Χ�������������ʱ��Χ�������źš�Ȼ�����ǿ����������ȴ�Χ�������źţ�ȷ������������ǰ��ɹ���
        ���磺�����������Ѿ���GPU������˹���������Ҫ��ͼ���GPU���䵽������Ȼ���ڴ汣�浽�ļ���������ִ�д����Χ��F���������A�������ύ����Χ��F���������A��Ȼ������֪ͨ�����ȴ�F�����źš���ᵼ������������֪���������A���ִ�С����Ե��ڴ洫�����ʱ�����԰�ȫ�����������ļ����浽����
        һ����˵�����Ǳ�Ҫ������ò�Ҫ��ֹ������������ΪGPU�������ṩ���õĹ�������Χ���ϵȴ��źŲ�����һ�����õĹ������������Ǹ�ϲ�����ź�����������δ�漰��ͬ��ԭ����ͬ������
        �����ֶ�����Χ����ʹ��ָ�δǩ��״̬����ΪΧ�����ڿ���������ִ�У������������Ծ�����ʱ����Χ�����෴���ź���������GPU������������漰����
    ��֮���ź�������ָ��GPU�ϲ�����ִ��˳�򣬶�Χ�����ڱ���CPU��GPU�˴�ͬ����
    ѡ��ʲô��������ͬ��ԭ������ã������������ط����Է����Ӧ��ͬ���������������͵ȴ�ǰһ֡��ɡ����ǽ��ź������ڽ�������������Ϊ���Ƿ�����GPU�ϣ����ǲ����������ȴ������ڵȴ�ǰһ֡�������ʹ��Χ������Ϊ�������������ȴ����������ǲ���һ�λ��ƶ��֡����Ϊÿһ֡�����¼�¼��������������ڵ�ǰִ֡�����ǰ���޷�����һ֡�Ĺ�����¼����������У���Ϊ���ǲ�����GPUʹ������������ʱ�����䵱ǰ���ݡ�*/

    //����ͬ������
    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{}; //�ź���
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};//Χ��
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        //�����ź�����Χ��
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }
    //��ͼ
    void drawFrame() {
        /*�ſ�vk����Ⱦ֡�������裺�ȴ���һ֡���->�ӽ������л�ȡͼ��->��¼һ�����������Ƶ���ͼ���ϵ��������->�ύ��¼���������->��ʾ������ͼ��*/
        
        //��֡��ʼʱ�ȴ�ǰһ֡�������Ա�����������ź����ɹ�ʹ�á�Ϊ�˵��õȴ�Χ��vkWaitForFences���ú�������һ��Χ�����飬���������ϵȴ��κλ�����Χ���ڷ���֮ǰ�����źš�VK_TRUE��ʾ����Ҫ�ȴ����е�դ�������ڵ���դ���������û��ϵ���ú������и���ʱ��������������Ϊ64λ�޷������������ֵUINT64_MAX������Ч�ؽ����˳�ʱ��
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        //�ȴ�֮���ֶ���Χ������Ϊ���ź�״̬
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        //�ڼ���֮ǰ�����ǵ�������и�С���⡣�ڵ�һ֡���ǵ���drawFrame()������ȴ�inFlightFence�����źţ�������ֻ����һ֡�����Ⱦ�󷢳��źţ��������ǵ�һ֡������û����ǰ��֡���Է���Χ���źţ�����vkWaitForFences()�����ڵ�������
        //API������һ������Ľ�����������ź�״̬�´���Χ�����Ա��һ�ε���vkWaitForFences()�������أ���Ϊդ���Ѿ������źš�
        
        //�ӽ�������ȡͼ�񡣽�������һ����չ���ܣ��������Ǳ���ʹ��vk*KHR����Լ���ĺ���
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);//ǰ�����������߼��豸�ͽ�����������������ָ��ͼ����õĳ�ʱʱ��(��λ����)��ʹ��64λ�޷������������ֵ��ζ�Ž�ֹ��ʱ����������������ָ������ʾ����ʹ��ͼ�����ʱҪ���䷢���źŵ�ͬ���������ǿ��Կ�ʼ���Ƶ�ʱ��㣬����ָ���ź���Χ����both��Ϊ��ʹ��imageAvailableSemaphore�����һ������ָ��һ������������������õĽ�����ӳ�����������������swapChainImages�����е�VkImage������ʹ�ø���������VkFrameBuffer

        //��¼���������ʹ��imageIndexָ��Ҫʹ�õĽ�����ͼ�����ڿ���ʹ�����������
        vkResetCommandBuffer(commandBuffers[currentFrame], 0);//������������ô˺�����ȷ�����ܹ�����¼��
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        //�ύ�������������������¼������������Ϳ����ύ��
        VkSubmitInfo submitInfo{};//�����ύ��ͬ����ͨ���˽ṹ���еĲ��������õ�
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        //ǰ��������ָ����ִ�п�ʼǰ�ȴ���Щ�ź������Լ��ڹ��ߵ��ĸ��׶εȴ�������ϣ���ȵ�ͼ�����ʱ������д����ɫ����������Ҫָ��д����ɫ������ͼ�ι��ߵĽ׶Ρ�����ζ�������ϣ���ͼ���в����õ�����£�ʵ�ֿ��Կ�ʼִ�ж�����ɫ���ȡ�waitStages�����е�ÿ����Ŀ��Ӧ��pWaitSemaphores�о�����ͬ�������ź�����
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        //����������������ָ��ʵ���ύִ�е��������������ֻ��Ҫ�ύ����ӵ�еĵ����������
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        //signalSemaphoreCount��pSignalSemaphores����ָ������������ִ�к�Ҫ�����źŵ��ź���������ʹ��renderFinishedSemaphore����ɴ�Ŀ��
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        //����ʹ��vkQueueSubmit����������ύ��ͼ�ζ��С����������ش�ö�ʱ���ú�������VkSubmitInfo�ṹ��������ΪЧ�ʲ��������һ������������һ����ѡΧ����������������ִ��ʱ���ᷢ���źš���������֪����ʱ���԰�ȫ�����������������������ϣ����FlightFence���ṩ����������һ֡�У�CPU���ȴ�������������ִ�У�Ȼ���������¼������
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer!");
        }

        //������ύ�ؽ�������ʹ��������ʾ����Ļ�ϡ�
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        //ǰ��������ָ���ڳ���֮ǰҪ�ȴ���Щ�ź���������vkSubmitInfo������������ȴ�����������ִ�У���˻��������Σ����ǻ�ȡ���������źŵ��ź������ȴ����ǣ��������ʹ��signalSemaphores
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        //����������������ָ����ͼ����ֵ��������Լ�ÿ����������ͼ���������⼸������һ��
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        //���һ����ѡ�����������ʾ�ɹ���������ָ��һ��vkResultֵ�����ÿ�������Ľ����������ֻʹ�õ�������������û��Ҫ����Ϊ���Լ򵥵�ʹ�õ�ǰ�����ķ���ֵ
        presentInfo.pResults = nullptr;
        //vkQueuePresentKHR�����ύ�򽻻�������ͼ�������
        vkQueuePresentKHR(presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;//ʹ��ģ%�������ȷ��ѭ��
    }
    //�ڽ����봫�ݸ�����֮ǰ�����뽫����װ��һ��VKShaderModule������
    VkShaderModule createShaderModule(const std::vector<char>& code) {//�ú�����ʹ���ֽ��� ��Ϊ�����Ļ������������д���һ��VKShaderModule�����������洢���ļ��У�����߻�������ڿ��ε���vkCreateGraphicsPipelines���������ִ�д洢����������ߴ�����ص����ݡ���ʹ���Ժ���������ӿ���ߴ����ٶ�
        //������ɫ��ģ��ܼ򵥣�ֻ��Ҫ���ֽ���ͳ���ָ��һ��ָ�򻺳�����ָ��
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        /*һ���������ֽ���Ĵ�С���ֽ�Ϊ��λָ�������ֽ���ָ����uint32_tָ�������charָ�롣��ˣ�������Ҫ��ָ��ת�� reinterpret_castΪ������ʾ������ִ��������ǿ��ת��ʱ��������Ҫȷ����������uint32_t. ���˵��ǣ����ݴ洢��std::vectorĬ�Ϸ������Ѿ�ȷ������������������Ҫ���λ�á�*/
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderMoudle;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderMoudle) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderMoudle;
    }
    //Ϊ������ѡ����ѱ����ʽ����
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {//ÿ��VkSurfaceFormatKHR������һ��format��һ��colorSpace����ʽָ����ɫͨ��������(����VK_FORMAT_B8G8R8A8_SRGB��ζ����8λ�޷���������˳�����BGR��alphaͨ����ÿ�����ع�32λ)ɫ�ʿռ�ָʾ�Ƿ�֧��srgb��ɫ�ռ䣬�����Ƿ�ʹ��VK_color_space_SRGB_NONLINEAR_KHR��־
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {//��ɫ�ռ価��ʹ��srgb������������ȷ�ĸ�֪��ɫ����Ҳ��ͼ��ı�׼��ɫ�ռ�
                return availableFormat;
            }
        }
        return availableFormats[0];//�����Ҳʧ���ˣ����Ը������ǵĳ̶ȶԿ��ø�ʽ���������������µ�һ�־Ϳ�����
    }
    //Ϊ������ѡ�������ʾģʽ����
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        /*��ʾģʽ�ǽ���������Ҫ�����ã��������˽�ͼ����ʾ����Ļ��ʵ��������vk�����ֿ��ܵ�ģʽ��
            VK_PRESENT_MODE_IMMEDIATE_KHR��app�ύ��ͼ�������ת�Ƶ���Ļ�ϣ����ܻᵼ��˺�ѡ�
            VK_PRESENT_MODE_FIFO_KHR����������һ�����У�����ʾ��ˢ��ʱ����ʾ���Ӷ��е�ǰ���ȡͼ�񣬳�����Ⱦ��ͼ����뵽���еĺ��档���������������������ȴ��������ִ���Ϸ�еĴ�ֱͬ����Ϊ���ơ�ˢ����ʾ����һ�̳�Ϊ����ֱ�հס���
            VK_PRESENT_MODE_FIFO_RELAXED_KHR����ģʽ����Ӧ�ó���ٵ����Ҷ��������һ����ֱ�հ״�Ϊ�յ��������ǰһ��ģʽ��ͬ��ͼ�����յ���ʱ�������䣬�����ǵȴ���һ����ֱ�հס�����ܻᵼ�����Ե�˺�ѡ�
            VK_PRESENT_MODE_MAILBOX_KHR: ���ǵڶ���ģʽ����һ�ֱ��塣��������ʱ��������Ӧ�ó��򣬶��ǽ����Ŷӵ�ͼ��򵥵��滻Ϊ���µ�ͼ�񡣴�ģʽ�����ھ����ܿ����Ⱦ֡��ͬʱ�Ա���˺�ѣ����׼��ֱͬ����ȣ��ӳ�������١�������׳Ƶġ����ػ��塱����Ȼ���������������岢��һ����ζ��֡���ǽ����ġ�*/
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {//�����ػ�������ػ��壬���ܾ�˫�ػ���
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    //Ϊ������ѡ����ѽ�����Χ����
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        //������Χ�ǽ�����ͼ��ķֱ��ʣ������������ڻ��ƵĴ��ڵķֱ��ʡ��ֱ��ʷ�Χ��VkSurfaceCapabilitiesKHR�ṹ�ж���
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
    //��ѯ������֧��
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);//���湦�ܣ��������ĺ������
        //��ѯ֧�ֵı����ʽ
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);//�����ʽ
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        //��ѯ֧�ֵ���ʾģʽ
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }
    //�������Կ��ʺϹ���
    bool isDeviceSuitable(VkPhysicalDevice device) {
        //VkPhysicalDeviceProperties deviceProperties;
        //VkPhysicalDeviceFeatures deviceFeatures;
        //vkGetPhysicalDeviceProperties(device, &deviceProperties);//������ơ����͡�֧�ֵ�vk�汾���豸��������
        //vkGetPhysicalDeviceFeatures(device, &deviceFeatures);//��ѯ������ѹ����64λ�������Ͷ��ӿ���Ⱦ(VR���)�ȿ�ѡ���ܵ�֧��
        
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        //��ѯ������֧��
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }
    //����豸��չ֧��
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties>availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string>requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());//��һ���ַ�������ʾδȷ�ϵ�������չ

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    //vk���в������ӻ�ͼ���ϴ�����Ҫ�������ύ������
    //������Ҫ�����ж����塣����豸֧����Щ�������Լ�������һ��֧����ʹ�õ�����
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);//�����豸�������б�
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        //��VkQueueFamilyProperties�ṹ������йض���ϵ�е�һЩ��ϸ��Ϣ������֧�ֵĲ��������Լ����ڸ�ϵ�д����Ķ�������
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {//����ÿһ��������
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);//�����ܹ����ֵ����ڱ���Ķ�����(�����豸�����������������ڱ�����)
            if (presentSupport) {
                indices.presentFamily = i;//�鵽���ܳ��ֵ�surface��queueFamily�����������浽QueueFamilyIndices
            }
            if (indices.isComplete()) break;
            i++;
        }
        return indices;
    }
    //������֤���Ƿ����� ���ش���messenger�������չ�б�Ĭ����֤��Ὣ������Ϣ��ӡ����Ҳ���ڳ�������ʽ�ص������� ��Ϊ�еĴ�������
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*>extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);//ʹ��VK_EXT_debug_utils��չ���ô��лص��ĵ�����ʹ���������������ַ���VK_EXT_debug_utils
        }
        return extensions;
    }
    //��������������֤ͼ���Ƿ񶼿���
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);//�г����п�����֤��
        std::vector<VkLayerProperties>availableLayers(layerCount);//�����洢������֤��
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        //���Ҫ��������֤���Ƿ����(availableLayers�Ƿ��������validationLayers)
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
    //���ļ��м��ض��������ݣ���ָ���ļ��ж�ȡ�����ֽڲ������Ƿ��ص�std::vector������ֽ�������
    static std::vector<char>readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);//ate:���ļ�ĩβ��ʼ��ȡ��binary:���ļ���ȡΪ�������ļ�(�����ı�ת��)
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();//��ĩβ��ʼ��ȡ�ĺô��ǿ���ʹ�ö�ȡλ����ȷ���ļ���С�����仺����
        std::vector<char>buffer(fileSize);
        //֮��ص��ļ��Ŀ�ͷ��һ���Զ�ȡ�����ֽڣ�
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        //�ر��ļ��������ֽ�
        file.close();
        return buffer;
    }
    //validation��Ϣ�ص�������ʹ��PFN_vkDebugUtilsMessengerCallbackEXTԭ�����һ����ΪdebugCallback�ľ�̬��Ա������
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(//VKAPI_ATTR��VKAPI_CALLȷ������ǩ����ȷ�Թ�vk���á�
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,//ָ����Ϣ�������ԡ��������±�־֮һ��VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT-�����Ϣ��VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT-��Ϣ����Ϣ���紴����Դ��VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT-������Ϊ����Ϣ,��һ���Ǵ���,���ܿ���������Ӧ�ó����еĴ���VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT-�й���Ч�ҿ��ܵ��±�������Ϊ����Ϣ��
        VkDebugUtilsMessageTypeFlagsEXT messageType,//���Ծ�������ֵ��VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT-������һЩ����������޹ص��¼���VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT-������Υ���淶��������ܴ��ڴ�������飻VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT-Vulkan ��Ǳ�ڷ����ʹ��
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,//�����ص���Ϣ��ϸ��Ϣ��VkDebugUtilsMessengerCallbackDataEXT�ṹ�壬��������Ҫ���ǣ�pMessage-������Ϣ��Ϊһ���Կ��ַ���β���ַ�����pObjects-����Ϣ��ص�vk���������飻objectCount-�����еĶ�������
        void* pUserData) {//��pUserData��������һ���ڻص������ڼ�ָ����ָ�룬�����������Լ������ݴ��ݸ�����
        std::cerr << "***ValidationLayer: " << pCallbackData->pMessage << std::endl;//�����validation��Ϣ��ӡ
        return VK_FALSE;//�ص�����һ������ֵ���Ƿ���ֹ������֤����Ϣ��vk���á��������true������ý���ֹ������VK_ERROR_VALIDATION_FAILED_EXT������ͨ�������ڲ�����֤�㱾�����Ӧʼ�շ���VK_FALSE��
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