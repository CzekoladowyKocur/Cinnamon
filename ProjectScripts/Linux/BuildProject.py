# Change working directory
import sys as System
System.path.append("Utilities/")

# Download needed packages 
import SetupPython as Setup
Setup.DownloadPackagesIfNeeded()

# Download the SDK
import DownloadVulkan as Vulkan
if Vulkan.CheckSDK():
	print("Rerun the script after vulkan has been installed! (Do not re-open the terminal)")
	exit()

# Build the project files
import subprocess as cmd
import os
print(os.getcwd())
cmd.check_call(["sh", os.getcwd() + "/GenerateVSCodeProjectFiles.sh"])

cwd = os.getcwd()
new_dir = os.path.abspath(os.path.join(cwd, "../../"))
os.chdir(new_dir)
cmd.check_call(["premake5", "gmake2"])
print(os.getcwd())
