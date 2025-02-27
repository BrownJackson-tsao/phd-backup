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

#ifndef ACACHE_H
#define ACACHE_H

/**
	@author Jeppe Rishede <jenslyn42@gmail.com>
*/

#include "CacheItem.h"
#include "testsetting.h"
#include "utility.h"
#include "compressedpidtokens.h"

#include <boost/foreach.hpp>
#include "boost/unordered_map.hpp"
#include <boost/dynamic_bitset.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <cmath>

class aCache{
public:
	aCache(){ };
	aCache(testsetting ts);

	~aCache(){ };

	bool insertItem(CacheItem ci);
	bool insertItemWithScore(CacheItem ci, double score);
	bool checkCache(int s, int t);
	bool checkCache(std::pair<int,int> query);
	bool checkCache(CacheItem ci);
	bool hasEnoughSpace(CacheItem ci);
    bool hasEnoughSpace(vector<int> sp);
	int getCacheUsed(){return cacheUsed;}
	int size(){return cacheSize;}
	int numberOfNodesInCache(){return numberOfNodes;}
	int numberOfItemsInCache(){return numItems;}
	vector<CacheItem> getCacheContentVector(){return cache;}
	void writeOutBitmaps();
	void printNodesTokensPaths();

private:
	int cacheType;
	double totalEntriesInCompressedBitsets; //only used with COMPRESSED_G_CACHE
	uint cacheSize;
	uint cacheUsed;
	uint numberOfNodes;
	uint numItems;
	boost::unordered_map<int, boost::dynamic_bitset<> > nodeIdsInCache;
	boost::unordered_map<int, vector<intPair> > nodeIdsInCacheCompressed; //used with COMPRESSED_G_CACHE
	vector<CacheItem> cache;

	compressedPidTokens pidSets;

	void updateCacheUsed(CacheItem ci);

};

#endif
