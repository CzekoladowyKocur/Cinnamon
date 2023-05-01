import os as OperatingSystem
import pathlib as path
import Utilities 
import time
import tarfile
import subprocess

CINNAMON_VULKAN_VERSION = "1.3.216.0"
VULKAN_SDK_ENVIRONMENT_VARIABLE = OperatingSystem.environ.get("VULKAN_SDK")

VULKAN_SDK_INSTALLER_URL = "https://sdk.lunarg.com/sdk/download/" + CINNAMON_VULKAN_VERSION  + "/linux/vulkansdk-linux-x86_64-" + CINNAMON_VULKAN_VERSION + ".tar.gz"
VULKAN_DIRECTORY = OperatingSystem.getcwd() + "/VulkanSDK/" + "vulkansdk-linux-x86_64-" + CINNAMON_VULKAN_VERSION + ".tar.gz"

def ShouldDownload():
    while True:
        reply = str(input('[Y/N]: ')).lower().strip()
        if reply[:1] == 'y':
            return True
        if reply[:1] == 'n':
            return False

def DownloadSDK():
    OperatingSystem.mkdir(OperatingSystem.getcwd() + "/VulkanSDK")
    print("Downloading Vulkan SDK")
    Utilities.DownloadFile(VULKAN_SDK_INSTALLER_URL, VULKAN_DIRECTORY)
    print("Installing Vulkan SDK")
    
    with tarfile.open(VULKAN_DIRECTORY, "r:gz") as tar:
        tar.extractall(OperatingSystem.getcwd() + "/VulkanSDK")

    setupScriptPath = OperatingSystem.getcwd() + "/VulkanSDK/" + CINNAMON_VULKAN_VERSION + "/setup-env.sh"

    currentPermissions = OperatingSystem.stat(setupScriptPath).st_mode
    newPermissions = currentPermissions | 0o100
    OperatingSystem.chmod(setupScriptPath, newPermissions)

    subprocess.run(['sh', setupScriptPath], check=True)
    print("Installation was successfull")

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