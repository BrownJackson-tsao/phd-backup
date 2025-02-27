/*
 * Takes as input a .gr file 
 * outputs a .cedge and a .ddsg file
 * 
 * It adds an extra empty line at the bottom of the output file!
 * 
 * execute as:
 * ./convertGr <filename>.gr
 * 
 */


#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <cstdlib>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>

using namespace std;
typedef boost::unordered_map<int, pair<pair<int,int>, int> > intIntPairIntMap;

/*
 * fileformat 
 * 
 * .gr: 
 * (first non-comment line) p sp [num nodes] [num edges]
 * c [comment]
 * a [nid1 nid2 distance]
 * 
 * .cedge: 
 * linenum nid1 nid2 distance
 * e.g.: 0 0 1 1.182663
 * 
 * .ddsg
 * d [num nodes] [num edges] (first line)
 * [nid1] [nid2] [distance] [direction] 
 * direction types:
 * 0 = open in both directions
 * 1 = open only in forward direction (from s to t)
 * 2 = open only in backward direction (from t to s)
 * 3 = closed
*/
int main(int argc, char *argv[]) {
  if(argc !=2){
    cout << "need to give a .gr file as input: " << argv[0] << " filename.gr" << endl; 
    return -1;
  }
  
  string fn = argv[1];
  string str;
  std::vector<string> tokens;
  boost::unordered_map<int, std::pair<std::pair<int,int>, int > > entry;
  int i=0, numNodes=0, numEdges=0;
  
  ifstream in_data (fn.c_str(), ios::in);
  cout << "FN: " <<fn << endl;
  if(in_data.is_open()){
    while(getline(in_data, str))
    {
      boost::algorithm::split(tokens, str, boost::algorithm::is_space());
      if (tokens[0].compare("a") == 0){
	entry[i] = make_pair<pair<int,int>, int>(make_pair<int,int>(atoi(tokens[1].c_str()), atoi(tokens[2].c_str())), atoi(tokens[3].c_str()));
	i++;
      }
//       else if(tokens[0].compare("c") == 0)
	//handle comments
      else if(tokens[0].compare("p") == 0){
	numNodes = atoi(tokens[2].c_str());
	numEdges = atoi(tokens[3].c_str());
      }
	//use info about number of nodes and edges
    }
    cout << "Reading file Done!" << endl;
    in_data.close();
  }
  
  ///file output .cedge
  i=0;
  fn.replace ((fn.size()-2), 5, "cedge"); //change file extention from .gr to .cedge
  ofstream resultfile;
  resultfile.open(fn.c_str(), ios::out);
  
  BOOST_FOREACH(intIntPairIntMap::value_type it, entry){
    resultfile <<it.first << " " <<  it.second.first.first << " " << it.second.first.second << " " << it.second.second << endl;
    i++;
  }
  cout << fn << " written.\n" << i << " lines" << endl;
  resultfile.close();
  
  
  ///file output .ddsg
  i=0;
  fn.replace ((fn.size()-5), 5, "ddsg"); //change file extention from .cedge to .ddsg
  resultfile.open(fn.c_str(), ios::out);
  
  resultfile << "d " << numNodes << " " << numEdges << endl; 

  BOOST_FOREACH(intIntPairIntMap::value_type it, entry){
    resultfile <<  it.second.first.first << " " << it.second.first.second << " " << it.second.second << " 0" << endl;
    i++;
  }
  cout << fn << " written.\n" << i << " lines" << endl;
  resultfile.close();
}


  
