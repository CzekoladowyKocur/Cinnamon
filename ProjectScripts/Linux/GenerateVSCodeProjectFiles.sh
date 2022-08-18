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
	echo "No .vscode directory, making one"
	mkdir .vscode
fi

cd .vscode
if [ $? != 0 ]
then
   echo "Failed entering directory .vscode"
   exit 1
fi

if [[ -f *.log ]]
then
	rm *.log
fi

cat << EOF > c_cpp_properties.json
{
    "configurations": [
        {
            "name": "linux-gcc-debug",
            "intelliSenseMode": "linux-gcc-x64",
            "includePath": [
				"\${workspaceFolder}",
				"\${workspaceFolder}/Cinnamon/include"
            ],
            "defines": [
				"CIN_DEBUG",
                "CIN_PLATFORM_LINUX",
				"VK_USE_PLATFORM_WAYLAND_KHR"
            ],
            "forcedInclude": [

			],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++20"
        },
        { /* Todo */
            "name": "linux-gcc-release",
            "intelliSenseMode": "linux-gcc-x64",
            "includePath": [
				"\${workspaceFolder}",
				"\${workspaceFolder}/Cinnamon/include"
            ],
            "defines": [
				"CIN_RELEASE",
				"NDEBUG",
                "CIN_PLATFORM_LINUX",
				"VK_USE_PLATFORM_WAYLAND_KHR"
            ],
            "forcedInclude": [

			],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++20"
        },
        { /* Todo */
            "name": "linux-gcc-distribution",
            "intelliSenseMode": "linux-gcc-x64",
            "includePath": [
				"\${workspaceFolder}",
				"\${workspaceFolder}/Cinnamon/include"
            ],
            "defines": [
				"CIN_DISTRIBUTION",
				"NDEBUG",
                "CIN_PLATFORM_LINUX",
				"VK_USE_PLATFORM_WAYLAND_KHR"
            ],
            "forcedInclude": [

			],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++20"
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
        "program": "\${workspaceFolder}/bin/Debug-linux-x86_64/Cinnamon/Cinnamon",
        "args": [

		],
        "stopAtEntry": false,
        "cwd": "\${fileDirname}",
        "environment": [],
        "externalConsole": true,
        "MIMode": "gdb",
        "miDebuggerPath": "/usr/bin/gdb"
    }
    ]
}
EOF

echo "Successfulyy generated VSCode project files"
