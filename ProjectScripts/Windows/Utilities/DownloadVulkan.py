import os as OperatingSystem
import pathlib as path
import Utilities 
import time

CINNAMON_VULKAN_VERSION = "1.3.216.0"
VULKAN_SDK_ENVIRONMENT_VARIABLE = OperatingSystem.environ.get("VULKAN_SDK")

VULKAN_SDK_INSTALLER_URL = "https://sdk.lunarg.com/sdk/download/" + CINNAMON_VULKAN_VERSION  + "/windows/VulkanSDK-" + CINNAMON_VULKAN_VERSION + "-Installer.exe"
VULKAN_DIRECTORY = path.Path.home().drive + "\\VulkanSDK\\VulkanSDKInstaller.exe"

def ShouldDownload():
    while True:
        reply = str(input('[Y/N]: ')).lower().strip()
        if reply[:1] == 'y':
            return True
        if reply[:1] == 'n':
            return False

def DownloadSDK():
    print("Downloading Vulkan SDK")
    Utilities.DownloadFile(VULKAN_SDK_INSTALLER_URL, VULKAN_DIRECTORY)
    print("Installing Vulkan SDK")
    print("NOTE: You will be prompted with an installation dialogue wizard. When selecting installation components, make sure select the 64-bit debuggable shader API libraries!")
    OperatingSystem.startfile(OperatingSystem.path.abspath(VULKAN_DIRECTORY))

def CheckSDK():
    if VULKAN_SDK_ENVIRONMENT_VARIABLE is None:
        print(f"Vulkan SDK is not installed! Download and run the installer at {VULKAN_DIRECTORY}?")
        if ShouldDownload():
            DownloadSDK()
            return True
    elif (CINNAMON_VULKAN_VERSION not in VULKAN_SDK_ENVIRONMENT_VARIABLE):
        print(f"Vulkan SDK found at {VULKAN_SDK_ENVIRONMENT_VARIABLE}. Cinnamon requires Vulkan SDK version {CINNAMON_VULKAN_VERSION}")
        print(f"Download Vulkan SDK version {CINNAMON_VULKAN_VERSION}?")
        if ShouldDownload():
            DownloadSDK()
            return True
    else:
        print(f"Vulkan SDK version {CINNAMON_VULKAN_VERSION} is installed in {VULKAN_SDK_ENVIRONMENT_VARIABLE}")
        return False
    return False