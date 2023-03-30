#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <math.hpp>

#define SHADER_PATH "./debug/"

void Assert(uint64_t expr, const char* msg)
{
    if(expr) return;
    MessageBoxExA(
            NULL,
            msg,
            "FAILED ASSERT",
            MB_OK,
            0);
    DebugBreak();
    ExitProcess(-1);
}

#define STMT(S) do { S;} while(0)
#define STRINGIFY(S) #S
#define ASSERT(EXPR) STMT(Assert((u64)(EXPR), STRINGIFY(EXPR)))
#define ARR_LEN(A)  (sizeof(A)/sizeof(A[0]))
#define FIND_STRING_IN_AOS(ARR, N, MATCH, MEMBER_NAME) \
    ({\
     i32 result = -1;\
     for(i32 i = 0; i < (N); i++) {\
        if(strcmp(MATCH, ARR[i].MEMBER_NAME) == 0) {\
            result = i;\
            break;\
        }\
     }\
     result;\
     })\

#define VK_ASSERT(EXPR) ASSERT((EXPR) == VK_SUCCESS)
// Usage: PROCNAME is vulkan procedure name without "vk" prefix.
#define VK_DECLARE_PROC(PROCNAME) PFN_vk##PROCNAME PROCNAME
#define VK_GET_IPROC(INSTANCE, PROCNAME) STMT(\
        PROCNAME = (PFN_vk##PROCNAME)vkGetInstanceProcAddr(INSTANCE, "vk" STRINGIFY(PROCNAME));\
        ASSERT(PROCNAME);)
#define VK_GET_DPROC(DEVICE, PROCNAME) STMT(\
        PROCNAME = (PFN_vk##PROCNAME)vkGetDeviceProcAddr(DEVICE, "vk" STRINGIFY(PROCNAME));\
        ASSERT(PROCNAME);)

HWND windowHandle;
bool closeApp       = false;
bool wasResized     = false;
i32 windowWidth     = 1280;
i32 windowHeight    = 720;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CLOSE:
            {
                closeApp = true;
            } break;
        case WM_SIZE:
            {
                windowWidth = LOWORD(lParam);
                windowHeight = HIWORD(lParam);
                wasResized = true;
            } break;
        default: return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void ProcessWindowMessages()
{
    MSG msg = {};
    while(true)
    {
        int ret = PeekMessage(&msg, windowHandle, 0, 0, PM_REMOVE);
        ASSERT(ret >= 0);
        if(!ret) break;
        DispatchMessage(&msg);
    }
}

u64 GetFileSize(const char* path)
{
    HANDLE hFile = CreateFile(
            path,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    ASSERT(hFile != INVALID_HANDLE_VALUE);
    DWORD fSize = ::GetFileSize(hFile, NULL);
    ASSERT(fSize != INVALID_FILE_SIZE);
    CloseHandle(hFile);
    return fSize;
}

u64 ReadFileAsBinary(const char* path, u64 sizeToRead, u8* outBuffer)
{
    HANDLE hFile = CreateFile(
            path,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    ASSERT(hFile != INVALID_HANDLE_VALUE);
    DWORD bytesRead = 0;
    BOOL ret = ::ReadFile(
            hFile,
            outBuffer,
            sizeToRead,
            &bytesRead,
            NULL);
    ASSERT(ret);

    CloseHandle(hFile);
    return (u64)bytesRead;
}

// Vulkan validation layer callback
VKAPI_ATTR VkBool32 VKAPI_CALL RendererDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
{
    printf("[VK_VALIDATION]: %s\n", callbackData->pMessage);
    // Halt the program if the validation issue is an error
    ASSERT(!(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT));

    return VK_FALSE;
}

#if _DEBUG
VK_DECLARE_PROC(CreateDebugUtilsMessengerEXT);
VK_DECLARE_PROC(DestroyDebugUtilsMessengerEXT);
#endif
VK_DECLARE_PROC(GetPhysicalDeviceSurfaceSupportKHR);

// TODO LIST
//      CreateRenderContext**
//      CreateSwapChain**
//      CreateRenderPass**
//      CreateGraphicsPipeline**
//
//      CreateCommandBuffer**
//
//      Render/Present code
//
//      ResizeSwapChain**
struct RenderContext
{
    VkInstance apiInstance = VK_NULL_HANDLE;
    VkSurfaceKHR apiSurface = VK_NULL_HANDLE;
    VkPhysicalDevice apiPhysicalDevice = VK_NULL_HANDLE;
    VkDevice apiDevice = VK_NULL_HANDLE;
    u32 apiCommandQueueFamily = -1;
    VkQueue apiCommandQueue = VK_NULL_HANDLE;
#if _DEBUG
    VkDebugUtilsMessengerEXT apiDebugMessenger;
#endif

    VkCommandPool apiCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer apiCommandBuffer = VK_NULL_HANDLE;  // TODO(caio): Add one buffer per simultaneous frame

    VkSemaphore apiSemaphoreRender = VK_NULL_HANDLE;
    VkSemaphore apiSemaphorePresent = VK_NULL_HANDLE;
    VkFence apiFenceRender = VK_NULL_HANDLE;
};

RenderContext CreateRenderContext(const char* appName, const char* engineName, HWND osWindow, HINSTANCE osInstance)
{
    // Detailing application info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Creating vulkan instance with required extensions
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    const char* instanceExtensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#if _DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };
    instanceInfo.enabledExtensionCount = ARR_LEN(instanceExtensions);
    instanceInfo.ppEnabledExtensionNames = &instanceExtensions[0];
#if _DEBUG
    //  Enabling debug validation layer (only in debug builds)
    const char* validationLayers[] =
    {
        "VK_LAYER_KHRONOS_validation",
    };

    u32 layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    ASSERT(layerCount);
    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for(i32 layerToFind = 0; layerToFind < ARR_LEN(validationLayers); layerToFind++)
    {
        i32 match = FIND_STRING_IN_AOS(availableLayers, ARR_LEN(availableLayers), validationLayers[layerToFind], layerName);
        ASSERT(match != -1);
    }
    
    instanceInfo.enabledLayerCount = ARR_LEN(validationLayers);
    instanceInfo.ppEnabledLayerNames = validationLayers;
#endif

    VkInstance instance;
    VkResult ret = vkCreateInstance(&instanceInfo, NULL, &instance);
    VK_ASSERT(ret);

#if _DEBUG
    //  Setup validation layer callback
    VkDebugUtilsMessengerCreateInfoEXT validationCallbackInfo = {};
    validationCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    validationCallbackInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    validationCallbackInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    validationCallbackInfo.pfnUserCallback = RendererDebugCallback;
    validationCallbackInfo.pUserData = NULL;

    VK_GET_IPROC(instance, CreateDebugUtilsMessengerEXT);
    VkDebugUtilsMessengerEXT debugMessenger;
    ret = CreateDebugUtilsMessengerEXT(instance, &validationCallbackInfo, NULL, &debugMessenger);
    VK_ASSERT(ret);
#endif

    // Create window surface to interface with OS window
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd = osWindow;
    surfaceInfo.hinstance = osInstance;
    VkSurfaceKHR surface;
    ret = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, NULL, &surface);
    VK_ASSERT(ret);

    // Selecting physical device (currently just selecting the first device to match requirements)
    const char* deviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    u32 physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
    ASSERT(physicalDeviceCount);
    VkPhysicalDevice physicalDevices[physicalDeviceCount];
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
    u32 selectedDevice = -1;
    for(i32 deviceIndex = 0; deviceIndex < physicalDeviceCount; deviceIndex++)
    {
        // Check if extensions are supported
        VkPhysicalDevice physicalDevice = physicalDevices[deviceIndex];
        u32 extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, NULL);
        VkExtensionProperties extensions[extensionCount];
        vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &extensionCount, extensions);
        bool supportsRequiredExtensions = true;
        for(i32 i = 0; i < ARR_LEN(deviceExtensions); i++)
        {
            const char* extensionToFind = deviceExtensions[i];
            i32 match = FIND_STRING_IN_AOS(extensions, ARR_LEN(extensions), extensionToFind, extensionName);
            supportsRequiredExtensions |= match != -1;
        }
        if(!supportsRequiredExtensions) continue;

        // Check if surface properties are supported 
        u32 surfaceFormatCount = 0, surfacePresentModeCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, NULL);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, NULL);
        if(!surfaceFormatCount || !surfacePresentModeCount) continue;

        // Check if device supports the required features for application
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &features);
        if(properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) continue; // Add more if needed

        // Found a device matching all requirements
        selectedDevice = deviceIndex;
        break;
    }

    ASSERT(selectedDevice != -1);
    VkPhysicalDevice physicalDevice = physicalDevices[selectedDevice];

    // Finding first command queue family that supports required command types
    u32 commandQueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &commandQueueFamilyCount, NULL);
    ASSERT(commandQueueFamilyCount);
    VkQueueFamilyProperties commandQueueFamilyProperties[commandQueueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &commandQueueFamilyCount, commandQueueFamilyProperties);
    u32 commandQueueFamily = -1;
    for(i32 i = 0; i < commandQueueFamilyCount; i++)
    {
        // Queue family supports graphics commands
        VkQueueFamilyProperties properties = commandQueueFamilyProperties[i];
        if(!(properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) continue;

        // Queue family supports present commands
        VK_GET_IPROC(instance, GetPhysicalDeviceSurfaceSupportKHR);
        VkBool32 supportsPresent;
        GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent);
        if(supportsPresent == VK_FALSE) continue;

        commandQueueFamily = i;
        break;
    }
    ASSERT(commandQueueFamily != -1);

    // Creating logical device
    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = commandQueueFamily;
    queueInfo.queueCount = 1;
    f32 deviceQueuePriority = 1;
    queueInfo.pQueuePriorities = &deviceQueuePriority;
    
    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.enabledExtensionCount = ARR_LEN(deviceExtensions);
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;
    VkDevice device;
    ret = vkCreateDevice(physicalDevice, &deviceInfo, NULL, &device);
    VK_ASSERT(ret);

    // Referencing command queue created from device
    VkQueue commandQueue;
    vkGetDeviceQueue(device, commandQueueFamily, 0, &commandQueue);

    // Creating command buffers
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = commandQueueFamily;
    commandPoolInfo.pNext = NULL;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCommandPool commandPool;
    ret = vkCreateCommandPool(device, &commandPoolInfo, NULL, &commandPool);
    VK_ASSERT(ret);

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandBufferCount = 1;
    commandBufferAllocInfo.commandPool = commandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.pNext = NULL;
    VkCommandBuffer commandBuffer;
    ret = vkAllocateCommandBuffers(device, &commandBufferAllocInfo, &commandBuffer);
    VK_ASSERT(ret);

    // Creating default render/present sync structures
    VkSemaphore semaphoreRender, semaphorePresent;
    VkFence fenceRender;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = NULL;
    ret = vkCreateSemaphore(device, &semaphoreInfo, NULL, &semaphoreRender);
    VK_ASSERT(ret);
    ret = vkCreateSemaphore(device, &semaphoreInfo, NULL, &semaphorePresent);
    VK_ASSERT(ret);

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    ret = vkCreateFence(device, &fenceInfo, NULL, &fenceRender);
    VK_ASSERT(ret);

    RenderContext result = {};
    result.apiInstance = instance;
    result.apiSurface = surface;
    result.apiPhysicalDevice = physicalDevice;
    result.apiDevice = device;
    result.apiCommandQueueFamily = commandQueueFamily;
    result.apiCommandQueue = commandQueue;
#if _DEBUG
    result.apiDebugMessenger = debugMessenger;
#endif
    result.apiCommandPool = commandPool;
    result.apiCommandBuffer = commandBuffer;
    result.apiSemaphoreRender = semaphoreRender;
    result.apiSemaphorePresent = semaphorePresent;
    result.apiFenceRender = fenceRender;
    return result;
}

void DestroyRenderContext(RenderContext* ctx)
{
    ASSERT(ctx);
    vkDestroySemaphore(ctx->apiDevice, ctx->apiSemaphoreRender, NULL);
    vkDestroySemaphore(ctx->apiDevice, ctx->apiSemaphorePresent, NULL);
    vkDestroyFence(ctx->apiDevice, ctx->apiFenceRender, NULL);
    vkDestroyCommandPool(ctx->apiDevice, ctx->apiCommandPool, NULL);
    vkDestroyDevice(ctx->apiDevice, NULL);
#if _DEBUG
    VK_GET_IPROC(ctx->apiInstance, DestroyDebugUtilsMessengerEXT);
    DestroyDebugUtilsMessengerEXT(ctx->apiInstance, ctx->apiDebugMessenger, NULL);
#endif
    vkDestroySurfaceKHR(ctx->apiInstance, ctx->apiSurface, NULL);
    vkDestroyInstance(ctx->apiInstance, NULL);

    *ctx = {};
}

#define SURFACE_MAX_FORMATS         16
#define SURFACE_MAX_PRESENT_MODES   16

struct WindowSurfaceDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount = -1;
    VkSurfaceFormatKHR formats[SURFACE_MAX_FORMATS];
    u32 presentModeCount = -1;
    VkPresentModeKHR presentModes[SURFACE_MAX_PRESENT_MODES];
};

WindowSurfaceDetails QueryWindowSurfaceDetails(RenderContext* ctx)
{
    WindowSurfaceDetails result = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->apiPhysicalDevice, ctx->apiSurface, &result.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->apiPhysicalDevice, ctx->apiSurface, &result.formatCount, NULL);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->apiPhysicalDevice, ctx->apiSurface, &result.formatCount, result.formats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->apiPhysicalDevice, ctx->apiSurface, &result.presentModeCount, NULL);
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->apiPhysicalDevice, ctx->apiSurface, &result.presentModeCount, result.presentModes);

    return result;
}

#define SWAP_CHAIN_MAX_IMAGE_COUNT 4
struct SwapChain
{
    VkSwapchainKHR      apiObject = VK_NULL_HANDLE;
    VkFormat            format;
    VkColorSpaceKHR     colorSpace;
    VkPresentModeKHR    presentMode;
    VkExtent2D          extents;
    
    u32         imageCount = 0;
    VkImage     apiImages[SWAP_CHAIN_MAX_IMAGE_COUNT];
    VkImageView apiImageViews[SWAP_CHAIN_MAX_IMAGE_COUNT];
};

SwapChain CreateSwapChain(RenderContext* ctx)
{
    ASSERT(ctx->apiDevice != VK_NULL_HANDLE);

    WindowSurfaceDetails surfaceDetails = QueryWindowSurfaceDetails(ctx);

    // Selecting swap chain parameters based on surface support
    VkFormat format;
    VkColorSpaceKHR colorSpace;
    VkPresentModeKHR presentMode;
    VkExtent2D extents;
    u32 minImageCount = 0;

    // Finding adequate swap chain format (if not found, just use first available)
    format = surfaceDetails.formats[0].format;
    colorSpace = surfaceDetails.formats[0].colorSpace;
    for(i32 i = 0; i < surfaceDetails.formatCount; i++)
    {
        if(surfaceDetails.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB 
                && surfaceDetails.formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            format = surfaceDetails.formats[i].format;
            colorSpace = surfaceDetails.formats[i].colorSpace;
            break;
        }
    }

    // Finding adequate swap chain present mode (if not found, just use FIFO)
    presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(i32 i = 0; i < surfaceDetails.presentModeCount; i++)
    {
        if(surfaceDetails.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = surfaceDetails.presentModes[i];
            break;
        }
    }

    // Finding swap chain image details
    ASSERT(surfaceDetails.capabilities.currentExtent.width != -1);  // Deal with this only if needed later.
    extents = surfaceDetails.capabilities.currentExtent;
    minImageCount = surfaceDetails.capabilities.minImageCount + 1;
    if(surfaceDetails.capabilities.maxImageCount > 0) minImageCount = MIN(minImageCount, surfaceDetails.capabilities.maxImageCount);

    // Create new swap chain based on details found
    SwapChain result = {};
    VkSwapchainKHR apiObject;
    VkSwapchainCreateInfoKHR apiObjectInfo = {};
    apiObjectInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    apiObjectInfo.surface = ctx->apiSurface;
    apiObjectInfo.minImageCount = minImageCount;
    apiObjectInfo.imageFormat = format;
    apiObjectInfo.imageColorSpace = colorSpace;
    apiObjectInfo.imageArrayLayers = 1;
    apiObjectInfo.imageExtent = extents;
    apiObjectInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Render directly to swap chain imgs
    apiObjectInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;     // Can't be shared between queues
    apiObjectInfo.preTransform = surfaceDetails.capabilities.currentTransform;
    apiObjectInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    apiObjectInfo.presentMode = presentMode;
    apiObjectInfo.clipped = VK_TRUE;
    apiObjectInfo.oldSwapchain = VK_NULL_HANDLE;
    VkResult ret = vkCreateSwapchainKHR(ctx->apiDevice, &apiObjectInfo, NULL, &apiObject);
    VK_ASSERT(ret);

    result =
    {
        apiObject, format, colorSpace, presentMode, extents
    };

    // Reference images and image views for new swap chain
    ret = vkGetSwapchainImagesKHR(ctx->apiDevice, result.apiObject, &result.imageCount, NULL);
    VK_ASSERT(ret);
    ASSERT(result.imageCount != 0);
    ret = vkGetSwapchainImagesKHR(ctx->apiDevice, result.apiObject, &result.imageCount, result.apiImages);
    VK_ASSERT(ret);
    for(i32 i = 0; i < result.imageCount; i++)
    {
        VkImageViewCreateInfo apiImageViewInfo = {};
        apiImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        apiImageViewInfo.image = result.apiImages[i];
        apiImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        apiImageViewInfo.format = result.format;
        apiImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        apiImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        apiImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        apiImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        apiImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        apiImageViewInfo.subresourceRange.baseMipLevel = 0;
        apiImageViewInfo.subresourceRange.levelCount = 1;
        apiImageViewInfo.subresourceRange.baseArrayLayer = 0;
        apiImageViewInfo.subresourceRange.layerCount = 1;
        VkResult ret = vkCreateImageView(ctx->apiDevice, &apiImageViewInfo, NULL, &result.apiImageViews[i]);
        VK_ASSERT(ret);
    }

    return result;
}

void DestroySwapChain(RenderContext* ctx, SwapChain* swapChain)
{
    ASSERT(ctx->apiDevice != VK_NULL_HANDLE);
    ASSERT(swapChain);
    for(i32 i = 0; i < swapChain->imageCount; i++)
    {
        vkDestroyImageView(ctx->apiDevice, swapChain->apiImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(ctx->apiDevice, swapChain->apiObject, NULL);

    *swapChain = {};
}

void ResizeSwapChain(RenderContext* ctx, SwapChain* swapChain)
{
    ASSERT(ctx->apiDevice != VK_NULL_HANDLE);
    ASSERT(swapChain);
    DestroySwapChain(ctx, swapChain);
    *swapChain = CreateSwapChain(ctx);
}

// ===================================================================
// Render pass

enum RenderPassLoadOp
{
    RENDER_PASS_LOAD_OP_LOAD,
    RENDER_PASS_LOAD_OP_CLEAR,
    RENDER_PASS_LOAD_OP_DONT_CARE,
};
VkAttachmentLoadOp renderPassLoadOpToVk[] =
{
    VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
};

enum RenderPassStoreOp
{
    RENDER_PASS_STORE_OP_STORE,
    RENDER_PASS_STORE_OP_DONT_CARE,
};
VkAttachmentStoreOp renderPassStoreOpToVk[] =
{
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE,
};

// TODO(caio): Move this out of here when doing texture/image resources
enum ImageFormat
{
    IMAGE_FORMAT_B8G8R8A8_SRGB,
};
VkFormat imageFormatToVk[] =
{
    VK_FORMAT_B8G8R8A8_SRGB,
};

enum ImageLayout
{
    IMAGE_LAYOUT_UNDEFINED,
    IMAGE_LAYOUT_COLOR_OUTPUT_OPTIMAL,
    IMAGE_LAYOUT_PRESENT_SRC,
};
VkImageLayout imageLayoutToVk[] =
{
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
};

struct RenderPassColorOutputInfo
{
    RenderPassLoadOp loadOp      = RENDER_PASS_LOAD_OP_DONT_CARE;
    RenderPassStoreOp storeOp    = RENDER_PASS_STORE_OP_DONT_CARE;
    ImageLayout initialLayout    = IMAGE_LAYOUT_UNDEFINED;
    ImageLayout finalLayout      = IMAGE_LAYOUT_UNDEFINED;
    ImageFormat format           = IMAGE_FORMAT_B8G8R8A8_SRGB;
};

#define RENDER_PASS_MAX_FRAME_COUNT  4     // Multiple framebuffers for double/triple-buffering support
#define RENDER_PASS_MAX_COLOR_OUTPUTS 8
// This contains actual image views used in framebuffer.
// Each color/depth output info describes one of the images in this struct.
// Render passes with multiple frames use the same output infos between frames.
// Note: RenderPass is not responsible for image resource ownership, only holds references.
struct RenderPassFrameOutputs
{
    VkImageView apiColorOutputImageViews[RENDER_PASS_MAX_COLOR_OUTPUTS];
    // TODO(caio): Depth/stencil output image view
};

struct RenderPass
{
    VkRenderPass apiObject = VK_NULL_HANDLE;

    u32 frameCount = 0;
    VkFramebuffer apiFramebuffers[RENDER_PASS_MAX_FRAME_COUNT];
    RenderPassFrameOutputs frameOutputs[RENDER_PASS_MAX_FRAME_COUNT];

    u32 outputWidth;
    u32 outputHeight;

    u32 colorOutputCount;
    RenderPassColorOutputInfo colorOutputInfo[RENDER_PASS_MAX_COLOR_OUTPUTS];
};

RenderPass CreateRenderPass(RenderContext* ctx, u32 frameCount, u32 width, u32 height,
        u32 colorOutputCount, RenderPassColorOutputInfo* colorOutputInfo, RenderPassFrameOutputs* frameOutputs)
{
    VkAttachmentDescription colorAttachments[RENDER_PASS_MAX_COLOR_OUTPUTS] = {0};
    for(i32 i = 0; i < colorOutputCount; i++)
    {
        colorAttachments[i].format = imageFormatToVk[colorOutputInfo[i].format];
        colorAttachments[i].initialLayout = imageLayoutToVk[colorOutputInfo[i].initialLayout];
        colorAttachments[i].finalLayout = imageLayoutToVk[colorOutputInfo[i].finalLayout];
        colorAttachments[i].loadOp = renderPassLoadOpToVk[colorOutputInfo[i].loadOp];
        colorAttachments[i].storeOp = renderPassStoreOpToVk[colorOutputInfo[i].storeOp];
        colorAttachments[i].samples = VK_SAMPLE_COUNT_1_BIT;    // No support for multisampling
        colorAttachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    // TODO(caio): Add support for depth/stencil attachments

    VkAttachmentReference colorOutputRefs[colorOutputCount];
    for(i32 i = 0; i < colorOutputCount; i++)
    {
        colorOutputRefs[i].attachment = i;
        colorOutputRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // Single subpass per render pass
    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = colorOutputCount;
    subpassDesc.pColorAttachments = colorOutputRefs;

    // Creating render pass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.attachmentCount = colorOutputCount;
    renderPassInfo.pAttachments = colorAttachments;
    VkRenderPass renderPass;
    VkResult ret = vkCreateRenderPass(ctx->apiDevice, &renderPassInfo, NULL, &renderPass);
    VK_ASSERT(ret);

    RenderPass result = {};
    result.apiObject = renderPass;

    // Creating output framebuffers
    ASSERT(frameCount > 0);
    ASSERT(frameCount <= RENDER_PASS_MAX_FRAME_COUNT);
    for(i32 i = 0; i < frameCount; i++)
    {
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = colorOutputCount;
        framebufferInfo.pAttachments = frameOutputs[i].apiColorOutputImageViews;
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        VkFramebuffer framebuffer;
        ret = vkCreateFramebuffer(ctx->apiDevice, &framebufferInfo, NULL, &framebuffer);
        VK_ASSERT(ret);
        result.apiFramebuffers[i] = framebuffer;
    }

    result.frameCount = frameCount;
    result.outputWidth = width;
    result.outputHeight = height;
    result.colorOutputCount = colorOutputCount;
    for(i32 i = 0; i < colorOutputCount; i++)
    {
        result.colorOutputInfo[i] = colorOutputInfo[i];
        for(i32 j = 0; j < frameCount; j++)
        {
            result.frameOutputs[j].apiColorOutputImageViews[i] = frameOutputs[j].apiColorOutputImageViews[i];
        }
    }

    return result;
}

void DestroyRenderPass(RenderContext* ctx, RenderPass* renderPass)
{
    ASSERT(ctx->apiDevice != VK_NULL_HANDLE);
    ASSERT(renderPass->apiObject != VK_NULL_HANDLE);
    for(i32 i = 0; i < renderPass->frameCount; i++)
    {
        vkDestroyFramebuffer(ctx->apiDevice, renderPass->apiFramebuffers[i], NULL);
    }
    vkDestroyRenderPass(ctx->apiDevice, renderPass->apiObject, NULL);

    *renderPass = {};
}

// On Resize:
//      Destroy swap chain
//      Create new swap chain
//      Destroy present render pass
//      Create new present render pass based on new swap chain

void OnResize(RenderContext* ctx, SwapChain* swapChain, RenderPass* presentRenderPass)
{
    vkDeviceWaitIdle(ctx->apiDevice);

    // First, verify if this is a minimize and if so, wait until window size is valid.
    WindowSurfaceDetails surfaceDetails = QueryWindowSurfaceDetails(ctx);
    while(surfaceDetails.capabilities.currentExtent.width == 0
    || surfaceDetails.capabilities.currentExtent.height == 0)
    {
        WaitMessage();
        ProcessWindowMessages();
        surfaceDetails = QueryWindowSurfaceDetails(ctx);
    }

    ResizeSwapChain(ctx, swapChain);
    DestroyRenderPass(ctx, presentRenderPass);

    u32 presentRenderPassColorOutputCount = 1;
    RenderPassColorOutputInfo presentRenderPassColorOutputInfo[] =
    {
        {
            RENDER_PASS_LOAD_OP_CLEAR,
            RENDER_PASS_STORE_OP_STORE,
            IMAGE_LAYOUT_UNDEFINED,
            IMAGE_LAYOUT_PRESENT_SRC,
            IMAGE_FORMAT_B8G8R8A8_SRGB,
        }
    };
    // Each swap chain image is tied to the first color output of each frame in the present pass.
    // This can probably be improved for clarity.
    RenderPassFrameOutputs presentRenderPassFrameOutputs[swapChain->imageCount];
    for(i32 i = 0; i < swapChain->imageCount; i++)
    {
        for(i32 j = 0; j < presentRenderPassColorOutputCount; j++)
        {
            presentRenderPassFrameOutputs[i].apiColorOutputImageViews[j] = swapChain->apiImageViews[i];
        }
    }
    *presentRenderPass = CreateRenderPass(ctx, 
            swapChain->imageCount, swapChain->extents.width, swapChain->extents.height, 1,
            presentRenderPassColorOutputInfo, presentRenderPassFrameOutputs);
};

// ===================================================================
// Graphics pipeline
enum ShaderType
{
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_PIXEL,
};

struct ShaderAsset
{
    ShaderType type;
    u64 bytecodeSize = -1;
    u8* bytecode = NULL;
};

#define SHADER_BYTECODE_ALIGN 4     // 4 byte alignment for shader memory
ShaderAsset CreateShaderAsset(const char* assetPath, ShaderType type)
{
    u64 assetSize = GetFileSize(assetPath);
    // TODO(caio)#MEMORY: Remove dynamic alloc in favor of custom resource arena allocator
    u8* assetData = (u8*)_aligned_malloc(assetSize, SHADER_BYTECODE_ALIGN);
    ReadFileAsBinary(assetPath, assetSize, assetData);
    return
    {
        type, assetSize, assetData
    };
}

enum PrimitiveType
{
    PRIMITIVE_TRIANGLE_LIST,
};
VkPrimitiveTopology primitiveTypeToVk[] =
{
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
};

enum FillMode
{
    FILL_MODE_SOLID,
    FILL_MODE_LINE,
    FILL_MODE_POINT,
};
VkPolygonMode fillModeToVk[] =
{
    VK_POLYGON_MODE_FILL,
    VK_POLYGON_MODE_LINE,
    VK_POLYGON_MODE_POINT,
};

enum CullMode
{
    CULL_MODE_NONE,
    CULL_MODE_FRONT,
    CULL_MODE_BACK,
    CULL_MODE_FRONT_BACK,
};
VkCullModeFlagBits cullModeToVk[] =
{
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_FRONT_BIT,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_AND_BACK,
};

enum FrontFace
{
    FRONT_FACE_CW,
    FRONT_FACE_CCW,
};
VkFrontFace frontFaceModeToVk[] =
{
    VK_FRONT_FACE_CLOCKWISE,
    VK_FRONT_FACE_COUNTER_CLOCKWISE,
};

struct InputAssemblyState
{
    PrimitiveType primitive = PRIMITIVE_TRIANGLE_LIST;
};

struct RasterizerState
{
    FillMode fillMode = FILL_MODE_SOLID;
    CullMode cullMode = CULL_MODE_BACK;
    FrontFace frontFace = FRONT_FACE_CW;
};

struct GraphicsPipeline
{
    VkPipeline apiObject = VK_NULL_HANDLE;
    VkPipelineLayout apiPipelineLayout = VK_NULL_HANDLE;

    // Programmable pipeline
    ShaderAsset shaderVertex;
    ShaderAsset shaderPixel;

    // Fixed pipeline
    // TODO(caio): Add vertex input stage handling here
    InputAssemblyState inputAssemblyState;
    // TODO(caio): Add support for dynamic viewport and scissor rect
    RasterizerState rasterizerState;
    // TODO(caio): Add support for color blend modes, depth testing modes...
};

GraphicsPipeline CreateGraphicsPipeline(
        RenderContext* ctx, 
        RenderPass* renderPass,
        InputAssemblyState inputAssemblyState,
        ShaderAsset vs, ShaderAsset ps, 
        RasterizerState rasterizerState)
{
    ASSERT(ctx->apiDevice != VK_NULL_HANDLE);
    ASSERT(renderPass->apiObject != VK_NULL_HANDLE);
    VkResult ret;

    // Setting up programmable stages
    ASSERT(vs.bytecodeSize != -1);
    ASSERT(ps.bytecodeSize != -1);
    VkShaderModuleCreateInfo vsShaderModuleInfo = {};
    vsShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vsShaderModuleInfo.codeSize = vs.bytecodeSize;
    vsShaderModuleInfo.pCode = (u32*)vs.bytecode;
    VkShaderModuleCreateInfo psShaderModuleInfo = vsShaderModuleInfo;
    psShaderModuleInfo.codeSize = ps.bytecodeSize;
    psShaderModuleInfo.pCode = (u32*)ps.bytecode;
    VkShaderModule vsShaderModule;
    ret = vkCreateShaderModule(ctx->apiDevice, &vsShaderModuleInfo, NULL, &vsShaderModule);
    VK_ASSERT(ret);
    VkShaderModule psShaderModule;
    ret = vkCreateShaderModule(ctx->apiDevice, &psShaderModuleInfo, NULL, &psShaderModule);
    VK_ASSERT(ret);

    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.pName = "main";     // Shader entrypoint hardcoded to always be main
    VkPipelineShaderStageCreateInfo vsShaderStageInfo = shaderStageInfo;
    vsShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vsShaderStageInfo.module = vsShaderModule;
    VkPipelineShaderStageCreateInfo psShaderStageInfo = shaderStageInfo;
    psShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    psShaderStageInfo.module = psShaderModule;
    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        vsShaderStageInfo, psShaderStageInfo
    };
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = primitiveTypeToVk[inputAssemblyState.primitive];
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // Vertex input (currently empty, no vertex buffers yet)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL;

    // Viewport state (currently only default viewports/scissor rect, created from RT)
    VkViewport viewport;
    viewport.x = 0; viewport.y = 0;
    viewport.width = renderPass->outputWidth;
    viewport.height = renderPass->outputHeight;
    viewport.minDepth = 0; viewport.maxDepth = 1;

    VkRect2D scissorRect;
    scissorRect.offset = {0,0};
    scissorRect.extent = {renderPass->outputWidth, renderPass->outputHeight};

    VkPipelineViewportStateCreateInfo viewportStateInfo = {};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissorRect;

    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
    rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateInfo.polygonMode = fillModeToVk[rasterizerState.fillMode];
    rasterizationStateInfo.cullMode = cullModeToVk[rasterizerState.cullMode];
    rasterizationStateInfo.frontFace = frontFaceModeToVk[rasterizerState.frontFace];
    rasterizationStateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateInfo.lineWidth = 1;
    rasterizationStateInfo.depthBiasEnable = VK_FALSE;

    // Blending (currently only supporting standard opaque blend)
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | 
        VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | 
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;    // Overwrite color
    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;

    // Pipeline layout (for uniform buffers, currently empty)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    VkPipelineLayout pipelineLayout;
    ret = vkCreatePipelineLayout(ctx->apiDevice, &pipelineLayoutInfo, NULL, &pipelineLayout);
    VK_ASSERT(ret);

    // Graphics pipeline creation
    VkGraphicsPipelineCreateInfo apiObjectInfo = {};
    apiObjectInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    apiObjectInfo.stageCount = 2;   // Hardcoded vertex and pixel stages
    apiObjectInfo.pStages = shaderStages;
    apiObjectInfo.pVertexInputState = &vertexInputInfo;
    apiObjectInfo.pInputAssemblyState = &inputAssemblyInfo;
    apiObjectInfo.pViewportState = &viewportStateInfo;
    apiObjectInfo.pRasterizationState = &rasterizationStateInfo;
    apiObjectInfo.pColorBlendState = &colorBlendInfo;
    apiObjectInfo.pDynamicState = NULL;
    apiObjectInfo.pDepthStencilState = NULL;
    apiObjectInfo.layout = pipelineLayout;
    apiObjectInfo.renderPass = renderPass->apiObject;
    apiObjectInfo.subpass = 0;      // Hardcoded to use only one subpass
    apiObjectInfo.basePipelineHandle = VK_NULL_HANDLE;
    apiObjectInfo.basePipelineIndex = -1;
    VkPipeline apiObject;
    ret = vkCreateGraphicsPipelines(ctx->apiDevice, VK_NULL_HANDLE, 1, &apiObjectInfo,
            NULL, &apiObject);
    VK_ASSERT(ret);

    // Destroy unneeded shader modules
    vkDestroyShaderModule(ctx->apiDevice, vsShaderModule, NULL);
    vkDestroyShaderModule(ctx->apiDevice, psShaderModule, NULL);

    GraphicsPipeline result =
    {
        apiObject, pipelineLayout, vs, ps, inputAssemblyState, rasterizerState,
    };
    return result;
}

void DestroyGraphicsPipeline(RenderContext* ctx, GraphicsPipeline* pipeline)
{
    vkDestroyPipeline(ctx->apiDevice, pipeline->apiObject, NULL);
    vkDestroyPipelineLayout(ctx->apiDevice, pipeline->apiPipelineLayout, NULL);

    *pipeline = {};
}

// ======================================================================
// Application main function
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, PWSTR pCmdLine, int nCmdShow)
{
    // ======================================================================
    // Win32 window initialization
    const char* windowClassName = "VulkanHelloCubeWndClass";
    WNDCLASSA windowClass = {};
    windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    windowClass.lpszClassName = windowClassName;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    RegisterClassA(&windowClass);

    windowHandle = CreateWindowEx(
            0,
            windowClassName,
            "Vulkan Hello Cube",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            windowWidth, windowHeight,
            NULL, NULL,
            hInstance,
            NULL);
    ASSERT(windowHandle);
    
    ShowWindow(windowHandle, nCmdShow);

    // ======================================================================
    // Render initialization

    RenderContext ctx = CreateRenderContext("Vulkan Hello Cube", "TypheusRendererVk", windowHandle, hInstance);
    SwapChain swapChain = CreateSwapChain(&ctx);

    u32 presentRenderPassColorOutputCount = 1;
    RenderPassColorOutputInfo presentRenderPassColorOutputInfo[] =
    {
        {
            RENDER_PASS_LOAD_OP_CLEAR,
            RENDER_PASS_STORE_OP_STORE,
            IMAGE_LAYOUT_UNDEFINED,
            IMAGE_LAYOUT_PRESENT_SRC,
            IMAGE_FORMAT_B8G8R8A8_SRGB,
        }
    };
    // Each swap chain image is tied to the first color output of each frame in the present pass.
    // This can probably be improved for clarity.
    RenderPassFrameOutputs presentRenderPassFrameOutputs[swapChain.imageCount];
    for(i32 i = 0; i < swapChain.imageCount; i++)
    {
        for(i32 j = 0; j < presentRenderPassColorOutputCount; j++)
        {
            presentRenderPassFrameOutputs[i].apiColorOutputImageViews[j] = swapChain.apiImageViews[i];
        }
    }
    RenderPass presentRenderPass = CreateRenderPass(&ctx, 
            swapChain.imageCount, swapChain.extents.width, swapChain.extents.height, 1,
            presentRenderPassColorOutputInfo, presentRenderPassFrameOutputs);

    ShaderAsset shader_TriangleVS = CreateShaderAsset(SHADER_PATH"first_triangle_vs.spv", SHADER_TYPE_VERTEX);
    ShaderAsset shader_TrianglePS = CreateShaderAsset(SHADER_PATH"first_triangle_ps.spv", SHADER_TYPE_PIXEL);
    InputAssemblyState defaultPassInputAssemblyState = {};
    defaultPassInputAssemblyState.primitive = PRIMITIVE_TRIANGLE_LIST;
    RasterizerState defaultPassRasterizerState = {};
    defaultPassRasterizerState.fillMode = FILL_MODE_SOLID;
    defaultPassRasterizerState.cullMode = CULL_MODE_BACK;
    defaultPassRasterizerState.frontFace = FRONT_FACE_CCW;
    GraphicsPipeline defaultPassPipeline = CreateGraphicsPipeline(&ctx, &presentRenderPass,
            defaultPassInputAssemblyState, shader_TriangleVS, shader_TrianglePS, defaultPassRasterizerState);


    // ======================================================================
    // Render loop (still not abstracted)
    
    uint32_t currentFrame = 0;
    while(!closeApp)
    {
        ProcessWindowMessages();

        //  Wait for previous frame to finish
        vkWaitForFences(ctx.apiDevice, 1, &ctx.apiFenceRender, VK_TRUE, UINT64_MAX);

        //  Acquire the next swap chain image to render to
        uint32_t currentSwapChainImage;
        VkResult ret = vkAcquireNextImageKHR(ctx.apiDevice, swapChain.apiObject, UINT64_MAX, ctx.apiSemaphorePresent, VK_NULL_HANDLE, &currentSwapChainImage);
        if(ret == VK_ERROR_OUT_OF_DATE_KHR)
        {
            wasResized = false;
            OnResize(&ctx, &swapChain, &presentRenderPass);
            continue;
        }
        else if(ret != VK_SUCCESS) ASSERT(0);

        // Reset render fence when work is to be submitted
        vkResetFences(ctx.apiDevice, 1, &ctx.apiFenceRender);

        // Reset command buffer from previous frame
        vkResetCommandBuffer(ctx.apiCommandBuffer, 0);

        // Prepare command buffer for recording commands
        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = NULL;
        commandBufferBeginInfo.pInheritanceInfo = NULL;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        ret = vkBeginCommandBuffer(ctx.apiCommandBuffer, &commandBufferBeginInfo);
        VK_ASSERT(ret);

        // Begin render pass
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = NULL;
        renderPassBeginInfo.renderPass = presentRenderPass.apiObject;
        renderPassBeginInfo.framebuffer = presentRenderPass.apiFramebuffers[currentSwapChainImage];
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        //renderPassBeginInfo.renderArea.extent = swapChainSupportDetails.extent;
        renderPassBeginInfo.renderArea.extent = {presentRenderPass.outputWidth, presentRenderPass.outputHeight};
        VkClearValue clearValue;
        float flash = fabsf(sinf(currentFrame / 2000.f)) * 0.05f;
        clearValue.color = {{flash, flash, flash, 1.0f}};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(ctx.apiCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Draw commands
        vkCmdBindPipeline(ctx.apiCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPassPipeline.apiObject);
        vkCmdDraw(ctx.apiCommandBuffer, 3, 1, 0, 0);

        // End render pass
        vkCmdEndRenderPass(ctx.apiCommandBuffer);

        // Finalize command buffer for submission
        ret = vkEndCommandBuffer(ctx.apiCommandBuffer);
        VK_ASSERT(ret);

        // Submit command buffer
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &ctx.apiCommandBuffer;
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitInfo.pWaitDstStageMask = &waitStage;
        //      Wait for present semaphore, to ensure swap chain image is ready
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &ctx.apiSemaphorePresent;
        //      Signal render semaphore, to indicate render commands have all been executed
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &ctx.apiSemaphoreRender;
        
        ret = vkQueueSubmit(ctx.apiCommandQueue, 1, &submitInfo, ctx.apiFenceRender);
        VK_ASSERT(ret);

        // Present image to window
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = NULL;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain.apiObject;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &ctx.apiSemaphoreRender;
        presentInfo.pImageIndices = &currentSwapChainImage;
        ret = vkQueuePresentKHR(ctx.apiCommandQueue, &presentInfo);
        if(ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR || wasResized)
        {
          wasResized = false;
          OnResize(&ctx, &swapChain, &presentRenderPass);
        }
        else if(ret != VK_SUCCESS) ASSERT(0);

        currentFrame++;
    }

    // ======================================================================
    // Render cleanup

    vkDeviceWaitIdle(ctx.apiDevice);
    DestroyGraphicsPipeline(&ctx, &defaultPassPipeline);
    DestroyRenderPass(&ctx, &presentRenderPass);
    DestroySwapChain(&ctx, &swapChain);
    DestroyRenderContext(&ctx);
    DestroyWindow(windowHandle);
    return 0;
}

int main()
{
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLineW(), SW_SHOWNORMAL);
}