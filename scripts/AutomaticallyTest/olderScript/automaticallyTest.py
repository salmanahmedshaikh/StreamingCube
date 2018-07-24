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


def setConfigurationFile( resultFile, queryIndex, joinSelectivity, stream1WindowSize,stream2WindowSize,stream3WindowSize,stream4WindowSize,stream1InputRate,stream2InputRate,stream3InputRate,stream4InputRate, activateDuration, numQueries):
    
    wrapper_folder = "wrapper_folder = ./configure/wrapper\n"
    file = open('../../configure/JStreamSpinner.conf', 'w')
    file.write(wrapper_folder)
    file.write("query_index = " + queryIndex + "\n")
    #file.write("join_selectivity = " + joinSelectivity + "\n")
    file.write("stream1_window_size = " + stream1WindowSize + "\n")
    file.write("stream2_window_size = " + stream2WindowSize + "\n")
    file.write("stream3_window_size = " + stream3WindowSize + "\n")    
    file.write("stream1_input_rate = " + stream1InputRate + "\n")
    file.write("stream2_input_rate = " + stream2InputRate + "\n")
    file.write("stream3_input_rate = " + stream3InputRate + "\n")    
    file.write("activate_duration = " + activateDuration + "\n")
    file.write("num_queries = " + numQueries + "\n")
    file.write("node_type = 0" + "\n")
    file.write("server_port = 8088" + "\n")
    file.write("worker_command_port = 8085" + "\n")
    file.write("worker_status_port = 8090" + "\n")
    file.write("worker_data_port = 8095" + "\n")
    file.write("simulator_stream_port = 8999" + "\n")
    file.close()
    
    #append(resultFile, "queryIndex")
    #append(resultFile, "join_selectivity")
    append(resultFile, "stream1_window_size")
    append(resultFile, "stream2_window_size")
    if queryIndex == "2" or queryIndex == "13":
        append(resultFile, "stream3_window_size")
    append(resultFile, "stream1_input_rate")
    append(resultFile, "stream2_input_rate")
    if queryIndex == "2" or queryIndex == "13":
        append(resultFile, "stream3_input_rate")
    #append(resultFile, "activate_duration")
    #append(resultFile, queryIndex)
    #append(resultFile, joinSelectivity)
    append(resultFile, stream1WindowSize)
    append(resultFile, stream2WindowSize)
    if queryIndex == "2" or queryIndex == "13":
        append(resultFile, stream3WindowSize)
    #append(resultFile, "stream4_window_size")
    #append(resultFile, stream4WindowSize)
    append(resultFile, stream1InputRate)
    append(resultFile, stream2InputRate)
    if queryIndex == "2" or queryIndex == "13":
        append(resultFile, stream3InputRate)
    #append(resultFile, "stream4_input_rate")
    #append(resultFile, stream4InputRate)
    #append(resultFile, activateDuration)
    #append(resultFile, numQueries)


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
    #file = open('system_throughput_multiQuery_sharing.txt','w')
    #file = open('smart_simpleJoinQuery_synthetic.txt','w')
    file = open('traditional_simpleJoinQuery_synthetic.txt','w')
    #file = open('Traditional_throughput_allQueries.txt','w')
    #file = open('Traditional_totalDeque_allQueries.txt','w')
    #file = open('smart_simpleJoinQuery_peopleFlow.txt','w')    
    #file = open('traditional_simpleJoinQuery_peopleFlow.txt','w')
    #file = open('smart_simpleJoinQuery_tsukuMobility.txt','w')    
    #file = open('traditional_simpleJoinQuery_tsukuMobility.txt','w')

    file.truncate()
    file.close()
    #file = open('system_throughput_multiQuery_sharing.txt','w')
    #file = open('smart_simpleJoinQuery_synthetic.txt','w')
    file = open('traditional_simpleJoinQuery_synthetic.txt','w')
    #file = open('Traditional_totalDeque_allQueries.txt','w')
    #file = open('smart_simpleJoinQuery_peopleFlow.txt','w')
    #file = open('traditional_simpleJoinQuery_peopleFlow.txt','w')    
    #file = open('smart_simpleJoinQuery_tsukuMobility.txt','w')    
    #file = open('traditional_simpleJoinQuery_tsukuMobility.txt','w')

    return file

def append(file, line):
    print(line + "\n")
    file.write(line + "\n")


def main():

    file = openTestResultFile()

    #queryIndexList = ["1","2","3","4","99"]
    queryIndexList = ["1"]

    stream1WindowSizeList = ["1", "10", "100", "1000", "10000"]    # window size in milliseconds
    stream2WindowSizeList = ["1", "10", "100", "1000", "10000"]    # window size in num of rows
    stream3WindowSizeList = ["1", "10", "100", "1000", "10000"]

    stream1InputRateList = ["1", "0.33", "0.2", "0.14", "0.11"] # stream arrival interval
    stream2InputRateList = ["100000", "200000", "300000", "400000", "500000"]    
    stream3InputRateList = ["0.2", "0.4", "0.6", "0.8", "1", "1.2"]

    numMultiQueryList = ["10", "20", "30", "40", "50"]

    #joinSelectivityList = ["1","0.1"]
    joinSelectivityList = ["1"]
    activateDurationList = ["0millisecond","1millisecond","10millisecond","100millisecond"]
    #setConfigurationFile(stream1WindowSizeList[0],stream2WindowSizeList[0],stream1InputRateList[0],stream2InputRateList[0],activateDurationList[0])
    #setConfigurationFile(defaultStream1WindowSize,defaultStream2WindowSize,defaultStream1InputRate,defaultStream2InputRate,defaultActivateDuration)
    defaultStream1WindowSize = stream1WindowSizeList[3] # 1000 ms
    defaultStream2WindowSize = stream2WindowSizeList[1] # 10 rows
    defaultStream3WindowSize = stream3WindowSizeList[3]
    defaultStream4WindowSize = "0"

    defaultStream1InputRate = stream1InputRateList[2] # (0.2)5 second
    defaultStream2InputRate = stream2InputRateList[4] # 500000 rows   
    defaultStream3InputRate = stream3InputRateList[0]
    defaultStream4InputRate = "0"

    defaultActivateDuration = activateDurationList[0]
    defaultNumQueries = "1"
    experimentTimes = 5


    for queryIndex in queryIndexList :        
        append(file, " ***********************queryIndex : " + queryIndex +"***************************")
        for joinSelectivity in joinSelectivityList:
            append(file, "+++++++++++++++ join selectivity : " + joinSelectivity +"+++++++++++++++")
            
            #test stream1InputRate
            append(file, " ----------------------stream1InputRate experiment--------------------- ")
            for stream1InputRate in stream1InputRateList:
                append(file , "<<<<<>>>>>>")
                setConfigurationFile(file, queryIndex, joinSelectivity, defaultStream1WindowSize,defaultStream2WindowSize,defaultStream3WindowSize,defaultStream4WindowSize,stream1InputRate,defaultStream2InputRate,defaultStream3InputRate,defaultStream4InputRate,defaultActivateDuration,defaultNumQueries)
                throughputResultList = []
                loadResultList = []
                for i in range(experimentTimes):
                    output = runSystem()
                    throughputLoad = output.split(",")                                        
                    throughput = float (throughputLoad[0])
                    load = float (throughputLoad[1])                    
                    throughputResultList.append(throughput)
                    loadResultList.append(load)
                (avgThroughput, stdDevThroughput) = calculate(throughputResultList)
                (avgLoad, stdDevLoad) = calculate(loadResultList)               
                append(file, "average Load")
                append(file, "stdDev Load")
                append(file, "average Throughput")
                append(file, "stdDev Throughput")
                append(file, str(avgLoad))
                append(file, str(stdDevLoad))
                append(file, str(avgThroughput))
                append(file, str(stdDevThroughput))                
                file.flush();


           #test stream2InputRate
            append(file, " ----------------------stream2InputRate experiment--------------------- ")
            for stream2InputRate in stream2InputRateList:
                append(file , "<<<<<>>>>>>")
                setConfigurationFile(file, queryIndex, joinSelectivity, defaultStream1WindowSize,defaultStream2WindowSize,defaultStream3WindowSize,defaultStream4WindowSize,defaultStream1InputRate,stream2InputRate,defaultStream3InputRate,defaultStream4InputRate,defaultActivateDuration,defaultNumQueries)
                throughputResultList = []
                loadResultList = []
                for i in range(experimentTimes):
                    output = runSystem()
                    throughputLoad = output.split(",")                                        
                    throughput = float (throughputLoad[0])
                    load = float (throughputLoad[1])                    
                    throughputResultList.append(throughput)
                    loadResultList.append(load)
                (avgThroughput, stdDevThroughput) = calculate(throughputResultList)
                (avgLoad, stdDevLoad) = calculate(loadResultList)               
                append(file, "average Load")
                append(file, "stdDev Load")
                append(file, "average Throughput")
                append(file, "stdDev Throughput")
                append(file, str(avgLoad))
                append(file, str(stdDevLoad))
                append(file, str(avgThroughput))
                append(file, str(stdDevThroughput))                
                file.flush();


            #test stream1WindowSize
            append(file, " ----------------------stream1WindowSize experiment--------------------- ")
            for stream1WindowSize in stream1WindowSizeList:
                append(file , "<<<<<>>>>>>")
                setConfigurationFile(file, queryIndex, joinSelectivity, stream1WindowSize,defaultStream2WindowSize,defaultStream3WindowSize,defaultStream4WindowSize,defaultStream1InputRate,defaultStream2InputRate,defaultStream3InputRate,defaultStream4InputRate,defaultActivateDuration,defaultNumQueries)
                throughputResultList = []
                loadResultList = []
                for i in range(experimentTimes):
                    output = runSystem()
                    throughputLoad = output.split(",")                                        
                    throughput = float (throughputLoad[0])
                    load = float (throughputLoad[1])                    
                    throughputResultList.append(throughput)
                    loadResultList.append(load)
                (avgThroughput, stdDevThroughput) = calculate(throughputResultList)
                (avgLoad, stdDevLoad) = calculate(loadResultList)               
                append(file, "averag Load")
                append(file, "stdDev Load")
                append(file, "averge Throughput")
                append(file, "stdDev Throughput")
                append(file, str(avgLoad))
                append(file, str(stdDevLoad))
                append(file, str(avgThroughput))
                append(file, str(stdDevThroughput)) 
                file.flush();

            #test stream2WindowSize
            append(file, " ----------------------stream2WindowSize experiment--------------------- ")
            for stream2WindowSize in stream2WindowSizeList:
                append(file , "<<<<<>>>>>>")
                setConfigurationFile(file, queryIndex, joinSelectivity, defaultStream1WindowSize,stream2WindowSize,defaultStream3WindowSize,defaultStream4WindowSize,defaultStream1InputRate,defaultStream2InputRate,defaultStream3InputRate,defaultStream4InputRate,defaultActivateDuration,defaultNumQueries)
                throughputResultList = []
                loadResultList = []
                for i in range(experimentTimes):
                    output = runSystem()
                    throughputLoad = output.split(",")                                        
                    throughput = float (throughputLoad[0])
                    load = float (throughputLoad[1])                    
                    throughputResultList.append(throughput)
                    loadResultList.append(load)
                (avgThroughput, stdDevThroughput) = calculate(throughputResultList)
                (avgLoad, stdDevLoad) = calculate(loadResultList)               
                append(file, "averag Load")
                append(file, "stdDev Load")
                append(file, "averge Throughput")
                append(file, "stdDev Throughput")
                append(file, str(avgLoad))
                append(file, str(stdDevLoad))
                append(file, str(avgThroughput))
                append(file, str(stdDevThroughput)) 
                file.flush();       
		'''
            #test multiQuery
            append(file, " ----------------------multi query experiments--------------------- ")
            for numQueries in numMultiQueryList:
                append(file , "<<<<<>>>>>>")
                setConfigurationFile(file, queryIndex, joinSelectivity, defaultStream1WindowSize,defaultStream2WindowSize,defaultStream3WindowSize,defaultStream4WindowSize,defaultStream1InputRate,stream2InputRate,defaultStream3InputRate,defaultStream4InputRate,defaultActivateDuration,defaultNumQueries,numQueries)
                resultList = []
                for i in range(experimentTimes):
                    output = runSystem()
                    f = float (output)
                    resultList.append(f)
                (average, standardDeviation) = calculate(resultList)
                append(file, "average_processing_time")
                append(file, "standard_deviation")
                append(file, str(average))
                append(file, str(standardDeviation))
                file.flush();
		    '''


        #break;

    file.close()
    print("all tests finished\n")
main()
