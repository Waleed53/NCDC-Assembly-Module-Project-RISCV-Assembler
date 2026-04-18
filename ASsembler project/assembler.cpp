
#include <bits/stdc++.h>
using namespace std;

static unordered_map<string,int> regMap = {
  {"zero",0},{"ra",1},{"sp",2},{"gp",3},{"tp",4},
  {"t0",5},{"t1",6},{"t2",7},{"s0",8},{"fp",8},{"s1",9},
  {"a0",10},{"a1",11},{"a2",12},{"a3",13},{"a4",14},{"a5",15},
  {"a6",16},{"a7",17},{"s2",18},{"s3",19},{"s4",20},{"s5",21},
  {"s6",22},{"s7",23},{"s8",24},{"s9",25},{"s10",26},{"s11",27},
  {"t3",28},{"t4",29},{"t5",30},{"t6",31}
};

string trim(const string &s){
  size_t a = s.find_first_not_of(" \t\r\n");
  if(a==string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a,b-a+1);
}

vector<string> splitArgs(const string &rest){
  vector<string> out;
  string cur; int par=0;
  for(char c: rest){
    if(c=='(') par++;
    if(c==')') par--;
    if(c==',' && par==0){
      string t = trim(cur);
      if(!t.empty()) out.push_back(t);
      cur.clear();
    } else cur.push_back(c);
  }
  string t=trim(cur);
  if(!t.empty()) out.push_back(t);
  return out;
}

int parseImm(const string &s){
  string t = trim(s);
  if(t.empty()) throw runtime_error("empty immediate");
  if(t.size()>2 && t[0]=='0' && (t[1]=='x' || t[1]=='X')) return (int)stol(t,nullptr,16);
  return stoi(t);
}

int getReg(const string &s){
  string t = trim(s);
  if(t.size()>0 && t[0]=='x'){
    int r = stoi(t.substr(1));
    if(r>=0 && r<=31) return r;
  }
  if(regMap.count(t)) return regMap[t];
  throw runtime_error("bad register: "+s);
}


uint32_t encR(int funct7,int rs2,int rs1,int funct3,int rd,int opcode){
  return ((uint32_t)funct7<<25)|((uint32_t)rs2<<20)|((uint32_t)rs1<<15)|((uint32_t)funct3<<12)|((uint32_t)rd<<7)|((uint32_t)opcode);
}
uint32_t encI(int imm,int rs1,int funct3,int rd,int opcode){
  uint32_t i = (uint32_t)imm & 0xFFF;
  return (i<<20)|((uint32_t)rs1<<15)|((uint32_t)funct3<<12)|((uint32_t)rd<<7)|((uint32_t)opcode);
}
uint32_t encS(int imm,int rs2,int rs1,int funct3,int opcode){
  uint32_t i = (uint32_t)imm & 0xFFF;
  uint32_t imm11_5 = (i>>5)&0x7F;
  uint32_t imm4_0  = i & 0x1F;
  return (imm11_5<<25)|((uint32_t)rs2<<20)|((uint32_t)rs1<<15)|((uint32_t)funct3<<12)|(imm4_0<<7)|((uint32_t)opcode);
}
uint32_t encB(int imm,int rs2,int rs1,int funct3,int opcode){
  int i = imm;
  i &= 0x1FFF; 
  uint32_t b12 = (i>>12)&1;
  uint32_t b10_5 = (i>>5)&0x3F;
  uint32_t b4_1 = (i>>1)&0xF;
  uint32_t b11 = (i>>11)&1;
  return (b12<<31)|(b10_5<<25)|((uint32_t)rs2<<20)|((uint32_t)rs1<<15)|((uint32_t)funct3<<12)|(b4_1<<8)|(b11<<7)|((uint32_t)opcode);
}
uint32_t encU(int imm,int rd,int opcode){
  return (((uint32_t)imm & 0xFFFFF)<<12)|((uint32_t)rd<<7)|((uint32_t)opcode);
}
uint32_t encJ(int imm,int rd,int opcode){
  int i = imm;
  i &= 0x1FFFFF; // 21 bits
  uint32_t j20 = (i>>20)&1;
  uint32_t j10_1 = (i>>1)&0x3FF;
  uint32_t j11 = (i>>11)&1;
  uint32_t j19_12 = (i>>12)&0xFF;
  return (j20<<31)|(j10_1<<21)|(j11<<20)|(j19_12<<12)|((uint32_t)rd<<7)|((uint32_t)opcode);
}

string toBin(uint32_t v){
  string s(32,'0');
  for(int i=31;i>=0;--i) s[31-i] = ((v>>i)&1)?'1':'0';
  return s;
}
string toHex(uint32_t v){
  stringstream ss; ss<<"0x"<<hex<<setw(8)<<setfill('0')<<v; return ss.str();
}


struct InstLine { string op; vector<string> args; int addr; string raw; };

int main(int argc,char** argv){
  if(argc!=4){ cerr<<"Usage: ./asm_rv32i input.s output.txt -h|-b\n"; return 1; }
  string inF = argv[1], outF = argv[2], mode = argv[3];
  if(mode!="-h" && mode!="-b"){ cerr<<"mode must be -h or -b\n"; return 1; }

  ifstream fin(inF);
  if(!fin){ cerr<<"cannot open "<<inF<<"\n"; return 1; }
  vector<string> lines;
  string ln;
  while(getline(fin,ln)) lines.push_back(ln);
  fin.close();

  unordered_map<string,int> labelAddr;
  vector<InstLine> insts;
  struct DataEntry{ string label; vector<int> words; int addr; };
  vector<DataEntry> dataEntries;

  bool inText=false, inData=false;
  int pcText=0;

  // pass 1
  for(size_t i=0;i<lines.size();++i){
    string s = lines[i];
    size_t pound = s.find('#');
    if(pound!=string::npos) s = s.substr(0,pound);
    s = trim(s);
    if(s.empty()) continue;

    if(s.rfind(".text",0)==0){ inText=true; inData=false; continue; }
    if(s.rfind(".data",0)==0){ inText=false; inData=true; continue; }


    if(s.back()==':'){
      string lab = trim(s.substr(0,s.size()-1));
      if(inText) labelAddr[lab]=pcText;
      else {
        DataEntry d; d.label=lab; d.words.clear(); d.addr=-1; dataEntries.push_back(d);
      }
      continue;
    }


    size_t colon = s.find(':');
    if(colon!=string::npos){
      string lab = trim(s.substr(0,colon));
      string rest = trim(s.substr(colon+1));
      if(inText) labelAddr[lab] = pcText;
      else {
        DataEntry d; d.label=lab; d.words.clear(); d.addr=-1; dataEntries.push_back(d);
      }
      if(rest.empty()) continue;
      s = rest;
    }

    if(inData){

      if(s.rfind(".word",0)==0){
        string rest = trim(s.substr(5));
        vector<string> toks;
        string cur;
        for(char c: rest){
          if(c==','){ toks.push_back(trim(cur)); cur.clear(); }
          else cur.push_back(c);
        }
        if(!cur.empty()) toks.push_back(trim(cur));
        vector<int> words;
        for(auto &t: toks) words.push_back(parseImm(t));
        if(dataEntries.empty()){
          DataEntry d; d.label="__d"+to_string(dataEntries.size()); d.words = words; d.addr=-1; dataEntries.push_back(d);
        } else {
          dataEntries.back().words.insert(dataEntries.back().words.end(), words.begin(), words.end());
        }
        continue;
      } else continue; 
    }


    string op; string rest;
    {
      stringstream ss(s);
      ss>>op;
      getline(ss,rest);
      rest = trim(rest);
    }
    InstLine il; il.op = op; il.raw = s; il.addr = pcText;
    if(rest.empty()) il.args = {};
    else il.args = splitArgs(rest);
    insts.push_back(il);
    pcText += 4;
  }


  int dataBase = ((pcText+3)/4)*4;
  int curDataAddr = dataBase;
  for(auto &d : dataEntries){
    d.addr = curDataAddr;
    labelAddr[d.label] = d.addr;
    curDataAddr += (int)d.words.size()*4;
  }

  // pass 2
  ofstream fout(outF);
  if(!fout){ cerr<<"cannot open "<<outF<<"\n"; return 1; }

  for(auto &il : insts){
    try{
      uint32_t enc = 0;
      string op = il.op;
      auto &args = il.args;

      // add, sub, sll, slt, sltu, xor, srl, sra, or, and
      if(op=="add"||op=="sub"||op=="sll"||op=="slt"||op=="sltu"||op=="xor"||op=="srl"||op=="sra"||op=="or"||op=="and"){
        if(args.size()!=3) throw runtime_error("R-type needs rd,rs1,rs2");
        int rd=getReg(args[0]), rs1=getReg(args[1]), rs2=getReg(args[2]);
        int funct3=0, funct7=0;
        if(op=="add") {funct3=0x0; funct7=0x00;}
        else if(op=="sub"){funct3=0x0; funct7=0x20;}
        else if(op=="sll"){funct3=0x1; funct7=0x00;}
        else if(op=="slt"){funct3=0x2; funct7=0x00;}
        else if(op=="sltu"){funct3=0x3; funct7=0x00;}
        else if(op=="xor"){funct3=0x4; funct7=0x00;}
        else if(op=="srl"){funct3=0x5; funct7=0x00;}
        else if(op=="sra"){funct3=0x5; funct7=0x20;}
        else if(op=="or"){funct3=0x6; funct7=0x00;}
        else if(op=="and"){funct3=0x7; funct7=0x00;}
        enc = encR(funct7, rs2, rs1, funct3, rd, 0x33);
      }
      //addi, slti, sltiu, xori, ori, andi
      else if(op=="addi"||op=="slti"||op=="sltiu"||op=="xori"||op=="ori"||op=="andi"){
        if(args.size()!=3) throw runtime_error("I-type needs rd,rs1,imm");
        int rd=getReg(args[0]), rs1=getReg(args[1]), imm=parseImm(args[2]);
        int funct3=0;
        if(op=="addi") funct3=0x0;
        else if(op=="slti") funct3=0x2;
        else if(op=="sltiu") funct3=0x3;
        else if(op=="xori") funct3=0x4;
        else if(op=="ori") funct3=0x6;
        else if(op=="andi") funct3=0x7;
        enc = encI(imm, rs1, funct3, rd, 0x13);
      }
      // slli, srli, srai  (I-type with funct7)
      else if(op=="slli"||op=="srli"||op=="srai"){
        if(args.size()!=3) throw runtime_error("shift.i needs rd,rs1,shamt");
        int rd=getReg(args[0]), rs1=getReg(args[1]), shamt=parseImm(args[2]);
        if(shamt < 0 || shamt > 31) throw runtime_error("shamt out of range");
        int funct3=0x1, funct7=0x00;
        if(op=="slli") {funct3=0x1; funct7=0x00;}
        else if(op=="srli"){funct3=0x5; funct7=0x00;}
        else if(op=="srai"){funct3=0x5; funct7=0x20;}
        uint32_t imm = ((uint32_t)funct7<<5) | (shamt & 0x1F);
        enc = (imm<<20)|(rs1<<15)|(funct3<<12)|(rd<<7)|0x13;
      }
      // lb, lh, lw, lbu, lhu
      else if(op=="lb"||op=="lh"||op=="lw"||op=="lbu"||op=="lhu"){
        if(args.size()!=2) throw runtime_error("load needs rd, off(rs1)");
        int rd=getReg(args[0]);
        string mem = args[1];
        size_t l = mem.find('('), r = mem.find(')');
        if(l==string::npos || r==string::npos) throw runtime_error("bad mem syntax");
        int off = parseImm(mem.substr(0,l));
        int rs1 = getReg(mem.substr(l+1, r-l-1));
        int funct3 = (op=="lb")?0x0: (op=="lh")?0x1: (op=="lw")?0x2: (op=="lbu")?0x4:0x5;
        enc = encI(off, rs1, funct3, rd, 0x03);
      }
      // sb, sh, sw 
      else if(op=="sb"||op=="sh"||op=="sw"){
        if(args.size()!=2) throw runtime_error("store needs rs2, off(rs1)");
        int rs2=getReg(args[0]);
        string mem = args[1];
        size_t l = mem.find('('), r = mem.find(')');
        if(l==string::npos || r==string::npos) throw runtime_error("bad mem syntax");
        int off = parseImm(mem.substr(0,l));
        int rs1 = getReg(mem.substr(l+1, r-l-1));
        int funct3 = (op=="sb")?0x0: (op=="sh")?0x1:0x2;
        enc = encS(off, rs2, rs1, funct3, 0x23);
      }
      // beq, bne, blt, bge, bltu, bgeu
      else if(op=="beq"||op=="bne"||op=="blt"||op=="bge"||op=="bltu"||op=="bgeu"){
        if(args.size()!=3) throw runtime_error("branch needs rs1,rs2,label/imm");
        int rs1=getReg(args[0]), rs2=getReg(args[1]);
        int imm=0;
        string target = args[2];
        if(labelAddr.count(target)) imm = labelAddr[target] - il.addr;
        else imm = parseImm(target);
        int funct3 = (op=="beq")?0x0: (op=="bne")?0x1: (op=="blt")?0x4: (op=="bge")?0x5: (op=="bltu")?0x6:0x7;
        enc = encB(imm, rs2, rs1, funct3, 0x63);
      }
      // jal 
      else if(op=="jal"){
        if(args.size()!=2) throw runtime_error("jal needs rd,label/imm");
        int rd = getReg(args[0]);
        int imm=0;
        string target = args[1];
        if(labelAddr.count(target)) imm = labelAddr[target] - il.addr;
        else imm = parseImm(target);
        enc = encJ(imm, rd, 0x6F);
      }
      // jalr
      else if(op=="jalr"){
        if(args.size()!=3 && args.size()!=2) throw runtime_error("jalr needs rd, rs1, imm  or jalr rd, rs1");
        // Accept forms: jalr rd, rs1, imm   or jalr rd, rs1
        int rd = getReg(args[0]);
        int rs1 = getReg(args[1]);
        int imm = 0;
        if(args.size()==3) imm = parseImm(args[2]);
        enc = encI(imm, rs1, 0x0, rd, 0x67);
      }
      // lui, auipc
      else if(op=="lui"||op=="auipc"){
        if(args.size()!=2) throw runtime_error("lui/auipc needs rd, imm20");
        int rd = getReg(args[0]);
        int imm = parseImm(args[1]);
        enc = encU(imm, rd, (op=="lui")?0x37:0x17);
      }
      else {
        throw runtime_error("unsupported op: "+op);
      }

      if(mode=="-h") fout<<toHex(enc)<<"\n";
      else fout<<toBin(enc)<<"\n";
    }catch(const exception &e){
      cerr<<"Error at address 0x"<<hex<<il.addr<<dec<<": "<<e.what()<<"\n";
    }
  }

  cout<<"Assembled "<<insts.size()<<" instructions. Text bytes="<<pcText<<", data bytes="<<(curDataAddr-dataBase)<<"\n";
  fout.close();
  return 0;
}
