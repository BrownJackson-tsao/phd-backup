/********************************************************************************
 *   Copyright (C) 2011 by Jeppe Rishede 					*
 *   jenslyn42@gmail.com							*
 *										*
 *   All rights reserved.							*
 *										*
 *   Redistribution and use in source and binary forms, with or without 	*
 *   modification, are permitted provided that the following conditions are met:*
 *   Redistributions of source code must retain the above copyright notice,	*
 *   this list of conditions and the following disclaimer. 			*
 *   Redistributions in binary form must reproduce the above copyright notice,	*
 *   this list of conditions and the following disclaimer in the documentation	*
 *   and/or other materials provided with the distribution. 			*
 *   Neither the name of the <ORGANIZATION> nor the names of its contributors 	*
 *   may be used to endorse or promote products derived from this software 	*
 *   without specific prior written permission					*
 *                                                                         	*
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   	*
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     	*
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 	*
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER	*
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 	*
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,   	*
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    	*
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 	*
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  	*
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS    	*
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          	*
 ********************************************************************************/


#include "Setting.h"
#include "RoadGraph.h"
#include "Cache.h"

#include "Competitors.h"
#include "ProtoStudy.h"
#include "Probcache.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define debug true


class TestObject {
public:
	TestObject(TestSetting settings);

	~TestObject() {
		delete test;
	};

	void runStaticTest();
	void printResults();

	AbstractCache *test;

private:
	TestSetting ts;
	double queryTime;
};


TestObject::TestObject(TestSetting settings) {
	ts = settings;


	cout << "TestObject:: constructor: " << MatchEnumString(ALGO_ENUM,ts.testAlgo) << " test choosen" <<endl;

	switch( ts.testAlgo ){
		case ALGO_NONE:
			ts.cacheSize = 0;	// a special case with 0 cacheSize
			test = new LRU(ts);
			break;
		case ALGO_LRU:
			test = new LRU(ts);
			break;
		case ALGO_SCACHE:
			test = new Scache(ts);
			break;
		case ALGO_SPC:
		case ALGO_SPCplus:
		case ALGO_SPCstar:
			test = new Probcache(ts);
			break;
        case ALGO_HQF:
			test = new HQF(ts);
			break;
        case ALGO_HQFLRU:
			test = new HybridHQFLRU(ts);
			break;
        case ALGO_ORACLE:
			test = new Oracle(ts);
			break;
	}
}

void TestObject::runStaticTest() {
	srand(0);	srand48(0);	// fix the random seed

	if (debug) cout << "TestObject::runStaticTest: static test started" <<endl;

	test->buildCache();
	if (debug) cout << "TestObject::runStaticTest: queries read" <<endl;

 	clock_t start = clock();
 	test->runQueryList();
 	queryTime = (double(clock()-start)/CLOCKS_PER_SEC);

  	if (debug) cout << "TestObject::runStaticTest: static test ended" <<endl;
 	printResults();

 	RoadGraph::mapObject(ts)->resetRoadGraph();
}

void TestObject::printResults() {
	///Console output


	// after the following update, we just use "ts" as "test->ts"
	ts = test->ts;

	unsigned long numNodeVisits = RoadGraph::mapObject(ts)->numNodeVisits;
	int ssspCalls = RoadGraph::mapObject(ts)->ssspCalls;

	cout << "\n\n--------------------------" << endl;
	cout << "QueryTime:\t" << queryTime << " sec" << endl;
	cout << "CacheHits:\t" << test->getCacheHits() << "(" << test->getTotalDijkstraCalls() << ")" << endl;
	cout << "SPcalls:\t" << ssspCalls << endl;
	cout << "NodesVisited:\t" << numNodeVisits << endl;

	//cout << "Class:\t" << typeid(*test).name() <<  endl;
	cout << "Algorithm:\t" << ts.testAlgo << " " << MatchEnumString(ALGO_ENUM,ts.testAlgo) << endl;
	cout << "Storage:\t" << ts.testStorage << " " << MatchEnumString(STORAGE_ENUM,ts.testStorage) << endl;
	cout << "Scenario:\t" << ts.testScenario << " " << MatchEnumString(ARCH_ENUM,ts.testScenario) << endl;

	cout << "CacheSize:\t" << ts.cacheSize << endl;
	cout << "CacheItems:\t" << ts.getItemsInCache() << endl;

	cout << "Splits:\t" << ts.getSplits() << endl;
	cout << "QueryFile:\t" << ts.testFilePrefix << endl;

	cout << "NonEmptyRegions:\t" << ts.getNonEmptyRegionPairs() << endl;
	cout << "CalcStatTime:\t" << ts.getBuildStatisticsTime() << " sec" <<endl;
	cout << "FillCacheTime:\t" << ts.getFillCacheTime() << " sec" << endl;

    ts.useRange ? cout << "useRange:\t True" << endl : cout << "useRange:\t False" << endl ;
    cout << "range: " << ts.range << endl;
    cout << "testRangetype:\t";
    if(ts.testRangetype == RALG_FAIR){cout << "FAIR" << endl;}
    else if(ts.testRangetype == RALG_NAIVE){cout << "NAIVE" << endl;}
    else cout << "testRangetype (the range search algorithm) is not set correctly" << endl;
    cout << "Number of POI: " << ts.numpoi << endl;
    ts.useSPtree ? cout << "useSPtree:\t True" << endl : cout << "useSPtree:\t False" << endl ;
    ts.skipSPcalc ? cout << "skipSPcalc:\t True" << endl : cout << "skipSPcalc:\t False" << endl ;

	cout << "--------------------------\n\n" << endl;


    bool fileExist = false;
    ifstream fin((ts.getTestName()).c_str());
    if (fin)
		fileExist = true;// check to see if file exists
    fin.close();

    ///file output
	ofstream resultfile;
	resultfile.open((ts.getTestName()).c_str(), ios::out | ios::ate | ios::app);
    if(!fileExist){
        resultfile 	<< "QueryTime\tCacheHits\tDijkstraCalls\tSPcalls\tNodesVisited\t"
					<< "Algorithm\tScenario\t"
					<< "CacheSize\tCacheItems\tSplits\tQueryFile\t"
					<< "NonEmptyRegions\tCalcStatTime\tFillCacheTime\t"
					<< "useRange\trange\tRangetype\tnumPOI\tuseSPtree\tskipSPcalc\t"<< endl;
    }

	// note: "typeid(*test).name()" no longer used
    resultfile 	<< queryTime << "\t"
				<< test->getCacheHits() << "\t"
				<< test->getTotalDijkstraCalls() << "\t"
				<< ssspCalls << "\t"
				<< numNodeVisits << "\t"

				<< MatchEnumString(ALGO_ENUM,ts.testAlgo)  << "\t"
				<< MatchEnumString(ARCH_ENUM,ts.testScenario) << "\t"

				<< ts.cacheSize << "\t"
				<< ts.getItemsInCache() << "\t"
				<< ts.getSplits() << "\t"
				<< ts.testFilePrefix << "\t"

				<< ts.getNonEmptyRegionPairs() << "\t"
				<< ts.getBuildStatisticsTime() << "\t"
				<< ts.getFillCacheTime() << "\t"

				<< ts.useRange << "\t"
				<< ts.range << "\t"
                << MatchEnumString(RALG_ENUM,ts.testRangetype) << "\t"
                << ts.numpoi  << "\t"
                << ts.useSPtree  << "\t"
                << ts.skipSPcalc  << "\t" <<  endl;

    resultfile.close();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int calcAVGpathlengthInCache(std::string fn) {
    int totallength=0, numpaths=0;
    std::string str;
    std::vector<std::string> tokens;

	std::ifstream cacheContent (fn.c_str(), std::ios::in);
	if(cacheContent.is_open()) {
	    while(getline(cacheContent, str)) {
	        if(!str.empty())
                totallength++;
            else
                numpaths++;
	    }
	    cacheContent.close();
	}

    return totallength/numpaths;
}




/*
int               2147483647
unsigned int      4294967295
long              9223372036854775807
unsigned long     18446744073709551615
double             1.79769e+308
*/

void extractTestParameters(TestSetting& ts) {
	ts.testFilePrefix = ts.getConfigString("testFilePrefix");
	ts.testName = ts.getConfigString("testName");

	//1:graph_large, 2: ppi.dat, 3:*.cedge
	ts.inputFileType = ts.getConfigInt("inputFileType");
	ts.splits = ts.getConfigInt("splits");	// for Probcache (SPC)
	ts.scacheQueryType = ts.getConfigInt("scacheQueryType");	// for SCACHE

	ts.cacheSize = ts.getConfigLong("cachesize");	// as number of bits
	ts.testAlgo = (ALGO_CHOICE) ts.getEnumCode(ALGO_ENUM,"testAlgo");
	ts.testScenario = (ARCH_CHOICE) ts.getEnumCode(ARCH_ENUM,"testScenario");

	ts.useDijkstra = ts.getConfigBool("useDijkstra");

    ts.useRange = ts.getConfigBool("useRange");
    ts.range = ts.getConfigInt("range");
    ts.testRangetype = (RALG_CHOICE) ts.getEnumCode(RALG_ENUM, "testRangetype");
    ts.numpoi = ts.getConfigInt("numpoi");

    ts.useSPtree = ts.getConfigBool("useSPtree");
    ts.skipSPcalc = ts.getConfigBool("skipSPcalc");

	// default storage method: the LIST cache
	ts.testStorage = STORE_LIST;
	if ( ts.testAlgo == ALGO_SPCplus )
		ts.testStorage = STORE_GRAPH;
	else if ( ts.testAlgo == ALGO_SPCstar )
		ts.testStorage = STORE_COMPRESS;


	// format: "experiment"_["R{range}"]_"testAlgo"_"testFile (3 letters)".test
	// 		   "experiment" to be added later
	if (ts.getConfigBool("autoTestName")==true) {
		string& tname = ts.testName;
		tname="";
		if(ts.useRange){
            tname.append("R");
            tname.append(ts.getConfigString("range"));
            tname.append("_");
            tname.append(MatchEnumString(RALG_ENUM,ts.testRangetype));
            tname.append("_");
		}
		tname.append(MatchEnumString(ALGO_ENUM,ts.testAlgo));
		tname.append("_");
		tname.append( ts.testFilePrefix, 0, 3);	// first 3 latters of testFile
		if(ts.testScenario == ARCH_SERVER) tname.append("_server");
		if(ts.getConfigBool("useDijkstra") == false) tname.append("_astar");
		tname.append(".test");
	}

}




void ExperimentVaryCacheSize(TestSetting ts) {
	if (ts.getConfigBool("autoTestName")==true) {
		ts.testName.insert(0,"v_cachesize_");
		cout << "(auto) testName: " << ts.testName << endl;
	}

	unsigned long lowCacheSize = ts.getConfigLong("lowCacheSize");
	unsigned long highCacheSize = ts.getConfigLong("highCacheSize");

	for (unsigned long csize = lowCacheSize; csize <= highCacheSize ; csize*=2) {
		ts.cacheSize = csize;
		cout << "*** Now using ts.cacheSize = " << ts.cacheSize << endl;

		TestObject *expTest = new TestObject(ts);
		expTest->runStaticTest();
		delete expTest;
	}
}

void ExperimentVarySplit(TestSetting ts) {
	if (ts.getConfigBool("autoTestName")==true) {
		ts.testName.insert(0,"v_split_");
		cout << "(auto) testName: " << ts.testName << endl;
	}

	int lowSplit = ts.getConfigInt("lowSplit");
	int highSplit = ts.getConfigInt("highSplit");

	for (int splits = lowSplit; splits <= highSplit ; splits+=2) {
		ts.splits = splits;
		cout << "*** Now using ts.splits = " << ts.splits << endl;

		TestObject *expTest = new TestObject(ts);
		expTest->runStaticTest();
		delete expTest;
	}
}

void ExperimentVaryRange(TestSetting ts) {
	if (ts.getConfigBool("autoTestName")==true) {
		ts.testName.insert(0,"v_range_");
		cout << "(auto) testName: " << ts.testName << endl;
	}

	int lowRange = ts.getConfigInt("lowRange");
	int highRange = ts.getConfigInt("highRange");

	for (int range = lowRange; range <= highRange ; range*=2) {
		ts.range = range;
		cout << "*** Now using ts.range = " << ts.range << endl;

		TestObject *expTest = new TestObject(ts);
		expTest->runStaticTest();
		delete expTest;
	}
}


void ExperimentSingle(TestSetting ts) {
	if (ts.getConfigBool("autoTestName")==true) {
		ts.testName.insert(0,"v_single_");
		cout << "(auto) testName: " << ts.testName << endl;
	}

	TestObject *expTest = new TestObject(ts);
	expTest->runStaticTest();
	delete expTest;
}

// Tasks for Ken:
// 0. test network connectivity
// 1. add a row about  "communication cost" (no needed)
// 2. add A* implementation

int main(int argc, char *argv[]) {

	InitEnumMappings();	// initialization

	srand(0);	srand48(0);


cout << "******************************************" << endl;
cout << "int \t\t" << std::numeric_limits<int>::max() << endl;
cout << "unsigned int\t" << std::numeric_limits<unsigned int>::max() << endl;
cout << "long\t\t" << std::numeric_limits<long>::max() << endl;
cout << "unsigned long\t\t" << std::numeric_limits<unsigned long>::max() << endl;
cout << "double\t\t" << std::numeric_limits<double>::max() << endl;
cout << "Rand Max\t\t" << RAND_MAX << endl;
cout << sizeof(long) << " " << sizeof(int) << " " << sizeof(unsigned long) << " " <<sizeof(double) <<endl;
cout << "******************************************" << endl;

	//Test setting
	///Load settings from config.prob
	//TestSetting ts;
	//ts.addConfigFromFile("config.prop");	// load default parameter values
	//ts.addConfigFromCmdLine(argc,argv);		// override parameter values
	//ts.listConfig();		// list the content of the config

	///Load settings from commandline
	TestSetting ts;
	ts.addConfigFromCmdLine(argc,argv);    // get the "configName" parameter from command line
	string configName= ts.getConfigString("configName");

///these lines are used when generating *.rq(test|train) files
///only one can be commented in at a time, and the existing config lines must be commented out
/// the generation calls are currently in constructor of LRU
//	ts.addConfigFromFile( "config_lru_aal_cachesize.prob");
//	ts.addConfigFromFile( "config_lru_bei_cachesize.prob");
    ts.addConfigFromFile(configName.c_str() );    // load default parameter values
	ts.addConfigFromCmdLine(argc,argv);     // override parameter values


	extractTestParameters(ts);
	ts.printSetting();

	string experiment = ts.getConfigString("experiment");
	boost::to_upper(experiment);


    if (experiment.compare("RANGE")==0)
		ExperimentVaryRange(ts);
	else if (experiment.compare("SINGLE")==0)
		ExperimentSingle(ts);
	else if (experiment.compare("SPLIT")==0)
		ExperimentVarySplit(ts);
	else if (experiment.compare("CACHESIZE")==0)
		ExperimentVaryCacheSize(ts);

//cout << "avg path length D4: " << calcAVGpathlengthInCache("level_SPC_scoreLengthDevide_AALD4.cache") << endl;
//cout << "avg path length D14: " << calcAVGpathlengthInCache("level_SPC_scoreLengthDevide_AALD14.cache") << endl;
//cout << "avg path length D18: " << calcAVGpathlengthInCache("level_SPC_scoreLengthDevide_AALD18.cache") << endl;

	return EXIT_SUCCESS;
};
