# Change working directory
import sys as OperatingSystem
OperatingSystem.path.append("Utilities/")

# Download needed packages 
import SetupPython as Setup
Setup.DownloadPackagesIfNeeded()

# Download the SDK
import DownloadVulkan as Vulkan
if Vulkan.CheckSDK():
	print("Rerun the script after vulkan has been installed! (Re-open the CMD)")
	exit()

# Retrieve the Visual Studio version (TODO: support auto deduce)
import SetupVisualStudio as VS
VISUAL_STUDIO_VERSION = VS.CheckVersion()

# Build the project files
import subprocess as cmd
cmd.check_call(["DeleteProjectFiles.bat"])
if VISUAL_STUDIO_VERSION == "2019":
	cmd.check_call(["GenerateWindowsProjectVS2019.bat"])
elif VISUAL_STUDIO_VERSION == "2022":
	cmd.check_call(["GenerateWindowsProjectVS2022.bat"])
else:
	print("Invalid Visual Studio version selected! Re-run the script.")