import subprocess
import pkg_resources

def InstallPackage(packageName):
    print(f"Installing package {packageName}")
    subprocess.check_call(['python', '-m', 'pip', 'install', packageName])

def CheckPackage(packageName):
    required = { packageName }
    installed = {pkg.key for pkg in pkg_resources.working_set}
    missing = required - installed

    if missing:
        InstallPackage(packageName)

def DownloadPackagesIfNeeded():
    CheckPackage('requests')
    CheckPackage('fake-useragent')