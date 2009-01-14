/* Minion Constraint Solver
   http://minion.sourceforge.net
   
   For Licence Information see file LICENSE.txt 

   $Id$
*/

#include "minion.h"
#include "CSPSpec.h"


#include "BuildConstraint.h"

#include "inputfile_parse.h"

#include "inputfile_parse/MinionInputReader.hpp"
#include "inputfile_parse/MinionThreeInputReader.hpp"

#include "counter.hpp"

#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>

using boost::iostreams::filtering_istream;
using boost::iostreams::gzip_decompressor;
using boost::iostreams::bzip2_decompressor;

using boost::iostreams::error_counter;

template<typename Reader, typename Stream>
    void ReadCSP(Reader& reader, ConcreteFileReader<Stream>* infile)
{
    reader.read(infile) ;
    oldtableout.set(string("Filename"), infile->filename);  
}

template<typename InputReader>
CSPInstance readInput(InputReader* infile, bool parser_verbose)
{  
    string test_name = infile->get_string();
    if(test_name != "MINION")
      INPUT_ERROR("All Minion input files must begin 'MINION'");
  
    int inputFileVersionNumber = infile->read_num();
  
    if(inputFileVersionNumber > 3)
      INPUT_ERROR("This version of Minion only supports formats up to 3");
  

    // C++0x comment : Need MOVE (which is std::move) here to activate r-value references.
    // Normally we wouldn't, but here the compiler can't figure out it can "steal" instance.
    if(inputFileVersionNumber == 3)
    {
      MinionThreeInputReader<InputReader> reader(parser_verbose);
      ReadCSP(reader, infile);
      return MOVE(reader.instance);
    } 
    else
    {
      MinionInputReader<InputReader> reader(parser_verbose);
      ReadCSP(reader, infile);
      return MOVE(reader.instance);
    }  
}


CSPInstance readInputFromFile(string fname, bool parser_verbose)
{
    const char* filename = fname.c_str();
    string extension;
    if(fname.find_last_of(".") < fname.size())
      extension = fname.substr(fname.find_last_of("."), fname.size());
  
    filtering_istream in;
    
    error_counter e_count;
    
    in.push(boost::ref(e_count));
    
    if(extension == ".gz" || extension == ".gzip" || extension == ".z" || extension == ".gzp" ||
        extension == ".bz2" || extension == ".bz" || extension == ".bzip2" || extension == ".bzip")
    {  
      if(extension == ".gz" || extension == ".gzip" || extension == ".z" || extension == ".gzp")
      {
        if(parser_verbose)
          cout << "# Using gzip uncompression" << endl;
        in.push(gzip_decompressor());
      }    
  
      if(extension == ".bz2" || extension == ".bz" || extension == ".bzip2" || extension == ".bzip")
      {
        if(parser_verbose)
          cout << "# Using bzip2 uncompression" << endl;
        in.push(bzip2_decompressor());
      }
      
    }
    
    ifstream* file;
    if(fname != "--")
    {
      file = new ifstream(filename, ios_base::in | ios_base::binary);
      if (!(*file)) {
        INPUT_ERROR("Can't open given input file '" + fname + "'.");
      }
      in.push(*file);
    }
    else
      in.push(cin);

    ConcreteFileReader<filtering_istream> infile(in, filename);

    if (infile.failed_open() || infile.eof()) {
      INPUT_ERROR("Can't open given input file '" + fname + "'.");
    }   
    
    try
    {
      return readInput(&infile, parser_verbose);
      // delete file;
    }
    catch(parse_exception s)
    {
      cerr << "Error in input!" << endl;
      cerr << s.what() << endl;
      
      cerr << "Error occurred on line " << e_count.lines_prev << ", around character " << e_count.chars_prev << endl;
      cerr << "The parser gave up around: '" << e_count.current_line_prev << "'" << endl;
      exit(1);
    }
    
}
