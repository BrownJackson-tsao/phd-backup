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
#ifndef UTILITY_H
#define UTILITY_H

#include <string>

#include "CacheItem.h"
#include "testsetting.h"

#include <boost/foreach.hpp>
#include "boost/unordered_map.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/dynamic_bitset.hpp>

//Used in acache and when defining tests
#define NODE_BITS 32
#define BIT 1
#define GRAPH_CACHE 1
#define LIST_CACHE 2
#define COMPRESSED_G_CACHE 3
#define SERVER_SCENARIO 10
#define PROXY_SCENARIO 11

//enum METHOD{ NONE, OSC, LRU, FIFO, SCACHE, SPC, HQF, RANDCACHE, HQFLRU };



struct region;

typedef std::pair<double, double> coordinate;
typedef std::vector<coordinate > regionlist;
typedef std::pair<int,int> intPair;
typedef std::pair<coordinate, coordinate> coordinatePairs;
typedef std::pair<intPair, intPair > intPairPairs;
typedef std::vector<intPair>  intPairVector;
typedef boost::unordered_map<int, int> intMap;
typedef boost::unordered_map<int, double> intDoubleMap;
typedef boost::unordered_map<int, region> regionMap;
typedef boost::unordered_map<int, coordinate> intCoordinateMap;
typedef boost::unordered_map<int, intPair > intPairMap;
typedef boost::unordered_map<int, std::vector<int> > intVectorMap;
typedef boost::unordered_map<int, std::pair<int, std::vector<int> > > intIntVectorMap;
typedef boost::unordered_map<std::pair<int,int >, int> intPairIntMap;
typedef boost::unordered_map<intPair,int > pairIntMap;
typedef boost::unordered_map<int, CacheItem> intCacheitemMap;
typedef boost::unordered_map<intPair, CacheItem> intPairCacheitemMap;
typedef	boost::unordered_map<int, boost::dynamic_bitset<> > intDBitset;
typedef	boost::unordered_map<coordinate, int> coordinateIntMap;
typedef boost::unordered_map<int, std::pair<intPair, std::vector<int> > > intIntPairVectorMap;

struct region
{
    int id;
    double xmin,xmax,ymin,ymax;
	regionlist points;

    region(){ }
    region(int rid, regionlist pts, double _xmin, double _xmax, double _ymin, double _ymax)
	{
        id = rid;
        xmin = _xmin;
		xmax = _xmax;
		ymin = _ymin;
		ymax = _ymax;
		points = pts;
    }
};

#endif
