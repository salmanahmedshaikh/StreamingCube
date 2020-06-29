# StreamingCube
StreamingCube: An integrated framework for stream processing and OLAP analysis

StreamingCube documentation can be found here:
http://www.streamspinner.org/streamingcube/documentation.html

Additional Library: To install Google Profiler
sudo apt install libgoogle-perftools-dev

Documentation

    -Installation
    -Supported Queries
    -Configuration Files
    -Compilation and Execution
    -StreamingCube GUI

Installation

    JsSpinner/StreamingCube require Ubuntu OS with the following list of packages
    - apt-get install python-dev gccxml gzip
    - apt-get install xsltproc docbook-xsl docbook-xml
    - apt-get install build-essential g++ autotools-dev libicu-dev build-essential libbz2-dev
    - apt-get install zlib1g-dev
    - apt-get install libevent-dev
    - apt-get install curl libcurl3 libcurl3-dev php5-mcrypt php5-curl

    JsSpinner/StreamingCube require Boost libraries 1.49.0 with the following modifications
    - Download boost libraries 1.49.0 "wget -c http://sourceforge.net/projects/boost/files/boost/1.49.0/boost_1_49_0.tar.gz/download"
    - Extract "tar zxvf boost_1_49_0.tar.gz"
    - Execute the following commands from the "boost_1_49_0" folder to remove the bug from boost libraries:
    - ~/Downloads/boost_1_49_0$ grep -r "TIME_UTC" *
    - ~/Downloads/boost_1_49_0$ find . -name "*.*" -exec sed -i "s/TIME_UTC/TIME_UTC_/g" {} \;
    - Build the libraries ~/Downloads/boost_1_49_0$ ./bootstrap.sh -with-libraries=all
    - Install the libraries ~/Downloads/boost_1_49_0$ sudo ./b2 install

    JsSpinner/StreamingCube require our locally developed Query Parser to parse queries written in Jaql
    - Download the Query Parser and start it as follows before executing the JsSpinner/StreamingCube
    - Install jdk if it is not available
    - sudo apt-get install openjdk-6-jdk
    - Install Eclipse
    - Import jsspinnercompiler-master project into Eclipse
    - Add the existing .Jar files into the project by right clicking on the jsspinnercompiler-master project->Properties->Libraries->Add JARS and adding antlr and gson .Jar files from the lib folder
    - Run the following two files from the specified folders to start the Query Parser
        src/others/Run.java
        src/test/JSSchemaServer.java

Supported Queries

    JsSpinner supports Continuous Queries (CQs) and Smart Continuous Queries (SCQ) while StreamingCube supports two different types of queries: 1) Continuous Queries (CQ) / Smart Continuous Queries (SCQ); 2) OLAP queries. CQs/SCQs in case of StreamingCube can be further subdivided into two types i.e., Ordinary CQs/SCQs, and Cubify CQs/SCQs. Ordinary CQs are supported by most of the existing stream processing engines. Users can register Ordinary CQs to perform typical stream processing operations including select, filter, aggregate, join, group-by, etc. Cubify CQs/SCQs are introduced in this work to support OLAP operations over data streams. Cubify CQs/SCQs make use of a cubify operator in addition to regular CQL operators.
    Sample Cubify CQ

    stream1 = readFromWrapper("LINEORDER");
    dim1 = read("CUSTOMER"); dim2 = read("PART");
    dim3 = read("SUPPLIER"); dim4 = read("STORE");
    tmp1 = stream1 -> window[rows n];
    j1 = join f in tmp1, d1 in dim1
       where f.CustomerID == d1.CustomerID
       into {d1.CName, f.PartID, f.SupplierID, f.StoreID, f.ExtendedPrice};
    j2 = join r in j1, d2 in dim2
       where r.PartID == d2.PartID
       into {r.CName, d2.PName, r.SupplierID, r.StoreID, r.ExtendedPrice};
    j3 = join s in j2, d3 in dim3
       where s.SupplierID == d3.SupplierID
       into {s.CName, s.PName, d3.SuppName, s.StoreID, s.ExtendedPrice};
    j4 = join t in j3, d4 in dim4
       where t.StoreID == d4.StoreID
       into {t.CName, t.PName, t.SuppName, d4.SArea, t.ExtendedPrice};
    j4 -> istream -> cubify(config.conf); 

Configuration Files

The JsSpinner requires one configuration file with the name "JStreamSpinner.conf", while the StreamingCube requires two configuration files with the names "JStreamSpinner.conf" and "cubify.conf" for execution. Sample configuration files along with the description of required parameters are available in the config folder of the JsSpinner/StreamingCube code.
Compiling and Executing JsSpinner/StreamingCube

    Compilation
    – jsspinner/jsonstream$ make
    Execution
    – jsspinner/jsonstream$ ./main.exe
    Once under execution, JsSpinner/StreamingCube can accept queries (CQs/SCQs and OLAP Queries) through the ports specified in the configuration file. 

StreamingCube GUI

We have developed a GUI for StreamingCube to submit CQs/SCQs and to perform interactive OLAP analysis. GUI is web-based and can be downloaded from the download page.
