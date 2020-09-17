import os

for file in os.listdir(os.getcwd()):
    if file.endswith(".edgelist"):
        string = "./vector.exe " + str(file) + " >> all_time.dat"
        print(string)
        os.system(string)

os.system("rm *.trg")
