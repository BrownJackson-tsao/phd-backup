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
#include "Probcache.h"
#define debug false



Probcache::Probcache(TestSetting ts)
{
	this->ts = ts;

	cache.init(ts);

	cacheSize = cache.size();
	numTotalQueries = 0;
	numCacheHits = 0;
	numDijkstraCalls = 0;

	readMapData();

	makePartitions(ts.getSplits());
	calcScoreCounter=0;
}

Probcache::~Probcache() { }



//does not use argument, but does instead get its info from a .train file.
void Probcache::runQueryList() {
	intVector spResult;
	unsigned long existingNodesvisited; //*1* used for keeping track of number of nodes visited by a SP call.

	RoadGraph::mapObject(ts)->resetRoadGraph(); //as the roadgraph object has been used already we need to reset it to clear the statistics.

	BOOST_FOREACH(intPair q, testSTPointPairs) {

		numTotalQueries++;	// track usage info

		if (debug) cout << "one, Probcache::checkCache! :cacheSize:" << (int) cache.size() <<"::"<< endl;

		if (cache.checkCache(q)) { // cacheHit
			numCacheHits++;
		} else {
			if(debug) cout << "three, Probcache::checkCache!" << endl;
			existingNodesvisited = RoadGraph::mapObject(ts)->numNodeVisits; //*1* nodes visited before call
			spResult = RoadGraph::mapObject(ts)->dijkstraSSSP(q.first, q.second, false);
			numDijkstraCalls++;

			cout << "Probcache::queryID: " << numTotalQueries << ": " << RoadGraph::mapObject(ts)->numNodeVisits - existingNodesvisited << endl;
		}

		if(debug) cout << "four, Probcache::checkCache!" << endl;
	}
}


void Probcache::buildCache()
{
	cout<< "2.0 done" << endl;
	double refTime = clock();
	readQueryLogData(QLOG_TEST);
	cout<< "2.1 done, " << getElapsedTime(refTime) << endl;

	readQueryLogData(QLOG_TRAIN);
	cout<< "2.2 done, " << getElapsedTime(refTime) << endl;

	extractStatistics();
	double _build_stat_time=getElapsedTime(refTime);
	ts.setBuildStatisticsTime(_build_stat_time);
	calcScoreCounter=0;
	cout<< "2.3 done, " << _build_stat_time << endl;

	fillCache();
	double _fill_cache_time=getElapsedTime(refTime);
	ts.setFillCacheTime(_fill_cache_time);
	cout<< "2.4 done, " << _fill_cache_time << endl;
}


void Probcache::extractStatistics() {
	int temp, r1,r2,v1,v2;
	int totalTrainingPairsSeen=0;
	intPair p;

	cout << "Probcache::populateProbStructures! calculating training stats" << endl;
	BOOST_FOREACH(intPair c, trainingSTPointPairs) {
		v1 = c.first;
		v2 = c.second;

		if (v1 > v2) {
			temp = v1; v1 = v2 ; v2 = temp;
		}

		if (v1 != v2) {
			r1 = mapNodeid2RegionId(v1);
			r2 = mapNodeid2RegionId(v2);
			if (r1 > r2) {
				temp = r1; r1 = r2 ; r2 = temp;
			}
			p = make_pair(r1,r2);
			if (trainingQueriesPerRegionPair.find(p) == trainingQueriesPerRegionPair.end())
				trainingQueriesPerRegionPair[p] = 1;
			else
				trainingQueriesPerRegionPair[p] =+ 1;
			totalTrainingPairsSeen++;
		}
	}
	cout << totalTrainingPairsSeen << " pairs, ... Done" << endl;
}


bool xxCompfunc(Point i,Point j) {return (i.first<j.first);} //sort based on x values
bool yyCompfunc(Point i,Point j) {return (i.second<j.second);} //sort based on y values

bool Probcache::makePartitions(int splits) {
	int axis = 0;
	cout << "Probcache::makePartitions start" << endl;
	double mapXmin,mapXmax,mapYmin,mapYmax;

	vector<Region> regionsVector;

	sort(points.begin(), points.end(), xxCompfunc);
	mapXmin = points.front().first;
	mapXmax = points.back().first;
	sort(points.begin(), points.end(), yyCompfunc);
	mapYmin = points.front().second;
	mapYmax = points.back().second;

	Region reg(0,mapXmin,mapXmax,mapYmin,mapYmax);
	reg.points=points;

	regionsVector.push_back(reg);
	cout << "two. Probcache::makePartitions splitting start: " << splits << " splits." << endl;
	for(int i=0; i<splits; i++) {
		split(regionsVector, axis);
		axis++;
	}
	cout << "three, Probcache::makePartitions set id of each region" << endl;

	int total_points=0;
	// update the region_id of points here
	for (uint region_id=0; region_id<regionsVector.size(); region_id++) {
		Region& r = regionsVector[region_id];
		r.id = region_id;
		mapRegions[region_id] = r;

		BOOST_FOREACH(Point p, r.points) {
			Point2regionidMap[p] = region_id;
		}

		total_points+=r.points.size();
	}
	cout << "total points: " << total_points << ", " << points.size() << endl;

	cout << "mapRegions size: " << mapRegions.size() << endl;

	// compute "nodeid2regionid", to be used for calcScore function
	nodeid2regionid.clear();
	for (int nid=0;nid<mapSize;nid++) {
		int rid = mapNodeid2RegionId(nid);
		nodeid2regionid.push_back( rid );
	}


	if (!mapRegions.empty())
		return true;
	else
		return false;
}

void Probcache::split(std::vector<Region>& regions, int axis) {
	std::vector<Region > tmpRegions;

	BOOST_FOREACH (Region reg, regions) {
        int size = reg.points.size();
		if (size == 0)
			continue;

		if (axis%2==0)
			sort(reg.points.begin(), reg.points.end(), xxCompfunc);
		else
			sort(reg.points.begin(), reg.points.end(), yyCompfunc);

		Region leftReg(axis,reg.xmin,reg.xmax,reg.ymin,reg.ymax);
		Region rightReg(axis,reg.xmin,reg.xmax,reg.ymin,reg.ymax);

		int middle = size/2;
		leftReg.points.assign( reg.points.begin(), reg.points.begin()+middle );
		rightReg.points.assign( reg.points.begin()+middle, reg.points.end() );

		if(axis%2==0) {
			leftReg.xmax = leftReg.points.back().first;	// left part split
			rightReg.xmin = rightReg.points.front().first; 	// right part split
		} else {
			leftReg.ymax = leftReg.points.back().second;		// left part split
			rightReg.ymin = rightReg.points.front().second;	// right part split
		}

		tmpRegions.push_back(leftReg);
		tmpRegions.push_back(rightReg);
	}

	regions.clear();
	regions=tmpRegions;

	tmpRegions.clear();	// clear temporary storage
}



// Note: the following function is called heavily
// optimize this function tomorrow: use an "array" for direct "node-to-region" access!
int Probcache::mapNodeid2RegionId(int nid) {
    if(nodeid2Point.find(nid) != nodeid2Point.end())
        return mapPoint2RegionId(nodeid2Point[nid]);
    else {
        cout << "node " << nid << " is not in nodeid2Point"  << endl;
        return -1;
    }
}

int Probcache::mapPoint2RegionId(Point coord) {
	if(Point2regionidMap.find(coord) != Point2regionidMap.end())
		return Point2regionidMap[coord];
	else {
		cout << "Point (" << coord.first << "," << coord.second << ") does not fit in a region" << endl;
		return -1;
	}
}


double Probcache::calcScore(intVector& spResult, intPairSet& vSeen) {
	double score = 0.0;
	bool isEmpty_vSeen=(vSeen.size()==0);

	calcScoreCounter++;

	// analyze the continuous region in the path
	int last_region=-2;
	vector<intPair> regionset;
	for (uint i=0;i<spResult.size();i++) {
		int nid = spResult[i];
		int rid = nodeid2regionid[nid];
		if (rid!=last_region) {
			last_region=rid;
			regionset.push_back(make_pair(rid,1));
		} else {
			regionset.back().second++;
		}
	}

	// do not repeat the same path
	int start_iter1=0;
	for (uint a=0;a<regionset.size();a++) {
		int r1=regionset[a].first;
		int length_iter1=regionset[a].second;

		// incorrect to use "a" directly because of "wrong offset
		int start_iter2=0;
		for (uint b=0;b<regionset.size();b++) {	// include "a" (different nodes in the same region)
			int r2=regionset[b].first;
			int length_iter2=regionset[b].second;
			if (b<a) {	// correct skipping
				start_iter2+=length_iter2;
				continue;
			}

			int temp_count=0;
			for (int i=0;i<length_iter1;i++) {
				int nid1 = spResult[start_iter1+i];
				for (int j=0;j<length_iter2;j++)
					if (start_iter1+i<start_iter2+j) 	// avoid duplicate check
					{
						if (isEmpty_vSeen)
							temp_count++;
						else {
						int nid2 = spResult[start_iter2+j];
						intPair nodepair = (nid1<nid2)? make_pair(nid1,nid2):make_pair(nid2,nid1);
						if (vSeen.find(nodepair)==vSeen.end())  // if not in cache
							temp_count++;
						}
					}
			}

			if (temp_count>0) {
				intPair regionpair = (r1 < r2)? make_pair(r1,r2):make_pair(r2,r1);
				double temp_score=0;
				if (trainingQueriesPerRegionPair.find(regionpair) != trainingQueriesPerRegionPair.end())
					temp_score = trainingQueriesPerRegionPair.at(regionpair);
				score = score + temp_count*temp_score;
			}
			start_iter2+=length_iter2;
		}
		start_iter1+=length_iter1;
	}

	// final update
	if (spResult.size()>0) {
		if (ts.testScenario == ARCH_SERVER)
			score = score * pow(spResult.size(),2);
		else
			score = score / spResult.size();
	}
	return score;
}

void Probcache::fillCache()
{
	fillCacheFromQueriesFileByStatistics();
}

void Probcache::fillCacheFromQueriesFileByStatistics() {
    intVector spResult;

}

intPair Probcache::pickSTpair(intPair regionPair) {
	intPair ansPair;

	vector<intPair>& nodePairVector = regionPair2nodePairVector[regionPair];
	if (debug)
		cout << "nodepairVector size: " << nodePairVector.size() << endl;

	if (nodePairVector.size()>0) { // pick path WITHOUT REPLACEMENT
		///random choice
		int slot=(int)rand()%nodePairVector.size();
		ansPair=nodePairVector[slot];

		nodePairVector[slot]=nodePairVector.back();	// WITHOUT REPLACEMENT
		nodePairVector.pop_back();
	} else { // random choice
		intVector& nodeVectorRegion1 = regionid2nodeidVector[regionPair.first];
		intVector& nodeVectorRegion2 = regionid2nodeidVector[regionPair.second];

		///random choice
		int nid1 = nodeVectorRegion1[(int)rand()%nodeVectorRegion1.size()];
		int nid2 = nodeVectorRegion2[(int)rand()%nodeVectorRegion2.size()];

		if (nid1 < nid2)
			ansPair=make_pair<int,int>(nid1,nid2);
		else
			ansPair=make_pair<int,int>(nid2,nid1);
	}

	return ansPair;
}

void Probcache::buildRegionId2NodeidVector() {
	BOOST_FOREACH(intPointMap::value_type ic, nodeid2Point) {
		int rid = mapPoint2RegionId(ic.second);
		if (regionid2nodeidVector.find(rid) == regionid2nodeidVector.end())
			regionid2nodeidVector[rid] = vector<int>();
		regionid2nodeidVector[rid].push_back(ic.first);
	}
}

void Probcache::buildRegionpair2NodepairVector() {
	int rid1,rid2,temp;
	BOOST_FOREACH(intPair c, trainingSTPointPairs) {
		rid1 = mapNodeid2RegionId(c.first);
		rid2 = mapNodeid2RegionId(c.second);
		if (rid1 > rid2) {
			temp = rid1; rid1 = rid2; rid2 = temp;
		}

		intPair ip = make_pair(rid1,rid2);
		if (regionPair2nodePairVector.find(ip) == regionPair2nodePairVector.end())
			regionPair2nodePairVector[ip] = vector<intPair>();
		regionPair2nodePairVector[ip].push_back(c);
	}
}
