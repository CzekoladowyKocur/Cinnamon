cd ../../

currentDirectory=$(echo "${PWD##*/}")
if [[ "$currentDirectory" != "Cinnamon" ]]
then
	echo "Executing in wrong directory, aborting"
	exit 1
fi

if [[ -d ".vscode" ]]
then
	echo "Found .vscode directory"
else
	echo "Did not found .vscode directory, making one..."
	mkdir .vscode
fi

cd .vscode
if [ $? != 0 ]
then
   echo "Failed entering directory .vscode"
   exit 1
fi

rm *.json
rm *.log

cat << EOF > c_cpp_propertires.json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [
                "CIN_DEBUG",
                "CIN_PLATFORM_LINUX",
                "VK_USE_PLATFORM_WAYLAND_KHR"
            ],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++20",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
EOF

cat << EOF > launch.json
{
    "configurations": [
    {
        "name": "Debug",
        "type": "cppdbg",
        "request": "launch",
        "program": "${workspaceFolder}/bin/Debug-linux-x86_64/Cinnamon/Cinnamon",
        "args": [],
        "stopAtEntry": false,
        "cwd": "${fileDirname}",
        "environment": [],
        "externalConsole": true,
        "MIMode": "gdb",
        "miDebuggerPath": "/usr/bin/gdb",
    }
    ]
}
EOF

echo "Successfulyy generated VSCode project files"