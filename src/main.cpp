/*
 *  Alpha emulation library
 *  Copyright (C) 2014 Franklin Wei
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <cctype>
#include <cstdio>
#include <csignal>
#include <cstring>

#include <alpha.h>

using namespace std;
char hex_chars[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
alpha_ctx* ctx;
bool disasm=false, debugger=false, quiet=false;
istream *in=&cin;
bool interactive=true;
bool ascii=true;
bool compile=false;
string compile_output="a.out";
void do_help(char* name)
{
  cerr << "Usage: " << name << " [options] <file>" << endl;
  cerr << "Execute the Alpha machine code in FILE or read from standard input." << endl;
  cerr << "  -b, --binary\t\t\tTreat the code as raw bytes instead of hex" << endl;
  cerr << "  -c, --compile\t\t\tStore the hex input as raw binary in a.out" << endl;
  cerr << "  -d, --debug\t\t\tEnable debugging mode" << endl;
  cerr << "  -D, --disasm\t\t\tRun disassembler and exit" << endl;
  cerr << "  -h, --help\t\t\tPrint this help and exit" << endl;
  cerr << "  -o <file>\t\t\tOutput compiled code to FILE instead of a.out" << endl;
  cerr << "  -q, --quiet\t\t\tDon't print help text in interactive mode" << endl;
  cerr << "  -v, --version\t\t\tDisplay version information" << endl;
}
void parse_args(int argc, char* argv[])
{
  for(int i=1;i<argc;++i)
    {
      string arg=argv[i];
      if(arg=="-b" or arg=="--binary")
	{
	  ascii=false;
	}
	else if(arg=="-c" or arg=="--compile")
	{
	  compile=true;
	}
      else if(arg=="-D" or arg=="--disasm") // disassembler
	{
	  if(!debugger or compile)
	    {
	      disasm=true;
	    }
	  else
	    {
	      cerr << "Cannot use debugger with disassembler." << endl;
	      exit(1);
	    }
	}
      else if(arg=="--debug" or arg=="-d")
	{
	  if(!disasm)
	    debugger=true;
	  else
	    {
	      cerr << "Cannot use debugger with disassembler." << endl;
	      exit(1);
	    } 
	}
      else if(arg=="--help" or arg=="-h")
	{
	  do_help(argv[0]);
	  exit(1);
	}
      else if(arg=="--quiet" or arg=="-q")
	{
	  quiet=true;
	}
      else if(arg=="-o")
	{
	  if(i<argc-1)
	    {
	      compile_output=argv[i+1];
	      ++i;
	    }
	}
      else if(arg=="--version" or arg=="-v")
	{
	  cout << "Alpha Emulation Library " << ALPHA_VERSION << " implementing instruction revision " << ALPHA_INSTRUCTION_SET_VERSION << endl;
	  cout << "Built " << __DATE__ << "." << endl;
	  cout << "Copyright (C) 2014 Franklin Wei" << endl;
	  cout << "This program comes with ABSOLUTELY NO WARRANTY; for details see `LICENSE'." << endl;
	  cout << "This is free software, and you are welcome to redistribute it" << endl;
	  cout << "under certain conditions; see `LICENSE' for details." << endl;
	  exit(1);
	}
      else if(arg.length()>0 and arg[0]!='-')
	{
	  if(interactive)
	    {
	      static ifstream ifs(arg.c_str());
	      if(ifs.good())
		{
		  in=&ifs;
		  if(!ascii)
		    {
		      in=new ifstream(arg.c_str(), ios::binary);
		    }
		  interactive=false;
		}
	      else
		{
		  cerr << "Error opening file." << endl;
		  exit(4);
		}
	    }
	  else
	    {
	      cerr << "Only one file can be specified." << endl;
	      exit(2);
	    }
	}
      else
	{
	  cerr << "Unknown option `" << arg << "'" << endl;
	  exit(1);
	}
    }
  if(interactive and !ascii)
    {
      cerr << "Cannot read binary input from terminal." << endl;
    }
  if(compile and debugger)
    {
      cerr << "Cannot run compiler and debugger." << endl;
      exit(1);
    }
  if(compile and disasm)
    {
      cerr << "Cannot run compiler and disassembler." << endl;
      exit(1);
    }
}
void run()
{
  if(!disasm)
    {
      if(!debugger)
	{
	  while(ctx->running)
	    alpha_exec(ctx);
	}
      else
	{
	  while(ctx->running)
	    {
	      alpha_exec(ctx);
	      cout << "Press enter to step..." << endl;
	      cin.get();
	    }
	}
    }
  else
    {
      while(ctx->running)
	{
	  alpha_disasm(ctx);
	}
    }
}
void ctrlc(int signum)
{
  cout << "1. Continue" << endl;
  cout << "2. Abort" << endl;
  signal(SIGINT, SIG_DFL);
  while(true)
    {
      string str;
      cout << "? " << flush;
      getline(cin, str);
      if(str.length()==0)
	exit(1);
      if(str[0]=='2' or str=="exit" or str=="quit")
	exit(1);
      if(str[0]=='1')
	{
	  signal(SIGINT, &ctrlc);
	  run();
	}
    }
  exit(1);
}
void compile_to_binary(vector<byte> prog)
{
  ofstream out(compile_output.c_str(), ios::binary);
  out<<"FW";
  for(word i=0;i<prog.size();++i)
    {
      out<<prog[i];
    }
}
int main(int argc, char* argv[])
{
  parse_args(argc, argv);
  vector<byte> prog;
  while(in->good())
    {
      if(ascii)
	{
	  if(interactive)
	    printf("0x%08X:", (unsigned int)prog.size()); // really shouldn't mix C-style and stream I/O!
	  string line;
	  getline(*in, line);
	  for(unsigned int i=0;i<line.length();i+=2)
	    {
	      byte val=0xFF;
	      bool good=false;
	      for(int j=0;j<16;++j)
		{
		  if(toupper(line[i+1])==hex_chars[j])
		    {
		      val=j;
		      good=true;
		    }
		}
	      if(!good)
		{
		  cerr << "Bad hex input." << endl;
		  return 1;
		}
	      good=false;
	      for(int j=0;j<16;++j)
		{
		  if(toupper(line[i])==hex_chars[j])
		    {
		      val|=((j&0xF)<<4);
		      good=true;
		    }
		}
	      if(!good)
		{
		  cerr << "Bad hex input." << endl;
		  return 1;
		}
	      prog.push_back(val);
	    }
	}
      else
	{
	  in->seekg(0);
	  char magic[2];
	  magic[0]=in->get();
	  magic[1]=in->get();
	  if(magic[0]=='F' and magic[1]=='W')
	    {
	      while(in->good())
		prog.push_back(in->get());
	    }
	  else
	    {
	      cerr << "Bad magic number." << endl;
	      return 1;
	    }
	}
    }
  cin.clear();
  byte* p=(byte*)malloc(prog.size());
  for(unsigned int i=0;i<prog.size();++i)
    p[i]=prog[i];
  if(!disasm)
    signal(SIGINT, &ctrlc);
  ctx=alpha_init((byte*)p, // memory
		 prog.size());
  if(interactive && !disasm && !compile and !quiet)
    cout << endl << "Beginning execution..." << endl;
  if(compile)
    {
      compile_to_binary(prog);
      return 0;
    }
  run();
  if(ctx->error_code!=0)
    {
      switch(ctx->error_code)
        {
        case ALPHA_OUT_OF_BOUNDS:
          cerr << "Bad memory access." << endl;
          break;
        case ALPHA_DIVIDE_BY_ZERO:
          cerr << "Attempted division by zero." << endl;
          break;
        default:
          cerr << "Unknown error." << endl;
          break;
        }
    }
  return ctx->return_value;
}
