def CheckVersion():
    print("Select your Visual studio version, eg: 2022")
    while True: 
        userInput = str(input('Visual studio version [2017, 2019, 2022]: ')).lower().strip()
        return userInput