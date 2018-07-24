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


def setConfigurationFile(outputFile, IoA, MMode, RFMode, windowSize, maxStorage, SystemExecSeconds):

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
    file.write("streamWindowSize = " + windowSize + "\n")
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
    file2.write("DimensionNames = Product, Supplier, Promotion, Customer, Store, SalesPerson" + "\n")
    file2.write("Dimensions = prodID, suppID, promoID, custID, storeID, SalesPersonID" + "\n")
    #file2.write("DimensionSizes = 20, 10, 15, 12, 11, 8" + "\n")
    file2.write("DimensionSizes = 250, 4, 5, 7, 2, 4" + "\n")
    file2.write("MVerticesNum = 10" + "\n")
    file2.write("IoA = " + IoA + "\n")
    file2.write("MMode = " + MMode + "\n")
    file2.write("RFMode = " + RFMode + "\n")
    file2.write("RefFrequencies = 58, 73, 85, 4, 1, 13, 72, 82, 51, 18, 97, 6, 58, 60, 61, 27, 71, 83, 28, 55, 56, 78, 50, 57, 89, 43, 39, 13, 27, 95, 87, 53, 13, 29, 63, 16, 56, 37, 50, 74, 46, 52, 43, 60, 45, 18, 12, 42, 92, 1, 65, 17, 92, 24, 64, 61, 48, 81, 19, 85, 35, 70, 67, 98, 40, 78, 14, 74, 76, 97, 58, 45, 91, 29, 29, 45, 17, 88, 42, 12, 86, 32, 41, 25, 55, 65, 100, 24, 51, 60, 44, 1, 4, 78, 94, 74, 11, 20, 50, 68" + "\n")
    file2.write("MVertices = custKey, partKey | partKey | custKey" + "\n")
    file2.write("MaxStorageNumTuples = " + maxStorage + "\n")
    file2.write("OptMethod = MaxStorage" + "\n")
    file2.write("TimeGrain = Second" + "\n")
    file2.write("LatticeOutputVertices = prodID" + "\n")
    file2.close()

    append(outputFile, "Approach: StreamOLAP")
    append(outputFile, "IoA")
    append(outputFile, "MMode")
    append(outputFile, "RFMode")
    append(outputFile, "windowSize")
    append(outputFile, "maxStorage")
    append(outputFile, "SystemExecSeconds")

    append(outputFile, IoA)
    append(outputFile, MMode)
    append(outputFile, RFMode)
    append(outputFile, windowSize)
    append(outputFile, maxStorage)
    append(outputFile, SystemExecSeconds)


def execCmd(cmd):
    r = os.popen(cmd)
    text = r.read()
    r.close()
    #print("Text:" + text + "\n")
    return text

def runSystem():
	return execCmd('cd ..;cd ..;./main.exe')
	#return execCmd('cd ..;cd ..;(./main.exe & /home/jsspinner/jsspinnerTimeBasedDistributed/jsonstream/main.exe);')

def openTestResultFile():
    file = open('streamOLAPEvaluation_Synthetic.txt','w')
    file.truncate()
    file.close()
    file = open('streamOLAPEvaluation_Synthetic.txt','w')
    return file

def append(file, line):
    print(line + "\n")
    file.write(line + "\n")

def main():

    file = openTestResultFile()

    MModeList = ["Optimized", "Random", "Frequency", "SmallestVertices", "LargestVertices"]
    RFModeList = ["Rand", "AllHigh", "AllLow", "CoarseHigh", "FineHigh", "OneDimHigh"]
    IoAList = ["500","1000","1500"] # size of materialized vertex
    SystemExecSecondsList = ["600","1100","1600"]
    streamWindowSizeList = ["1000","2000","3000","4000","5000"]
    #SystemExecSecondsList = ["15","15","15","15","15"]
    #MaxStorageList = ["10000", "20000", "40000", "60000", "80000", "100000"]
    MaxStorageList = ["3000", "6000", "9000", "12000", "15000"]

    #MNodesList = ["6","12","18","24","30"]
    experimentTimes = 2

    for windowSize in streamWindowSizeList:
        append(file, "+++++++++++++++ WindowSize : " + windowSize +" +++++++++++++++")
        for RFMode in RFModeList:
            append(file, "+++++++++++++++ RFMode : " + RFMode +" +++++++++++++++")
            for maxStorage in MaxStorageList:
                append(file, "+++++++++++++++ MaxStorage : " + maxStorage +" +++++++++++++++")
                for idx, IoA in enumerate(IoAList):
                    append(file, " *********************** IoA : " + IoA +" ***************************")
                    SystemExecSeconds = SystemExecSecondsList[idx]
                    for MMode in MModeList:
                        append(file, "+++++++++++++++ MMode : " + MMode +" +++++++++++++++")

                        setConfigurationFile(file, IoA, MMode, RFMode, windowSize, maxStorage, SystemExecSeconds)

                        numOfMaterializedVerticesList = []
                        storageResultList = []
                        queryingResultList = []
                        throughputResultList = []
                        perTupleProcCostList = []

                        for i in range(experimentTimes):
                            output = runSystem()
                            print("output:" + output)
                            outputValues = output.split(",")

                            numOfMaterializedVertices = float (outputValues[0])
                            storage = float (outputValues[1])
                            querying = float (outputValues[2])
                            throughput = float (outputValues[3])
                            perTupleProcCost = float (outputValues[4])

                            numOfMaterializedVerticesList.append(numOfMaterializedVertices)
                            storageResultList.append(storage)
                            queryingResultList.append(querying)
                            throughputResultList.append(throughput)
                            perTupleProcCostList.append(perTupleProcCost)

                        (avgMVerticesList, stdDevMVerticesList) = calculate(numOfMaterializedVerticesList)
                        (avgStorage, stdDevStorage) = calculate(storageResultList)
                        (avgQuerying, stdDevQuerying) = calculate(queryingResultList)
                        (avgThroughput, stdDevThroughput) = calculate(throughputResultList)
                        (avgProcCost, stdDevProcCost) = calculate(perTupleProcCostList)

                        append(file, "avgMVerticesList")
                        append(file, "stdDevMVerticesList")
                        append(file, "avgStorage")
                        append(file, "stdDevStorage")
                        append(file, "avgQuerying")
                        append(file, "stdDevQuerying")
                        append(file, "avgThroughput")
                        append(file, "stdDevThroughput")
                        append(file, "avgProcCost")
                        append(file, "stdDevProcCost")

                        append(file, str(avgMVerticesList))
                        append(file, str(stdDevMVerticesList))
                        append(file, str(avgStorage))
                        append(file, str(stdDevStorage))
                        append(file, str(avgQuerying))
                        append(file, str(stdDevQuerying))
                        append(file, str(avgThroughput))
                        append(file, str(stdDevThroughput))
                        append(file, str(avgProcCost))
                        append(file, str(stdDevProcCost))

                        file.flush();


    file.close()
    print("all tests finished\n")

main()
