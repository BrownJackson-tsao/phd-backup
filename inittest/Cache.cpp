/****************************************************************************************
 *   Copyright (C) 2011 by Jeppe Rishede 						*
 *   jenslyn42@gmail.com								*
 *											*
 *   All rights reserved.								*
 *											*
 *   Redistribution and use in source and binary forms, with or without 		*
 *   modification, are permitted provided that the following conditions are met:	*
 *   Redistributions of source code must retain the above copyright notice,		*
 *   this list of conditions and the following disclaimer. 				*
 *   Redistributions in binary form must reproduce the above copyright notice,		*
 *   this list of conditions and the following disclaimer in the documentation		*
 *   and/or other materials provided with the distribution. 				*
 *   Neither the name of the <ORGANIZATION> nor the names of its contributors 		*
 *   may be used to endorse or promote products derived from this software 		*
 *   without specific prior written permission						*
 *                                                                         		*
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   		*
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     		*
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 		*
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER		*
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 		*
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,   		*
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    		*
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 		*
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  		*
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS    		*
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          		*
 ***************************************************************************************/

#include "Cache.h"

#define debug (false)

//////////////////////////////////////////////////////////////////////////////////////////

CacheItem::CacheItem(){};

CacheItem::CacheItem(int key, intVector& item)
{
	this->id = key;
	this->accessKey = key;
	this->item = item;		// copy
	this->size = item.size();

	this->s = -1;
	this->t = -1;

	if (item.size()>0) {
		this->s = item.front();
		this->t = item.back();
	}

	this->score = -1;
};

CacheItem::CacheItem(int key, intVector& item, int queryStartNode, int queryTargetNode)
{
	this->id = key;
	this->accessKey = key;
	this->item = item;		// copy
	this->size = item.size();
	this->s = queryStartNode;
	this->t = queryTargetNode;
	this->score = -1;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////


void AbstractCache::readMapData() {
	cout << "one, Base::readMapData start!" << endl;
	mapSize = RoadGraph::mapObject(ts)->getMapsize();
	cout << "mapsize: " << mapSize << endl;
	string mapFile = ts.getTestFile();
	std::pair<double, double> tmpPair;
	string str;
	std::vector<string> tokens;

	mapFile.replace ((mapFile.size()-4), 4, "node"); //change file extention from .cedge to .cnode
	cout << "mapfile: " << mapFile << endl;
	ifstream in_data (mapFile.c_str(), ios::in); //*.cnode file

	//read in the mapping between Points and node ids from *.cnode file
	if (in_data.is_open()) {
		for(int i = 0; i < mapSize; i++)
		{
			getline(in_data, str);
			boost::algorithm::split(tokens, str, boost::algorithm::is_space());
//cout << tokens[0] << ","<< tokens[1] << ","<< tokens[2] << " : " << endl;
			tmpPair = std::make_pair(atof(tokens[1].c_str()),atof(tokens[2].c_str()));
			Point2Nodeid[tmpPair] = atoi(tokens[0].c_str());
//			BOOST_FOREACH(intPairIntMap::value_type pnid, Point2Nodeid)
//			{
//			    cout << "[(" << pnid.first.first << "," << pnid.first.second << "):" << pnid.second << "] ";
//
//			}
//cout << tmpPair.first << "," << tmpPair.second << ":" << tokens[0] << " ; " << endl;
			nodeid2Point[atoi(tokens[0].c_str())] = tmpPair;
			points.push_back(tmpPair);
			// WARNING: brittle code (error when the file contains one more line!)
		}
	}
	in_data.close();

	cout << "two, Base::readMapData end!: " << Point2Nodeid.size() << endl;
}

void AbstractCache::plotCachePoints(vector<CacheItem>& cm) {
	// obtain parameter from "ts"
	string testbasename=ts.getTestName();
	int numSplits=ts.getSplits();

	cout << "Base::plotCachePoints start!";
	vector<int> sp;

	ofstream of;
	string fn = testbasename;
	string app = "D" + boost::lexical_cast<std::string>(numSplits);
	string app2 = "C" + boost::lexical_cast<std::string>(cacheSize);
	app.append(app2);
 	app.append(".cache");
	fn.replace ((fn.size()-5), 5, app); //change file extention from .test to fnD#splits.cache
	of.open(fn.c_str(), ios::out | ios::ate | ios::app);

	int i=0;
	BOOST_FOREACH(CacheItem ci, cm) {
		sp = ci.item;

		BOOST_FOREACH(int v, sp) {
			if(nodeid2Point.find(v) != nodeid2Point.end()) {
				Point c = nodeid2Point.at(v);
				of << ci.getScore() << " " << c.first << " " << c.second << "\n";
			} else
				cout << "\nBase::plotCachePoints ERROR:  unknown node id." << endl;
		}
		of << endl;
		i++;
	}
	of.close();

	cout << " ... Done!" << endl;
}



bool AbstractCache::plotShortestPaths(QLOG_CHOICE qlog) {
	// obtain parameter from "ts"
	string testbasename=ts.getTestName();
	string app="";
	intPairVector* ptrPointPairs=NULL;

	if (qlog==QLOG_TRAIN) {
		app = "TEST.cache";
		ptrPointPairs=&trainingSTPointPairs;
	} else if (qlog==QLOG_TEST) {
		app = "TRAIN.cache";
		ptrPointPairs=&testSTPointPairs;
	} else {
		printf("*** invalid qlog parameter\n");
		exit(0);
		return false;
	}

	cout << "Base::plotShortestPaths start!";
	intPairVector& stPointPairs=(*ptrPointPairs);
	string fn = testbasename;
	fn.replace ((fn.size()-5), 5, app); //change file extention from .test TEST.cache
	ifstream ifile(fn.c_str());
	if (ifile) { //file already exist
		ifile.close();
		return false;
	}

	ofstream of(fn.c_str(), ios::out | ios::ate | ios::app);

	BOOST_FOREACH(intPair ip, stPointPairs) {
		intVector sp = RoadGraph::mapObject(ts)->dijkstraSSSP(ip.first, ip.second, false);

		BOOST_FOREACH(int v, sp) {
			if(nodeid2Point.find(v) != nodeid2Point.end()) {
				Point c = nodeid2Point.at(v);
				of << c.first << " " << c.second << "\n";
			} else
				cout << "\nBase::plotShortestPaths ERROR:  unknown node id." << endl;
		}
		of << endl;
	}
	cout << " ... Done!" << endl;
	of.close();
	return true;
}


//file on the form:
//record_id, S_id, T_id, S_x, S_y, T_x, T_y.
void AbstractCache::readQueryLogData(QLOG_CHOICE qlog) {
	// extract parameter from "ts"
	string fn=ts.queryFileName;
	string app="";
	intPairVector* ptrPointPairs=NULL;

	if (qlog==QLOG_TRAIN) {
		app = "train";
		ptrPointPairs=&trainingSTPointPairs;
	} else if (qlog==QLOG_TEST) {
		app = "test";
		ptrPointPairs=&testSTPointPairs;
	} else {
		printf("*** invalid qlog parameter\n");
		exit(0);
		return;
	}
	intPairVector& stPointPairs=(*ptrPointPairs);

	std::pair<double, double> firstPair, secondPair;
	int firstPnt, secondPnt, temp;
	string str;
	std::vector<string> tokens;
	fn.replace ((fn.size()-4), 4, app); //change file extention from .test to .train
	ifstream qlogFile (fn.c_str(), ios::in); //*.train file

	cout << "Base::readQueryLogData start: " << fn << ":" << Point2Nodeid.size() << endl;

	//find all pairs of nodeids in the training set to have SP done for them. map nodeids to Points.
	if (qlogFile.is_open()) {
 		if (debug)
			cout << "two, Base::readQueryLogData opened! " << endl;

		while(getline(qlogFile, str)) {
			boost::algorithm::split(tokens, str, boost::algorithm::is_space());
//cout << tokens[0] << ","<< tokens[1] << ","<< tokens[2] << ","<< tokens[3] << ","<< tokens[4] << ","<< tokens[5] << ","<< tokens[6] << endl;
			firstPair = std::make_pair(atof(tokens[3].c_str()),atof(tokens[4].c_str()));
			secondPair = std::make_pair(atof(tokens[5].c_str()),atof(tokens[6].c_str()));

			firstPnt = Point2Nodeid[firstPair];
			secondPnt = Point2Nodeid[secondPair];
cout << "[(" << firstPair.first << "," << firstPair.second << ")(" << secondPair.first << "," << secondPair.second << ")] ";
			if (firstPnt > secondPnt)
				temp = firstPnt; firstPnt = secondPnt; secondPnt = temp;
cout << " - (" << firstPnt << "," << secondPnt << ") " << endl;

		stPointPairs.push_back(std::make_pair(firstPnt,secondPnt));
		}
	}
	qlogFile.close();
	cout << "Base::readQueryLogData end! size:" << stPointPairs.size() << endl;
}

//////////////////////////////////////////////////////////////////////////////////////////

CacheStorage::CacheStorage() { }

CacheStorage::CacheStorage(TestSetting ts) {
	init(ts);
}

void CacheStorage::init(TestSetting ts)
{
	// assume that checking has been done for "testStorage" elsewhere
	testStorage = ts.testStorage;
	cacheSize = ts.cacheSize;
	cacheUsed = 0;
	numberOfNodes = 0;
	totalEntriesInCompressedBitsets = 0;

	mapSize = RoadGraph::mapObject(ts)->getMapsize();
	if (testStorage!=STORE_LIST)
		invertedLists=new intVector[mapSize];
	else
		invertedLists=NULL;
}

bool CacheStorage::insertItem(CacheItem ci) {
	if (!hasEnoughSpace(ci)) return false;

	int path_id = numberOfItemsInCache();

	cache.push_back(ci);
	updateCacheUsed(ci);

	// update inverted list
	// note that ci.id is not sorted (need to sort them later?)
	if (testStorage!=STORE_LIST) {
		intVector& sp = ci.item;
		BOOST_FOREACH(int v, sp) {
			assert(v>=0&&v<mapSize);
			invertedLists[v].push_back(path_id);
		}
	}
	return true;
}

bool CacheStorage::insertItemWithScore(CacheItem ci, double score) {
	ci.setScore(score);
	return insertItem(ci);
}

bool CacheStorage::checkCache(intPair query) {
	vector<int> cItem;

	int s = query.first;
	int t = query.second;
	assert(s>=0&&s<mapSize);
	assert(t>=0&&t<mapSize);


	if (testStorage==STORE_LIST) {
		BOOST_FOREACH(CacheItem& ci, cache ) {
			cItem = ci.item;
			if(find(cItem.begin(),cItem.end(), s) != cItem.end() && find(cItem.begin(),cItem.end(), t) != cItem.end())
				return true;
		}
	} else {
		intVector& vecA=invertedLists[s];
		intVector& vecB=invertedLists[t];
		if (vecA.size()==0 || vecB.size()==0)
			return false;

		// *** merge algorithm (for sorted arrays)
		int posA=0,posB=0;
		while (posA<vecA.size() && posB<vecB.size()) {
			if (vecA[posA]<vecB[posB])
				posA++;
			else if (vecA[posA]>vecB[posB])
				posB++;
			else {// equal path_id
				return true;
			}
		}
	}
	return false;
}

//assumes cache item ci has NOT been added to vector<CacheItem> cache
bool CacheStorage::hasEnoughSpace(CacheItem ci)
{
    return hasEnoughSpace(ci.item);
}

bool CacheStorage::hasEnoughSpace(intVector& sp) {
    if(testStorage == STORE_GRAPH) {

		int newNodes = 0; //nodes in ci which is not already in graph
		BOOST_FOREACH(int v, sp) {
			if (nodeIdsInCache.find(v) == nodeIdsInCache.end())
				newNodes++;
		}

		if ( (nodeIdsInCache.size() + newNodes ) * ( NODE_BITS + BIT*(cache.size()+1)) <= cacheSize )
			return true;

	} else if(testStorage == STORE_LIST) {

		if ( cacheUsed + sp.size()*NODE_BITS < cacheSize )
			return true;

	} else if(testStorage == STORE_COMPRESS) {
		if ( cacheUsed < cacheSize )
			return true;
	}
	return false;
}

//assumes cache item ci has already been added to vector<CacheItem> cache
void CacheStorage::updateCacheUsed(CacheItem ci) {

	if (testStorage == STORE_GRAPH) {

		// add a bitset for each new node
		intVector& sp = ci.item;

		BOOST_FOREACH(int v, sp) {
			if (nodeIdsInCache.find(v) == nodeIdsInCache.end()) {
				 //set all bits to zero in the bitmap for the first cache.size()-1 bits
				nodeIdsInCache[v] = boost::dynamic_bitset<>(cache.size()-1);;
			}
		}

		BOOST_FOREACH(intDBitset::value_type nb, nodeIdsInCache) {
			int nid = nb.first;
			if (find(sp.begin(), sp.end(), nid) != sp.end())
				nodeIdsInCache[nid].push_back(1);
			else
				nodeIdsInCache[nid].push_back(0);
		}

		cacheUsed =  nodeIdsInCache.size() * (NODE_BITS + BIT*cache.size()) ;
		numberOfNodes = nodeIdsInCache.size();

	} else if (testStorage == STORE_LIST) {

		cacheUsed = cacheUsed + ci.size*NODE_BITS;
		numberOfNodes = numberOfNodes + ci.size;

	} else if (testStorage == STORE_COMPRESS) {

	    pidSets.insertPath(ci.item);

        int num_paths = cache.size();
        int num_tokens = pidSets.GetNumTokens();

        int pid_bits = ceil( log( num_paths ) / log(2) );
        int token_bits = ceil( log(  num_tokens ) / log(2) );

        cacheUsed =  	pidSets.getNumNodes() * ( NODE_BITS + token_bits ) +
						pidSets.GetTotalNumItems()  * ( pid_bits + 1 )  +  num_tokens * token_bits * 2 ;
		numberOfNodes = pidSets.getNumNodes();

	}

}

void CacheStorage::printNodesTokensPaths()
{
    pidSets.printNodesTokensPaths();
}


//////////////////////////////////////////////////////////////////////////////////////////


CompressedPidTokens::CompressedPidTokens()
{
       _num_token = 0;
       pathID =0;
       _total_pid_items=0;
}

CompressedPidTokens::~CompressedPidTokens()
{
    //dtor
}

// pre-condition: assume that "path_id" must be larger than any existing id in "vecpair"
void CompressedPidTokens::insertVecPair( intPairVector& vecpair, int path_id )
{
    if(debug) cout << "compressedPidTokens::insertVecPair! Start";
	if (vecpair.size()==0) {
		vecpair.push_back(make_pair(path_id,path_id));
		_total_pid_items++;	// single item only
	} else {
		intPair tmppair=vecpair.back();
		if (tmppair.second + 1 == path_id) {
			if (tmppair.first == tmppair.second)	// a item to a pair
				_total_pid_items++;
			vecpair.pop_back();		// remove the last pair
			tmppair.second=path_id;	// update that pair
		} else {
			tmppair=make_pair(path_id,path_id);	// make a new pair
			_total_pid_items++;	// single item only
		}
		vecpair.push_back(tmppair);
	}
	if(debug) cout << " ... Done" << endl;
}

void CompressedPidTokens::printVecPair( intPairVector& vecpair )
{
    if(debug) cout << "compressedPidTokens::printVecPair! Start";
	BOOST_FOREACH(intPair curpair, vecpair)
	{
		cout << curpair.first << "-" << curpair.second << " ";
	}
	cout << endl;
    if(debug) cout << "compressedPidTokens::printVecPair! Done" << endl;

}

void CompressedPidTokens::printNodesTokensPaths()
{
	if(debug) cout << "compressedPidTokens::printNodesTokensPaths! Start";

	cout << "*** " << NodeTokens.size() << " nodes ***" << endl;

	BOOST_FOREACH(intMap::value_type nt, NodeTokens)
	{
	    cout << nt.first << ": " << nt.second << "$ " << endl;
	}

	cout << "*** " << TokenCompressPids.size() << " tokens ***" << endl;

	boost::unordered_map<int,CompressPidType*>::iterator itB;

	for (itB=TokenCompressPids.begin();itB!=TokenCompressPids.end();itB++)
	{
		cout << itB->first << "$: ";

		CompressPidType* comppids=itB->second;
		if (comppids->prev_token<0)
			cout << "[root] ";
		else
			cout << "[" << comppids->prev_token  << "$^" << comppids->prev_size << "] ";
		printVecPair( comppids->vecpair );
	}

	cout << "*** " << GetTotalNumItems() << " total items ***" << endl;

	if(debug) cout << "compressedPidTokens::printNodesTokensPaths! Done" << endl;
}

void CompressedPidTokens::insertCompressedPid( int path_id, intVector& tmpModify, CompressPidType* prevcomppath )
{
    if(debug) cout << "compressedPidTokens::insertCompressedPid! Begin ";
	int next_token=GenerateToken();

	// create a compressed path
	CompressPidType* splitcomppath=new CompressPidType;
	splitcomppath->token=next_token;

	if (prevcomppath!=NULL) {
		splitcomppath->prev_token=prevcomppath->token;
		splitcomppath->prev_size=prevcomppath->vecpair.size();
	} else {
		splitcomppath->prev_token=-1;
		splitcomppath->prev_size=0;
	}

	insertVecPair(splitcomppath->vecpair,path_id);
	splitcomppath->vecnodes.assign(tmpModify.begin(),tmpModify.end());

	// update the structures
	TokenCompressPids[next_token]=splitcomppath;
	BOOST_FOREACH(int nodeid, tmpModify)
		NodeTokens[nodeid]=next_token;

	if(debug) cout << "compressedPidTokens::insertCompressedPid! Done " << endl;
}

void CompressedPidTokens::insertPath(vector<int>& vecpath)
{
    if(debug) cout << "compressedPidTokens::insertPath! Begin ";
	vector<int> newnodes,oldnodes;	// temporary vectors
	vector<int> tmpRemain,tmpModify;
	int path_id = pathID++;

	BOOST_FOREACH(int nodeid, vecpath)
	{
		if (NodeTokens.find(nodeid)==NodeTokens.end())
			newnodes.push_back(nodeid);
		else
			oldnodes.push_back(nodeid);
	}

	// for new node
	if (newnodes.size()>0) {
		insertCompressedPid( path_id, newnodes, NULL );
	}

	while (oldnodes.size()>0) {
		tmpRemain.clear();	// init temp vectors to be used in this loop
		tmpModify.clear();

		assert(NodeTokens.find(oldnodes[0])!=NodeTokens.end());	// must be true
		int prevtoken=NodeTokens[oldnodes[0]];


		CompressPidType* prevcomppath=TokenCompressPids[prevtoken];

		// only consider the nodes with "prevtoken"
		BOOST_FOREACH(int nodeid, prevcomppath->vecnodes)
		{
			vector<int>::iterator it=find(oldnodes.begin(),oldnodes.end(), nodeid);
			if (it != oldnodes.end()) {
				oldnodes.erase(it);
				tmpModify.push_back(nodeid);
			} else {
				tmpRemain.push_back(nodeid);
			}
		}

		assert(tmpModify.size()>0);

		if (tmpRemain.size()==0) {
			// just change "prevcomppath"
			insertVecPair(prevcomppath->vecpair,path_id);
		} else {
			prevcomppath->vecnodes.clear();
			prevcomppath->vecnodes.assign(tmpRemain.begin(),tmpRemain.end());

			insertCompressedPid( path_id, tmpModify, prevcomppath );
		}
	}


	// clear temporary vectors
	newnodes.clear();
	oldnodes.clear();
	tmpRemain.clear();
	tmpModify.clear();

	if(debug) cout << "compressedPidTokens::insertPath! Done " << endl;
}

//////////////////////////////////////////////////////////////////////////////////////////
