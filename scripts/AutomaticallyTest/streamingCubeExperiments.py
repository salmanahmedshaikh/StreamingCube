#python
import os
import shutil
from math import sqrt


def grades_sum(data):
    total = 0
    for grade in data:
        total += grade
    return total

def grades_average(data):
    sum_of_grades = grades_sum(data)
    average = sum_of_grades / float(len(data))
    return average

def grades_variance(data):
    average=grades_average(data)
    variance=0
    for score in data:
        variance+=(average-score)**2
    return variance/len(data)

def grades_std_deviation(data):
    return grades_variance(data)**0.5

def calculate(data):
    average = grades_average(data)
    std = grades_std_deviation(data)
    return (average, std)


def setConfigurationFile(outputFile, IoI, MNodes, SystemExecSeconds, LatticeOutputVertices):

    file = open('../../configure/JStreamSpinner.conf', 'w')
    wrapper_folder = "wrapper_folder = ./configure/wrapper\n"
    file.write(wrapper_folder)
    file.write("query_index = 0" + "\n")
    file.write("query_source = 1" + "\n")
    file.write("num_queries = 1" + "\n")
    file.write("node_type = 0" + "\n")
    file.write("stream1_window_size = 1000" + "\n")
    file.write("stream2_window_size = 10000" + "\n")
    file.write("stream3_window_size = 1000" + "\n")
    file.write("stream1_input_rate = 0.2" + "\n")
    file.write("stream2_input_rate = 500000" + "\n")
    file.write("stream3_input_rate = 0.2" + "\n")
    file.write("IoI = " + IoI + "\n")
    file.write("SystemExecSeconds = " + SystemExecSeconds + "\n")
    file.write("activate_duration = 0millisecond" + "\n")
    file.write("node_type = 0" + "\n")
    file.write("server_port = 8088" + "\n")
    file.write("worker_command_port = 8085" + "\n")
    file.write("worker_status_port = 8090" + "\n")
    file.write("worker_data_port = 8095" + "\n")
    file.write("simulator_stream_port = 8999" + "\n")
    file.close()

    file2 = open('../../configure/cubify.conf', 'w')

    # Cubify file for 6D synthetic data
    file2.write("DimensionNames = Product, Supplier, Promotion, Customer, Store, SalesPerson" + "\n")
    file2.write("Dimensions = prodID, suppID, promoID, custID, storeID, SalesPersonID" + "\n")
    file2.write("DimensionSizes = 250, 4, 5, 7, 2, 4" + "\n")

    # Cubify file for 4D TPCH data
    #file2.write("DimensionNames = Part, Supplier, Customer, Store" + "\n")
    #file2.write("Dimensions = partKey, suppKey, custKey, storeKey" + "\n")
    #file2.write("DimensionSizes = 250, 4, 5, 7" + "\n")

    file2.write("MMode = Frequency" + "\n")
    file2.write("MVerticesNum = " + MNodes + "\n")
    file2.write("RFMode = Custom" + "\n")
    file2.write("RefFrequencies = 58, 73, 85, 4, 1, 13, 72, 82, 51, 18, 97, 6, 58, 60, 61, 27, 71, 83, 28, 55, 56, 78, 50, 57, 89, 43, 39, 13, 27, 95, 87, 53, 13, 29, 63, 16, 56, 37, 50, 74, 46, 52, 43, 60, 45, 18, 12, 42, 92, 1, 65, 17, 92, 24, 64, 61, 48, 81, 19, 85, 35, 70, 67, 98, 40, 78, 14, 74, 76, 97, 58, 45, 91, 29, 29, 45, 17, 88, 42, 12, 86, 32, 41, 25, 55, 65, 100, 24, 51, 60, 44, 1, 4, 78, 94, 74, 11, 20, 50, 68" + "\n")
    file2.write("MVertices = custKey, partKey | partKey | custKey" + "\n")
    file2.write("LatticeOutputVertices = " + LatticeOutputVertices + "\n")
    file2.write("MaxStorageNumTuples = 2147483647" + "\n")
    file2.write("OptMethod = NumVertices" + "\n")
    file2.write("TimeGrain = Second" + "\n")
    file2.write("IoA = " + IoI + "\n")
    file2.close()

    append(outputFile, "Approach: Unified StreamingCube")
    append(outputFile, "IoI")
    append(outputFile, "MNodes")
    append(outputFile, "SystemExecSeconds")
    append(outputFile, "LatticeOutputVertices")
    append(outputFile, IoI)
    append(outputFile, MNodes)
    append(outputFile, SystemExecSeconds)
    append(outputFile, LatticeOutputVertices)


def execCmd(cmd):
    r = os.popen(cmd)
    text = r.read()
    r.close()
    #print(text + "\n")
    return text

def runSystem():
	return execCmd('cd ..;cd ..;./main.exe')
	#return execCmd('cd ..;cd ..;./main.exe & /home/jsspinner/dataSimulator/main.exe;')
    #return execCmd('cd ..;cd ..;./main.exe;')

def openTestResultFile():
    file = open('cubifyOpExperiments.txt','w')
    file.truncate()
    file.close()
    file = open('cubifyOpExperiments.txt','w')
    return file

def append(file, line):
    print(line + "\n")
    file.write(line + "\n")

def main():

    file = openTestResultFile()

    #IoIList = ["30000","60000","90000","120000"]
    #SystemExecSecondsList = ["60","120","180","240"]
    IoIList = ["30","60","90","120"]
    SystemExecSecondsList = ["60","120","180","240"]
    #SystemExecSecondsList = ["120"]
    MNodesList = ["6","12","18","24","30"]
    #LatticeOutputVerticesList = ["prodID, suppID", "prodID, suppID | prodID", "prodID, suppID | prodID | suppID", "prodID, suppID | prodID | suppID | prodID, promoID", "prodID, suppID | prodID | suppID | prodID, promoID | promoID, storeID, SalesPersonID"]
    LatticeOutputVerticesList = ["prodID | prodID, suppID | prodID, suppID, promoID | prodID, suppID, promoID, custID | prodID, suppID, promoID, custID, storeID", "prodID | prodID, suppID | prodID, suppID, promoID | prodID, suppID, promoID, custID | prodID, suppID, promoID, custID, storeID | suppID | prodID, promoID | prodID, suppID, custID | prodID, suppID, promoID, storeID | prodID, suppID, promoID, custID, SalesPersonID", "prodID | prodID, suppID | prodID, suppID, promoID | prodID, suppID, promoID, custID | prodID, suppID, promoID, custID, storeID | suppID | prodID, promoID | prodID, suppID, custID | prodID, suppID, promoID, storeID | prodID, suppID, promoID, custID, SalesPersonID | promoID | prodID, custID | prodID, suppID, storeID | prodID, suppID, promoID, SalesPersonID | prodID, suppID, promoID, storeID, SalesPersonID", "prodID | prodID, suppID | prodID, suppID, promoID | prodID, suppID, promoID, custID | prodID, suppID, promoID, custID, storeID | suppID | prodID, promoID | prodID, suppID, custID | prodID, suppID, promoID, storeID | prodID, suppID, promoID, custID, SalesPersonID | promoID | prodID, custID | prodID, suppID, storeID | prodID, suppID, promoID, SalesPersonID | prodID, suppID, promoID, storeID, SalesPersonID | custID | prodID, storeID | prodID, suppID, SalesPersonID | prodID, suppID, custID, storeID | prodID, suppID, custID, storeID, SalesPersonID", "prodID | prodID, suppID | prodID, suppID, promoID | prodID, suppID, promoID, custID | prodID, suppID, promoID, custID, storeID | suppID | prodID, promoID | prodID, suppID, custID | prodID, suppID, promoID, storeID | prodID, suppID, promoID, custID, SalesPersonID | promoID | prodID, custID | prodID, suppID, storeID | prodID, suppID, promoID, SalesPersonID | prodID, suppID, promoID, storeID, SalesPersonID | custID | prodID, storeID | prodID, suppID, SalesPersonID | prodID, suppID, custID, storeID | prodID, suppID, custID, storeID, SalesPersonID | storeID | prodID, SalesPersonID | prodID, promoID, custID | prodID, suppID, custID, SalesPersonID | prodID, promoID, custID, storeID, SalesPersonID"]

    #IoIList = ["30000","60000","90000","120000"]
    #SystemExecSecondsList = ["600","1200","1800","2400"]
    #IoIList = ["60000"]
    #SystemExecSecondsList = ["1200"]
    #MNodesList = ["3","6","9","12","15"]

    experimentTimes = 5

    for idx, IoI in enumerate(IoIList):
        append(file, " *********************** IoI : " + IoI +" ***************************")
        SystemExecSeconds = SystemExecSecondsList[idx]
        for MNodes in MNodesList:
            append(file, "+++++++++++++++ MNodes : " + MNodes +" +++++++++++++++")
            for LatticeOutputVertices in LatticeOutputVerticesList:
                append(file, "+++++++++++++++ LatticeOutputVertices : " + LatticeOutputVertices +" +++++++++++++++")
                setConfigurationFile(file, IoI, MNodes, SystemExecSeconds, LatticeOutputVertices)
                storageResultList = []
                queryingResultList = []
                throughputResultList = []
                maintenanceResultList = []
                networkLoadResultList = []
                for i in range(experimentTimes):
                    output = runSystem()
                    outputValues = output.split(",")
                    storage = float (outputValues[0])
                    querying = float (outputValues[1])
                    throughput = float (outputValues[2])
                    maintenance = float (outputValues[3])
                    networkLoad = float (outputValues[4])

                    storageResultList.append(storage)
                    queryingResultList.append(querying)
                    throughputResultList.append(throughput)
                    maintenanceResultList.append(maintenance)
                    networkLoadResultList.append(networkLoad)

                (avgStorage, stdDevStorage) = calculate(storageResultList)
                (avgQuerying, stdDevQuerying) = calculate(queryingResultList)
                (avgThroughput, stdDevThroughput) = calculate(throughputResultList)
                (avgMaintenance, stdDevMaintenance) = calculate(maintenanceResultList)
                (avgNetworkLoad, stdDevNetworkLoad) = calculate(networkLoadResultList)

                append(file, "avgStorage")
                append(file, "stdDevStorage")
                append(file, "avgQuerying")
                append(file, "stdDevQuerying")
                append(file, "avgThroughput")
                append(file, "stdDevThroughput")
                append(file, "avgMaintenance")
                append(file, "stdDevMaintenance")
                append(file, "avgNetworkLoad")
                append(file, "stdDevNetworkLoad")
                append(file, str(avgStorage))
                append(file, str(stdDevStorage))
                append(file, str(avgQuerying))
                append(file, str(stdDevQuerying))
                append(file, str(avgThroughput))
                append(file, str(stdDevThroughput))
                append(file, str(avgMaintenance))
                append(file, str(stdDevMaintenance))
                append(file, str(avgNetworkLoad))
                append(file, str(stdDevNetworkLoad))

                file.flush();


    file.close()
    print("all tests finished\n")
main()
