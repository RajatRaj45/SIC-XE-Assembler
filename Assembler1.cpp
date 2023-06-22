#include<bits/stdc++.h>
using namespace std;
map<string, pair<int, string>> opCode;

vector<string> assmDir = {"START", "END", "BYTE", "WORD", "RESB", "RESW", "BASE", "EQU", "ORG", "LTORG"};

vector<string> registers = {"A","X","L","B","S","T","F","__","PC","SW"};
vector<vector<string>> instructions;
vector<int> locctr;
vector<int> modification;
vector<int> modificationSize;
vector<string> modificationName;
vector<string> objCode;
vector<string> objProgram;
vector<vector<string>> Finalobj;
string programName;
string programLength;
int startAddr=0;
int org_address = -1;
set<string> globalD;
set<string> globalR;
vector<string> extDef;
vector<string> extRef;

bool isSubset(set<string> sub, set<string> super){
    for(auto x:sub){
        if(!super.count(x)) return 0;
    }
    return 1;
}

class Symbol{
  public:
    string name;
    string parent;
    int addr;
    bool isSym;
    // bool isEqu;
    bool relative;

    Symbol(string x){
        name=x;
        parent = "";
        isSym = 0;
        relative = 1;
        addr = -1;
    }

    Symbol(string x, int address){
        name = x;
        isSym = 0;
        parent = "";
        relative = 1;
        addr = address;
    }
};

class Literal{
  public:
    string name;
    int addr;
    int size;
    bool written;
    
    Literal(string x){
        name = x;
        addr = -1;
        written = 0;
        if(x[0]=='C'){
            size = x.size()-3;
        }
        if(x[0]=='X'){
            size = (x.size()-3)/2;
        }
    }
};

vector<int> CSind;
vector<Symbol> symTab;
vector<Literal> litTab;

int findinvec(vector<string> vec, string s){
    for(int i=0;i<vec.size();i++){
        if(vec[i]==s){
            return i;
        }
    }
    return -1;
}

int findincharvec(vector<char> vec, char s){
    for(int i=0;i<vec.size();i++){
        if(vec[i]==s){
            return i;
        }
    }
    return -1;
}

int hextoint(string s){
    int result=0;
    map<char,int> map_w;
    map_w.insert({'0',0});
    map_w.insert({'1',1});
    map_w.insert({'2',2});
    map_w.insert({'3',3});
    map_w.insert({'4',4});
    map_w.insert({'5',5});
    map_w.insert({'6',6});
    map_w.insert({'7',7});
    map_w.insert({'8',8});
    map_w.insert({'9',9});
    map_w.insert({'A',10});
    map_w.insert({'B',11});
    map_w.insert({'C',12});
    map_w.insert({'D',13});
    map_w.insert({'E',14});
    map_w.insert({'F',15});
    for(int i=s.size()-1;i>=0;i--){
        result = result + pow(16,s.size()-i-1)*map_w[s[i]];
    }
    return result;
}

string inttohex(int x){
    string s = "";
    bool neg;
    int z;
    map<int,char> map_w;
    map_w.insert({0,'0'});
    map_w.insert({1,'1'});
    map_w.insert({2,'2'});
    map_w.insert({3,'3'});
    map_w.insert({4,'4'});
    map_w.insert({5,'5'});
    map_w.insert({6,'6'});
    map_w.insert({7,'7'});
    map_w.insert({8,'8'});
    map_w.insert({9,'9'});
    map_w.insert({10,'A'});
    map_w.insert({11,'B'});
    map_w.insert({12,'C'});
    map_w.insert({13,'D'});
    map_w.insert({14,'E'});
    map_w.insert({15,'F'});
    while(x>0){
        z = x%16;
        s = map_w[z] + s;
        x = x/16;
    }
    if(s==""){
        s = "0";
    }
    return s;
}

string inttohex_param(int x,int n){
    if(x>=0){
        string s = inttohex(x);
        while(s.size()<n){
            s = '0' + s;
        }
        return s;
    }
    else{
        int q = pow(16,n) - 1;
        q = q - abs(x) + 1;
        string s = inttohex(q);
        while(s.size()<n){
            s = 'F' + s;
        }
        return s;
    }
}

string hexsub(string s1, string s2){
    return inttohex(hextoint(s1) - hextoint(s2));
}

string hexadd(string s1, string s2){
    return inttohex(hextoint(s1) + hextoint(s2));
}

bool isNum(string s){
    vector<char> vec = {'0','1','2','3','4','5','6','7','8','9'};
    bool flag=0;
    for(int i=0;i<s.size();i++){
        flag=0;
        for(int j=0;j<vec.size();j++){
            if(s[i]==vec[j]){
                flag=1;
                break;
            }
        }
        if(!flag){
            return flag;
        }
    }
    return flag;
}

int findSymbol(string s){
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].name==s) return i;
    }
    return -1;
}

int findLiteral(string s){
    for(int i=0;i<litTab.size();i++){
        if(litTab[i].name==s){
            return i;
        }
    }
    return -1;
}

void opCodeMapping(){
    opCode.insert({"ADD", make_pair(4,"18")});
    opCode.insert({"ADDF", make_pair(4,"58")});
    opCode.insert({"ADDR", make_pair(2,"90")});
    opCode.insert({"AND", make_pair(4,"40")});
    opCode.insert({"CLEAR", make_pair(2,"B4")});
    opCode.insert({"COMP", make_pair(4,"28")});
    opCode.insert({"COMPF", make_pair(4,"88")});
    opCode.insert({"COMPR", make_pair(2,"A0")});
    opCode.insert({"DIV", make_pair(4,"24")});
    opCode.insert({"DIVF", make_pair(4,"64")});
    opCode.insert({"DIVR", make_pair(2,"9C")});
    opCode.insert({"FIX", make_pair(1,"C4")});
    opCode.insert({"FLOAT", make_pair(1,"C0")});
    opCode.insert({"HIO", make_pair(1,"F4")});
    opCode.insert({"J", make_pair(4,"3C")});
    opCode.insert({"JEQ", make_pair(4,"30")});
    opCode.insert({"JGT", make_pair(4,"34")});
    opCode.insert({"JLT", make_pair(4,"38")});
    opCode.insert({"JSUB", make_pair(4,"48")});
    opCode.insert({"LDA", make_pair(4,"00")});
    opCode.insert({"LDB", make_pair(4,"68")});
    opCode.insert({"LDCH", make_pair(4,"50")});
    opCode.insert({"LDF", make_pair(4,"70")});
    opCode.insert({"LDL", make_pair(4,"08")});
    opCode.insert({"LDS", make_pair(4,"6C")});
    opCode.insert({"LDT", make_pair(4,"74")});
    opCode.insert({"LDX", make_pair(4,"04")});
    opCode.insert({"LPS", make_pair(4,"D0")});
    opCode.insert({"MUL", make_pair(4,"20")});
    opCode.insert({"MULF", make_pair(4,"60")});
    opCode.insert({"MULR", make_pair(4,"98")});
    opCode.insert({"NORM", make_pair(1,"C8")});
    opCode.insert({"OR", make_pair(4,"44")});
    opCode.insert({"RD", make_pair(4,"D8")});
    opCode.insert({"RMO", make_pair(2,"AC")});
    opCode.insert({"RSUB", make_pair(4,"4C")});
    opCode.insert({"SHIFTL", make_pair(2,"A4")});
    opCode.insert({"SHIFTR", make_pair(2,"A8")});
    opCode.insert({"SIO", make_pair(1,"F0")});
    opCode.insert({"SSK", make_pair(4,"EC")});
    opCode.insert({"STA", make_pair(4,"0C")});
    opCode.insert({"STB", make_pair(4,"78")});
    opCode.insert({"STCH", make_pair(4,"54")});
    opCode.insert({"STF", make_pair(4,"80")});
    opCode.insert({"STI", make_pair(4,"D4")});
    opCode.insert({"STL", make_pair(4,"14")});
    opCode.insert({"STS", make_pair(4,"7C")});
    opCode.insert({"STSW", make_pair(4,"E8")});
    opCode.insert({"STT", make_pair(4,"84")});
    opCode.insert({"STX", make_pair(4,"10")});
    opCode.insert({"SUB", make_pair(4,"1C")});
    opCode.insert({"SUBF", make_pair(4,"5C")});
    opCode.insert({"SUBR", make_pair(2,"94")});
    opCode.insert({"SVC", make_pair(2,"B0")});
    opCode.insert({"TD", make_pair(4,"E0")});
    opCode.insert({"TIO", make_pair(1,"F8")});
    opCode.insert({"TIX", make_pair(4,"2C")});
    opCode.insert({"TIXR", make_pair(2,"B8")});
    opCode.insert({"WD", make_pair(4,"DC")});    
}

bool containsArithmetic(string s){
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            return 1;
        }
    }
    return 0;
}

int isExpression(string s){
    if(s[0]=='-' or s[0]=='+'){
        s="0"+s;
    }
    vector<string> elements;
    vector<char> operators;
    string s1,s2,res;
    vector<int> values;
    int result = 0;
    int counter;
    int relative_count=0;
    string temp="";
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            elements.push_back(temp);
            operators.push_back(s[i]);
            temp = "";
        }
        else{
            temp = temp + s[i];
        }
    }
    if(temp.size()!=0){
        elements.push_back(temp);
        temp = "";
    }
    if(findSymbol(elements[0])!=-1 and symTab[findSymbol(elements[0])].relative==1){
        relative_count = 1;
    }
    for(int i=0;i<elements.size();i++){
        if(isNum(elements[i]) or findSymbol(elements[i])!=-1){
            if(i>0){
                if(findSymbol(elements[i])!=-1 and symTab[findSymbol(elements[i])].relative==1){
                    if(operators[i]=='*' or operators[i]=='/' or operators[i-1]=='*' or operators[i-1]=='/'){
                        return INT_MAX;
                    }
                    else{
                        if(operators[i-1]=='+'){
                            relative_count++;
                        }
                        else{
                            relative_count--;
                        }
                    }
                }
            }    
        }
        else{
            return INT_MAX;
        }
    }
    if(relative_count<0 or relative_count>1){
        return INT_MAX;
    }
    int index = -1;
    while(findincharvec(operators,'*')!=-1 or findincharvec(operators,'/')!=-1){
        for(int i=0;i<operators.size();i++){
            if(operators[i]=='*' or operators[i]=='/'){
                index = i;
                break;
            }
        }
        if(operators[index]=='*'){
            elements[index] = to_string(stoi(elements[index])*stoi(elements[index+1]));
        }
        else{
            elements[index] = to_string(stoi(elements[index])/stoi(elements[index+1]));
        }
        elements.erase(elements.begin()+index+1);
        operators.erase(operators.begin()+index);
    }
    if(findSymbol(elements[0])==-1){
        result = stoi(elements[0]);
    }
    else{
        result = symTab[findSymbol(elements[0])].addr;
    }
    for(int i=0;i<operators.size();i++){
        // if(i!=0){
            if(operators[i]=='+'){
                if(findSymbol(elements[i+1])==-1){
                    result += stoi(elements[i+1]);
                }
                else{
                    result += symTab[findSymbol(elements[i+1])].addr;
                }
            }
            else{
                if(findSymbol(elements[i+1])==-1){
                    result -= stoi(elements[i+1]);
                }
                else{
                    result -= symTab[findSymbol(elements[i+1])].addr;
                }
            }
    }
    return result;
}

int detectRExp(string s){
    vector<string> elements;
    vector<char> operators;
    string temp="";
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            elements.push_back(temp);
            operators.push_back(s[i]);
            temp = "";
        }
        else{
            temp = temp + s[i];
        }
    }
    if(temp.size()!=0){
        elements.push_back(temp);
        temp = "";
    }
    for(int i=0;i<operators.size();i++){
        if(operators[i]=='*' or operators[i]=='/'){
            if(!isNum(elements[i]) or !isNum(elements[i+1])){
                return -1;
            }
        }
    }
    for(int i=0;i<elements.size();i++){
        if(findinvec(extRef, elements[i])!=-1){
            return 1;
        }
    }
    return 0;
}

vector<string> breakRExp(string s){
    vector<string> vec;
    vector<string> elements;
    vector<char> operators;
    string temp="";
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            elements.push_back(temp);
            operators.push_back(s[i]);
            temp = "";
        }
        else{
            temp = temp + s[i];
        }
    }
    if(temp.size()!=0){
        elements.push_back(temp);
        temp = "";
    }
    operators.insert(operators.begin(), '+');
    string vec1="";
    vector<string> vec2;
    for(int i=0;i<elements.size();i++){
        if(findinvec(extRef, elements[i])!=-1){
            vec2.push_back(operators[i]+elements[i]);
        }
        else{
            vec1=vec1+operators[i]+elements[i];
        }
    }
    vec2.insert(vec2.begin(), vec1);
    return vec2;
}

bool absOrRelative(string s){
    vector<string> elements;
    vector<char> operators;
    string s1,s2,res;
    vector<int> values;
    int result = 0;
    int counter;
    int relative_count=0;
    string temp="";
    for(int i=0;i<s.size();i++){
        if(s[i]=='+' or s[i]=='-' or s[i]=='*' or s[i]=='/'){
            elements.push_back(temp);
            operators.push_back(s[i]);
            temp = "";
        }
        else{
            temp = temp + s[i];
        }
    }
    if(temp.size()!=0){
        elements.push_back(temp);
        temp = "";
    }
    if(findSymbol(elements[0])!=-1 and symTab[findSymbol(elements[0])].relative==1){
        relative_count = 1;
    }
    for(int i=0;i<elements.size();i++){
        if(isNum(elements[i]) or findSymbol(elements[i])!=-1){
            if(i>0){
                if(findSymbol(elements[i])!=-1 and symTab[findSymbol(elements[i])].relative==1){
                    if(operators[i]=='*' or operators[i]=='/' or operators[i-1]=='*' or operators[i-1]=='/'){
                        return INT_MAX;
                    }
                    else{
                        if(operators[i-1]=='+'){
                            relative_count++;
                        }
                        else{
                            relative_count--;
                        }
                    }
                }
            }    
        }
        else{
            return INT_MAX;
        }
    }
    if(relative_count==0){
        return 0;
    }
    else{
        return 1;
    }
}

void printSymTab(){
    cout<<"\n\n\tSYMBOL TABLE\n----------------------------------------------------------\n";
    for(int i=0;i<symTab.size();i++){
        cout<<symTab[i].name<<" : "<<inttohex(symTab[i].addr)<<"\n";
    }
    cout<<"-----------------------------------------------------------\n";
}

void printLitTab(){
    cout<<"\n\tLITERAL TABLE\n--------------------------------------------------------------------\n";
    for(int i=0;i<litTab.size();i++){
        cout<<litTab[i].name<<" : "<<litTab[i].size<<" , "<<inttohex(litTab[i].addr)<<"\n";
    }
    cout<<"------------------------------------------------------------------\n";
}

vector<string> getElements(string inst){
    vector<string> el;
    string temp="";
    for(int i=0;i<inst.size();i++){
        if(inst[i]==' ' || inst[i]==','){
            el.push_back(temp);
            temp="";
        }
        else temp=temp+inst[i];
    }
    if(temp!=""){
        el.push_back(temp);
    }
    return el;
}

void getInstructions(){
    int ind=0;
    CSind.push_back(ind);
    string s;
    while(1){
        getline(cin, s);
        vector<string> elements=getElements(s);
        instructions.push_back(elements);
        if(elements[0]=="EXTDEF"){
            for(int i=1;i<elements.size();i++){
                globalD.insert(elements[i]);
            }
        }
        else if(elements[0]=="EXTREF"){
            for(int i=1;i<elements.size();i++){
                globalR.insert(elements[i]);
            }
        }
        else if(elements.size()>1 && elements[1]=="CSECT"){
            CSind.push_back(ind);
            globalD.insert(elements[0]);
        }
        else if(elements[0]=="END") break;
        else if(elements.size()>1 && elements[1]=="END") break;
        ind++;
    }
    if(globalR.size()>0 and !isSubset(globalR, globalD)){
        cout<<"ERROR: EXTERNAL REFERENCE NOT FOUND IN EXTERNAL DEFINITIONS!\n";
    }
}

void printListingFile(vector<vector<string>> v){
    cout<<"\n";
    cout<<"\t\t"<<setw(31)<<"LISTING FILE\n\n";
    cout<<"\tADDRESS\t\tINSTRUCTION";
    cout<<setw(39)<<"OBJECT CODE\n";
    for(int i=0;i<v.size();i++){
        int charlength = 0;
        cout<<"\t"<<inttohex(locctr[i])<<"\t";
        cout<<"\t";
        for(int j=0;j<v[i].size();j++){
            cout<<v[i][j];
            charlength += v[i][j].size();
            if(j!=v[i].size()-1){
                cout<<" ";
                charlength++;
            }
        }
        cout<<setw(50-charlength)<<objCode[i]<<"\n";
    }
    cout<<"\n\nLENGTH OF PROGRAM: "<<programLength<<"\n";
    cout<<"--------------------------------------------------------------\n";
}

int firstPass(){
    int returnInt=1;
    programName=instructions[0][0];
    org_address = -1;
    if(programName.size()>6){
        cout<<"error at line 1: NAME OF PROGRAM ENTERED IS MORE THAN 6 COLUMNS!\n";
        returnInt=0;
    }
    if(instructions[0].size()>1){
        if(instructions[0][1]!="START"){
            cout<<"error at line 1: START assembler directive not found"<<endl;
            returnInt=0;
        }
        else{
            if(instructions[0].size()==3) startAddr=hextoint(instructions[0][2]);
            if(instructions[0].size()>3){
                cout<<"error at line 1\n";
                returnInt=0;
            }
        }
    }
    locctr.push_back(startAddr);
    int curr_addr = startAddr;
    for(int iterator=1;iterator<instructions.size()-1;iterator++){
        vector<string> inst=instructions[iterator];
        int format_supported;
        int curr_format = 0;
        int counter_literal=0;
        string opcode;
        bool flag=0;
        bool form4 = 0;
        for(int i=0;i<inst.size();i++){
            if(flag and format_supported==1){
                cout<<"error at line "<<iterator+1<<" : INVALID INSTRUCTION FORMAT!\n";
                returnInt=0;
            }
            form4=0;
            if(flag and inst[i-1]!="BYTE" and inst[i-1]!="EQU"){
                if(findinvec(registers,inst[i])==-1){
                    //NOT A REGISTER
                    if(inst[i][0]=='#' or inst[i][0]=='@'){
                        if(!isNum(inst[i].substr(1))){
                            //#LABEL or @LABEL
                            if(findSymbol(inst[i].substr(1))==-1){
                                Symbol operand(inst[i].substr(1));
                                symTab.push_back(operand);
                            }
                        }
                    }
                    else if(inst[i][0]=='='){
                        if(inst[i][1]=='C' or inst[i][1]=='X'){
                            Literal lit(inst[i].substr(1));
                            if(findLiteral(inst[i].substr(1))==-1){
                                litTab.push_back(lit);
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": Invalid Literal definition!\n";
                            returnInt=0; 
                        }
                    }
                    else{
                        if(!isNum(inst[i])){
                            //LABEL
                            if(findSymbol(inst[i])==-1){
                                Symbol operand(inst[i]);
                                symTab.push_back(operand);
                            }
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="ORG"){
                if(inst.size()!=2){
                    cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT!\n";
                    returnInt=0;
                }
                else{
                    if(isNum(inst[i])==1){
                        curr_format = stoi(inst[i]) - curr_addr;
                        org_address = curr_addr;
                    }
                    else{
                        if(findSymbol(inst[i])!=-1 and symTab[findSymbol(inst[i])].addr != -1){
                            curr_format = symTab[findSymbol(inst[i])].addr - curr_addr;
                            org_address = curr_addr;
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT AS FORWARD REFERENCE IS NOT ALLOWED!\n";
                            returnInt=0;
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="EQU"){
                if((i-2)!=0){
                    cout<<"error at line "<<iterator+1<<": NO SYMBOL NAME!\n";
                    returnInt=0;
                }
                else{
                    if(findSymbol(inst[i-2])!=-1 and symTab[findSymbol(inst[i-2])].addr != curr_addr){
                        cout<<"error at line "<<iterator+1<<": Forward reference in the symbol "<<inst[i-2]<<"!\n";
                        returnInt=0;
                    }
                    else{
                        symTab[findSymbol(inst[i-2])].isSym = 1;
                        if(isNum(inst[i])){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].relative = 0;
                            symTab[findSymbol(inst[i-2])].addr = stoi(inst[i]);
                        }
                        else if(findSymbol(inst[i])!=-1){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].parent = inst[i];
                            symTab[findSymbol(inst[i-2])].addr = symTab[findSymbol(inst[i])].addr;
                            symTab[findSymbol(inst[i-2])].relative = symTab[findSymbol(inst[i])].relative;    
                        }
                        else{
                            if(inst[i]=="*"){
                                symTab[findSymbol(inst[i-2])].addr = curr_addr;
                            }
                            else if(containsArithmetic(inst[i])){
                                Symbol expression(inst[i]);
                                symTab[findSymbol(inst[i-2])].parent = inst[i];
                                symTab.push_back(expression);
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": Invalid declaration of symbol!\n";
                                returnInt=0;
                            }    
                        }
                    }
                }
            }
            if(inst[i][0]=='+'){
                form4 = 1;
            }
            if((opCode.find(inst[i]) != opCode.end()) or (form4 and (opCode.find(inst[i].substr(1)) != opCode.end()))){
                if(!flag){
                    flag=1;
                }
                else{
                    cout<<"error at line "<<iterator+1<<"\n";
                    returnInt=0;
                }
                if(inst[i][0]=='+'){
                    opcode = opCode[inst[i].substr(1)].second;
                    format_supported = opCode[inst[i].substr(1)].first;
                }
                else{
                    opcode = opCode[inst[i]].second;
                    format_supported = opCode[inst[i]].first;
                }
                if(inst[i][0] == '+'){
                    if(format_supported == 4){
                        curr_format = 4;
                    }
                    else{
                        cout<<"error at line "<<iterator+1<<": Format 4 not supported for this opcode!\n";
                        returnInt=0;
                    }
                }
                else if(format_supported==4){
                    curr_format = 3;
                }
                else{
                    curr_format = format_supported;
                }
            }
            if(findinvec(assmDir,inst[i])!=-1){
                if(!flag){
                    flag=1;
                    if(inst[i]=="BYTE"){
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                        else{
                            if(inst[i+1][0]=='C' or inst[i+1][0]=='c' or inst[i+1][0]=='X'){
                                if(inst[i+1][0]=='C' or inst[i+1][0]=='c'){
                                    curr_format = inst[i+1].size() - 3;
                                }
                                else{
                                    curr_format = (inst[i+1].size() - 3)/2;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                                returnInt=0;
                            }
                        }
                    }
                    else if(inst[i]=="WORD"){
                        curr_format = 3;
                        //INCLUDE FOR EXPRESSION
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESB"){
                        if(inst.size()==(i+2)) curr_format = stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESW"){
                        if(inst.size()==(i+2)) curr_format = 3*stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="BASE"){
                        if(inst.size()==(i+2)) curr_format = 0;
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="ORG"){
                        if(inst.size()!=2){
                            if(org_address==-1){
                                cout<<"error at line "<<iterator+1<<" : ORG statement incomplete!\n";
                                returnInt=0;
                            }
                            else{
                                curr_format = org_address - curr_addr;
                            }
                        }
                    }
                    else if(inst[i]=="LTORG"){
                        if(inst.size()!=1){
                            cout<<"error at line "<<iterator+1<<" : Invalid LTORG statement!\n";
                            returnInt=0;
                        }
                        else{
                            counter_literal = 0;
                            for(int i=0;i<litTab.size();i++){
                                vector<string> vec;
                                if(litTab[i].written==0){
                                    vec.push_back("*");
                                    vec.push_back('='+litTab[i].name);
                                    litTab[i].written = 1;
                                }
                                instructions.insert(instructions.begin()+iterator+1+counter_literal, vec);
                                counter_literal++;
                            }
                        }
                    }
                }
                else{
                    cout<<"error at line "<<iterator+1<<"!\n";
                    returnInt=0;
                }
            }
            if(i==0 and flag==0){
                if(inst[i]!="*"){
                    Symbol label(inst[i],curr_addr);
                    if(findSymbol(inst[i])==-1 or symTab[findSymbol(inst[i])].addr==-1){
                        if(findSymbol(inst[i])==-1){
                            symTab.push_back(label);
                        }
                        else{
                            symTab[findSymbol(inst[i])].addr = curr_addr;
                        }    
                    }
                    else{
                        cout<<"error at line "<<iterator+1<<": Symbol "<<inst[i]<<" has multiple instances!\n";
                        returnInt=0;
                    }
                }
            }
            if(i==1 and flag==0){
                if(inst[i-1]=="*"){
                    curr_format = litTab[findLiteral(inst[i].substr(1))].size;
                    litTab[findLiteral(inst[i].substr(1))].addr = curr_addr;
                }
                else{
                    cout<<"error at line "<<iterator+1<<": No OPCODE or ASSEMBLER DIRECTIVE FOUND!\n";
                    returnInt=0;
                }
            }

        }
        locctr.push_back(curr_addr);
        curr_addr += curr_format;
    }
    for(int i=0;i<symTab.size();i++){
        if(containsArithmetic(symTab[i].name)){
            if(symTab[i].addr == -1){
                if(isExpression(symTab[i].name)!=INT_MAX){
                    symTab[i].addr = isExpression(symTab[i].name);
                    symTab[i].relative = absOrRelative(symTab[i].name);
                    symTab[i].isSym = 1;
                }
                else{
                    cout<<"error: Expression "<<symTab[i].name<<" is not a valid expression!\n";
                    returnInt=0;
                }
            }
            else{
                cout<<"error: BT HO GAYI! "<<symTab[i].name<<" HAGG RAHA HAI!\n";            //HATANA HAI!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                returnInt=0;
            }
        }
    }
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].parent != ""){
            symTab[i].addr = symTab[findSymbol(symTab[i].parent)].addr;
            symTab[i].relative = symTab[findSymbol(symTab[i].parent)].relative;
        }
        if(symTab[i].addr==-1){
            cout<<"error: Label "<<symTab[i].name<<" is accessed without definition!\n";
            returnInt=0;
        }
    }
    for(int i=0;i<litTab.size();i++){
        vector<string> vec;
        if(litTab[i].written==0){
            vec.push_back("*");
            vec.push_back('='+litTab[i].name);
            litTab[i].written = 1;
            instructions.insert(instructions.begin()+instructions.size()-1, vec);
            litTab[i].addr = curr_addr;
            locctr.push_back(curr_addr);
            curr_addr = curr_addr + litTab[i].size;
        }
    }
    locctr.push_back(curr_addr);
    programLength = inttohex(curr_addr - locctr[0]);

    return returnInt;
}

int firstPass1(vector<vector<string>> &instructions){
    int returnInt=1;
    programName=instructions[0][0];
    org_address = -1;
    if(programName.size()>6){
        cout<<"error at line 1: NAME OF PROGRAM ENTERED IS MORE THAN 6 COLUMNS!\n";
        returnInt=0;
    }
    if(instructions[0].size()>1){
        if(instructions[0][1]!="START"){
            cout<<"error at line 1: START assembler directive not found"<<endl;
            returnInt=0;
        }
        else{
            if(instructions[0].size()==3) startAddr=hextoint(instructions[0][2]);
            if(instructions[0].size()>3){
                cout<<"error at line 1\n";
                returnInt=0;
            }
        }
    }
    locctr.push_back(startAddr);
    int curr_addr = startAddr;
    int qq=1;
    if(instructions[qq][0]=="EXTDEF"){
        for(int x=1;x<instructions[qq].size();x++){
            extDef.push_back(instructions[qq][x]);
        }
        locctr.push_back(startAddr);
        qq++;
    }
    if(instructions[qq][0]=="EXTREF"){
        for(int x=1;x<instructions[qq].size();x++){
            extRef.push_back(instructions[qq][x]);
        }
        locctr.push_back(startAddr);
        qq++;
    }
    for(int iterator=qq;iterator<instructions.size();iterator++){
        vector<string> inst=instructions[iterator];
        int format_supported;
        int curr_format = 0;
        int counter_literal=0;
        string opcode;
        bool flag=0;
        bool form4 = 0;
        for(int i=0;i<inst.size();i++){
            if(flag and format_supported==1){
                cout<<"error at line "<<iterator+1<<" : INVALID INSTRUCTION FORMAT!\n";
                returnInt=0;
            }
            form4=0;
            if(flag and inst[i-1]!="BYTE" and inst[i-1]!="EQU"){
                if(findinvec(registers,inst[i])==-1){
                    //NOT A REGISTER
                    if(inst[i][0]=='#' or inst[i][0]=='@'){
                        if(!isNum(inst[i].substr(1))){
                            //#LABEL or @LABEL
                            if(findinvec(extRef, inst[i].substr(1))!=-1){
                                //EXTERNAL REF LABEL
                                if(inst[i-1]!="WORD" and inst[i-1][0]!='+'){
                                    cout<<"error in usage of external reference\n";
                                    returnInt=0;
                                }
                            }
                            else if(findSymbol(inst[i].substr(1))==-1){
                                if(containsArithmetic(inst[i].substr(1))){
                                    if(detectRExp(inst[i].substr(1))==-1){
                                        cout<<"error at line "<<iterator+1<<": invalid expression\n";
                                        returnInt=0;
                                    }
                                    else if(detectRExp(inst[i].substr(1))==1){
                                        vector<string> vv=breakRExp(inst[i].substr(1));
                                        if(vv[0]!=""){
                                            Symbol operand(vv[0]);
                                            symTab.push_back(operand);
                                        }
                                    }
                                    else{
                                        Symbol operand(inst[i].substr(1));
                                        symTab.push_back(operand);
                                    }
                                }
                                else{
                                    Symbol operand(inst[i].substr(1));
                                    symTab.push_back(operand);
                                }
                            }
                        }
                    }
                    else if(inst[i][0]=='='){
                        if(inst[i][1]=='C' or inst[i][1]=='X'){
                            Literal lit(inst[i].substr(1));
                            if(findLiteral(inst[i].substr(1))==-1){
                                litTab.push_back(lit);
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": Invalid Literal definition!\n";
                            returnInt=0; 
                        }
                    }
                    else{
                        if(!isNum(inst[i])){
                            //LABEL
                            if(findinvec(extRef, inst[i])!=-1){
                                //EXTERNAL REF LABEL
                                if(inst[i-1]!="WORD" and inst[i-1][0]!='+'){
                                    cout<<"error in usage of external reference\n";
                                    returnInt=0;
                                }
                            }
                            else if(findSymbol(inst[i])==-1){
                                if(containsArithmetic(inst[i])){
                                    if(detectRExp(inst[i])==-1){
                                        cout<<"error at line "<<iterator+1<<": invalid expression\n";
                                        returnInt=0;
                                    }
                                    else if(detectRExp(inst[i])==1){
                                        vector<string> vv=breakRExp(inst[i]);
                                        if(vv[0]!=""){
                                            Symbol operand(vv[0]);
                                            symTab.push_back(operand);
                                        }
                                    }
                                    else{
                                        Symbol operand(inst[i]);
                                        symTab.push_back(operand);
                                    }
                                }
                                else{
                                    Symbol operand(inst[i]);
                                    symTab.push_back(operand);
                                }
                            }
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="ORG"){
                if(inst.size()!=2){
                    cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT!\n";
                    returnInt=0;
                }
                else{
                    if(isNum(inst[i])==1){
                        curr_format = stoi(inst[i]) - curr_addr;
                        org_address = curr_addr;
                    }
                    else{
                        if(findSymbol(inst[i])!=-1 and symTab[findSymbol(inst[i])].addr != -1){
                            curr_format = symTab[findSymbol(inst[i])].addr - curr_addr;
                            org_address = curr_addr;
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT AS FORWARD REFERENCE IS NOT ALLOWED!\n";
                            returnInt=0;
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="EQU"){
                if((i-2)!=0){
                    cout<<"error at line "<<iterator+1<<": NO SYMBOL NAME!\n";
                    returnInt=0;
                }
                else{
                    if(findSymbol(inst[i-2])!=-1 and symTab[findSymbol(inst[i-2])].addr != curr_addr){
                        cout<<"error at line "<<iterator+1<<": Forward reference in the symbol "<<inst[i-2]<<"!\n";
                        returnInt=0;
                    }
                    else{
                        symTab[findSymbol(inst[i-2])].isSym = 1;
                        if(isNum(inst[i])){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].relative = 0;
                            symTab[findSymbol(inst[i-2])].addr = stoi(inst[i]);
                        }
                        else if(findSymbol(inst[i])!=-1){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].parent = inst[i];
                            symTab[findSymbol(inst[i-2])].addr = symTab[findSymbol(inst[i])].addr;
                            symTab[findSymbol(inst[i-2])].relative = symTab[findSymbol(inst[i])].relative;    
                        }
                        else{
                            if(inst[i]=="*"){
                                symTab[findSymbol(inst[i-2])].addr = curr_addr;
                            }
                            else if(containsArithmetic(inst[i])){
                                Symbol expression(inst[i]);
                                symTab[findSymbol(inst[i-2])].parent = inst[i];
                                symTab.push_back(expression);
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": Invalid declaration of symbol!\n";
                                returnInt=0;
                            }    
                        }
                    }
                }
            }
            if(inst[i][0]=='+'){
                form4 = 1;
            }
            if((opCode.find(inst[i]) != opCode.end()) or (form4 and (opCode.find(inst[i].substr(1)) != opCode.end()))){
                if(!flag){
                    flag=1;
                }
                else{
                    cout<<"error at line "<<iterator+1<<"\n";
                    returnInt=0;
                }
                if(inst[i][0]=='+'){
                    opcode = opCode[inst[i].substr(1)].second;
                    format_supported = opCode[inst[i].substr(1)].first;
                }
                else{
                    opcode = opCode[inst[i]].second;
                    format_supported = opCode[inst[i]].first;
                }
                if(inst[i][0] == '+'){
                    if(format_supported == 4){
                        curr_format = 4;
                    }
                    else{
                        cout<<"error at line "<<iterator+1<<": Format 4 not supported for this opcode!\n";
                        returnInt=0;
                    }
                }
                else if(format_supported==4){
                    curr_format = 3;
                }
                else{
                    curr_format = format_supported;
                }
            }
            if(findinvec(assmDir,inst[i])!=-1){
                if(!flag){
                    flag=1;
                    if(inst[i]=="BYTE"){
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                        else{
                            if(inst[i+1][0]=='C' or inst[i+1][0]=='c' or inst[i+1][0]=='X'){
                                if(inst[i+1][0]=='C' or inst[i+1][0]=='c'){
                                    curr_format = inst[i+1].size() - 3;
                                }
                                else{
                                    curr_format = (inst[i+1].size() - 3)/2;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                                returnInt=0;
                            }
                        }
                    }
                    else if(inst[i]=="WORD"){
                        curr_format = 3;
                        //INCLUDE FOR EXPRESSION
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESB"){
                        if(inst.size()==(i+2)) curr_format = stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESW"){
                        if(inst.size()==(i+2)) curr_format = 3*stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="BASE"){
                        if(inst.size()==(i+2)) curr_format = 0;
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="ORG"){
                        if(inst.size()!=2){
                            if(org_address==-1){
                                cout<<"error at line "<<iterator+1<<" : ORG statement incomplete!\n";
                                returnInt=0;
                            }
                            else{
                                curr_format = org_address - curr_addr;
                            }
                        }
                    }
                    else if(inst[i]=="LTORG"){
                        if(inst.size()!=1){
                            cout<<"error at line "<<iterator+1<<" : Invalid LTORG statement!\n";
                            returnInt=0;
                        }
                        else{
                            counter_literal = 0;
                            for(int i=0;i<litTab.size();i++){
                                vector<string> vec;
                                if(litTab[i].written==0){
                                    vec.push_back("*");
                                    vec.push_back('='+litTab[i].name);
                                    litTab[i].written = 1;
                                }
                                instructions.insert(instructions.begin()+iterator+1+counter_literal, vec);
                                counter_literal++;
                            }
                        }
                    }
                }
                else{
                    cout<<"error at line "<<iterator+1<<"!\n";
                    returnInt=0;
                }
            }
            if(i==0 and flag==0){
                if(inst[i]!="*"){
                    if(findinvec(extRef, inst[i])!=-1){
                        cout<<"error: external reference symbol defined here!\n";
                        returnInt=0;
                    }
                    else{
                        Symbol label(inst[i],curr_addr);
                        if(findSymbol(inst[i])==-1 or symTab[findSymbol(inst[i])].addr==-1){
                            if(findSymbol(inst[i])==-1){
                                symTab.push_back(label);
                            }
                            else{
                                symTab[findSymbol(inst[i])].addr = curr_addr;
                            }    
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": Symbol "<<inst[i]<<" has multiple instances!\n";
                            returnInt=0;
                        }
                    }
                }
            }
            if(i==1 and flag==0){
                if(inst[i-1]=="*"){
                    curr_format = litTab[findLiteral(inst[i].substr(1))].size;
                    litTab[findLiteral(inst[i].substr(1))].addr = curr_addr;
                }
                else{
                    cout<<"error at line "<<iterator+1<<": No OPCODE or ASSEMBLER DIRECTIVE FOUND!\n";
                    returnInt=0;
                }
            }

        }
        locctr.push_back(curr_addr);
        curr_addr += curr_format;
    }
    for(int i=0;i<symTab.size();i++){
        if(containsArithmetic(symTab[i].name)){
            if(symTab[i].addr == -1){
                if(isExpression(symTab[i].name)!=INT_MAX){
                    symTab[i].addr = isExpression(symTab[i].name);
                    symTab[i].relative = absOrRelative(symTab[i].name);
                    symTab[i].isSym = 1;
                }
                else{
                    cout<<"error: Expression "<<symTab[i].name<<" is not a valid expression!\n";
                    returnInt=0;
                }
            }
            else{
                cout<<"error: BT HO GAYI! "<<symTab[i].name<<" HAGG RAHA HAI!\n";            //HATANA HAI!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                returnInt=0;
            }
        }
    }
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].parent != ""){
            symTab[i].addr = symTab[findSymbol(symTab[i].parent)].addr;
            symTab[i].relative = symTab[findSymbol(symTab[i].parent)].relative;
        }
        if(symTab[i].addr==-1){
            cout<<"error: Label "<<symTab[i].name<<" is accessed without definition!\n";
            returnInt=0;
        }
    }
    for(int i=0;i<litTab.size();i++){
        vector<string> vec;
        if(litTab[i].written==0){
            vec.push_back("*");
            vec.push_back('='+litTab[i].name);
            litTab[i].written = 1;
            instructions.insert(instructions.begin()+instructions.size(), vec);
            litTab[i].addr = curr_addr;
            locctr.push_back(curr_addr);
            curr_addr = curr_addr + litTab[i].size;
        }
    }
    locctr.push_back(curr_addr);
    programLength = inttohex(curr_addr - locctr[0]);

    return returnInt;
}

int firstPass2(vector<vector<string>> &instructions){
    int returnInt=1;
    programName=instructions[0][0];
    org_address = -1;
    if(programName.size()>6){
        cout<<"error at line 1: NAME OF PROGRAM ENTERED IS MORE THAN 6 COLUMNS!\n";
        returnInt=0;
    }
    if(instructions[0].size()>1){
        if(instructions[0][1]!="CSECT"){
            cout<<"error at line 1: CSECT assembler directive not found"<<endl;
            returnInt=0;
        }
        else{
            if(instructions[0].size()==3) startAddr=hextoint(instructions[0][2]);
            if(instructions[0].size()>3){
                cout<<"error at line 1\n";
                returnInt=0;
            }
        }
    }
    locctr.push_back(startAddr);
    int curr_addr = startAddr;
    int qq=1;
    if(instructions[qq][0]=="EXTDEF"){
        for(int x=1;x<instructions[qq].size();x++){
            extDef.push_back(instructions[qq][x]);
        }
        locctr.push_back(startAddr);
        qq++;
    }
    if(instructions[qq][0]=="EXTREF"){
        for(int x=1;x<instructions[qq].size();x++){
            extRef.push_back(instructions[qq][x]);
        }
        locctr.push_back(startAddr);
        qq++;
    }
    for(int iterator=qq;iterator<instructions.size();iterator++){
        vector<string> inst=instructions[iterator];
        int format_supported;
        int curr_format = 0;
        int counter_literal=0;
        string opcode;
        bool flag=0;
        bool form4 = 0;
        for(int i=0;i<inst.size();i++){
            if(flag and format_supported==1){
                cout<<"error at line "<<iterator+1<<" : INVALID INSTRUCTION FORMAT!\n";
                returnInt=0;
            }
            form4=0;
            if(flag and inst[i-1]!="BYTE" and inst[i-1]!="EQU"){
                if(findinvec(registers,inst[i])==-1){
                    //NOT A REGISTER
                    if(inst[i][0]=='#' or inst[i][0]=='@'){
                        if(!isNum(inst[i].substr(1))){
                            //#LABEL or @LABEL
                            if(findinvec(extRef, inst[i].substr(1))!=-1){
                                //EXTERNAL REF LABEL
                                if(inst[i-1]!="WORD" and inst[i-1][0]!='+'){
                                    cout<<"error in usage of external reference\n";
                                    returnInt=0;
                                }
                            }
                            else if(findSymbol(inst[i].substr(1))==-1){
                                if(containsArithmetic(inst[i].substr(1))){
                                    if(detectRExp(inst[i].substr(1))==-1){
                                        cout<<"error at line "<<iterator+1<<": invalid expression\n";
                                        returnInt=0;
                                    }
                                    else if(detectRExp(inst[i].substr(1))==1){
                                        vector<string> vv=breakRExp(inst[i].substr(1));
                                        if(vv[0]!=""){
                                            Symbol operand(vv[0]);
                                            symTab.push_back(operand);
                                        }
                                    }
                                    else{
                                        Symbol operand(inst[i].substr(1));
                                        symTab.push_back(operand);
                                    }
                                }
                                else{
                                    Symbol operand(inst[i].substr(1));
                                    symTab.push_back(operand);
                                }
                            }
                        }
                    }
                    else if(inst[i][0]=='='){
                        if(inst[i][1]=='C' or inst[i][1]=='X'){
                            Literal lit(inst[i].substr(1));
                            if(findLiteral(inst[i].substr(1))==-1){
                                litTab.push_back(lit);
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": Invalid Literal definition!\n";
                            returnInt=0; 
                        }
                    }
                    else{
                        if(!isNum(inst[i])){
                            //LABEL
                            if(findinvec(extRef, inst[i])!=-1){
                                //EXTERNAL REF LABEL
                                if(inst[i-1]!="WORD" and inst[i-1][0]!='+'){
                                    cout<<"error in usage of external reference\n";
                                    returnInt=0;
                                }
                            }
                            else if(findSymbol(inst[i])==-1){
                                if(containsArithmetic(inst[i])){
                                    if(detectRExp(inst[i])==-1){
                                        cout<<"error at line "<<iterator+1<<": invalid expression\n";
                                        returnInt=0;
                                    }
                                    else if(detectRExp(inst[i])==1){
                                        vector<string> vv=breakRExp(inst[i]);
                                        if(vv[0]!=""){
                                            Symbol operand(vv[0]);
                                            symTab.push_back(operand);
                                        }
                                    }
                                    else{
                                        Symbol operand(inst[i]);
                                        symTab.push_back(operand);
                                    }
                                }
                                else{
                                    Symbol operand(inst[i]);
                                    symTab.push_back(operand);
                                }
                            }
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="ORG"){
                if(inst.size()!=2){
                    cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT!\n";
                    returnInt=0;
                }
                else{
                    if(isNum(inst[i])==1){
                        curr_format = stoi(inst[i]) - curr_addr;
                        org_address = curr_addr;
                    }
                    else{
                        if(findSymbol(inst[i])!=-1 and symTab[findSymbol(inst[i])].addr != -1){
                            curr_format = symTab[findSymbol(inst[i])].addr - curr_addr;
                            org_address = curr_addr;
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT AS FORWARD REFERENCE IS NOT ALLOWED!\n";
                            returnInt=0;
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="EQU"){
                if((i-2)!=0){
                    cout<<"error at line "<<iterator+1<<": NO SYMBOL NAME!\n";
                    returnInt=0;
                }
                else{
                    if(findSymbol(inst[i-2])!=-1 and symTab[findSymbol(inst[i-2])].addr != curr_addr){
                        cout<<"error at line "<<iterator+1<<": Forward reference in the symbol "<<inst[i-2]<<"!\n";
                        returnInt=0;
                    }
                    else{
                        symTab[findSymbol(inst[i-2])].isSym = 1;
                        if(isNum(inst[i])){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].relative = 0;
                            symTab[findSymbol(inst[i-2])].addr = stoi(inst[i]);
                        }
                        else if(findSymbol(inst[i])!=-1){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].parent = inst[i];
                            symTab[findSymbol(inst[i-2])].addr = symTab[findSymbol(inst[i])].addr;
                            symTab[findSymbol(inst[i-2])].relative = symTab[findSymbol(inst[i])].relative;    
                        }
                        else{
                            if(inst[i]=="*"){
                                symTab[findSymbol(inst[i-2])].addr = curr_addr;
                            }
                            else if(containsArithmetic(inst[i])){
                                Symbol expression(inst[i]);
                                symTab[findSymbol(inst[i-2])].parent = inst[i];
                                symTab.push_back(expression);
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": Invalid declaration of symbol!\n";
                                returnInt=0;
                            }    
                        }
                    }
                }
            }
            if(inst[i][0]=='+'){
                form4 = 1;
            }
            if((opCode.find(inst[i]) != opCode.end()) or (form4 and (opCode.find(inst[i].substr(1)) != opCode.end()))){
                if(!flag){
                    flag=1;
                }
                else{
                    cout<<"error at line "<<iterator+1<<"\n";
                    returnInt=0;
                }
                if(inst[i][0]=='+'){
                    opcode = opCode[inst[i].substr(1)].second;
                    format_supported = opCode[inst[i].substr(1)].first;
                }
                else{
                    opcode = opCode[inst[i]].second;
                    format_supported = opCode[inst[i]].first;
                }
                if(inst[i][0] == '+'){
                    if(format_supported == 4){
                        curr_format = 4;
                    }
                    else{
                        cout<<"error at line "<<iterator+1<<": Format 4 not supported for this opcode!\n";
                        returnInt=0;
                    }
                }
                else if(format_supported==4){
                    curr_format = 3;
                }
                else{
                    curr_format = format_supported;
                }
            }
            if(findinvec(assmDir,inst[i])!=-1){
                if(!flag){
                    flag=1;
                    if(inst[i]=="BYTE"){
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                        else{
                            if(inst[i+1][0]=='C' or inst[i+1][0]=='c' or inst[i+1][0]=='X'){
                                if(inst[i+1][0]=='C' or inst[i+1][0]=='c'){
                                    curr_format = inst[i+1].size() - 3;
                                }
                                else{
                                    curr_format = (inst[i+1].size() - 3)/2;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                                returnInt=0;
                            }
                        }
                    }
                    else if(inst[i]=="WORD"){
                        curr_format = 3;
                        //INCLUDE FOR EXPRESSION
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESB"){
                        if(inst.size()==(i+2)) curr_format = stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESW"){
                        if(inst.size()==(i+2)) curr_format = 3*stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="BASE"){
                        if(inst.size()==(i+2)) curr_format = 0;
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="ORG"){
                        if(inst.size()!=2){
                            if(org_address==-1){
                                cout<<"error at line "<<iterator+1<<" : ORG statement incomplete!\n";
                                returnInt=0;
                            }
                            else{
                                curr_format = org_address - curr_addr;
                            }
                        }
                    }
                    else if(inst[i]=="LTORG"){
                        if(inst.size()!=1){
                            cout<<"error at line "<<iterator+1<<" : Invalid LTORG statement!\n";
                            returnInt=0;
                        }
                        else{
                            counter_literal = 0;
                            for(int i=0;i<litTab.size();i++){
                                vector<string> vec;
                                if(litTab[i].written==0){
                                    vec.push_back("*");
                                    vec.push_back('='+litTab[i].name);
                                    litTab[i].written = 1;
                                }
                                instructions.insert(instructions.begin()+iterator+1+counter_literal, vec);
                                counter_literal++;
                            }
                        }
                    }
                }
                else{
                    cout<<"error at line "<<iterator+1<<"!\n";
                    returnInt=0;
                }
            }
            if(i==0 and flag==0){
                if(inst[i]!="*"){
                    if(findinvec(extRef, inst[i])!=-1){
                        cout<<"error: external reference symbol defined here!\n";
                        returnInt=0;
                    }
                    else{
                        Symbol label(inst[i],curr_addr);
                        if(findSymbol(inst[i])==-1 or symTab[findSymbol(inst[i])].addr==-1){
                            if(findSymbol(inst[i])==-1){
                                symTab.push_back(label);
                            }
                            else{
                                symTab[findSymbol(inst[i])].addr = curr_addr;
                            }    
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": Symbol "<<inst[i]<<" has multiple instances!\n";
                            returnInt=0;
                        }
                    }
                }
            }
            if(i==1 and flag==0){
                if(inst[i-1]=="*"){
                    curr_format = litTab[findLiteral(inst[i].substr(1))].size;
                    litTab[findLiteral(inst[i].substr(1))].addr = curr_addr;
                }
                else{
                    cout<<"error at line "<<iterator+1<<": No OPCODE or ASSEMBLER DIRECTIVE FOUND!\n";
                    returnInt=0;
                }
            }

        }
        locctr.push_back(curr_addr);
        curr_addr += curr_format;
    }
    for(int i=0;i<symTab.size();i++){
        if(containsArithmetic(symTab[i].name)){
            if(symTab[i].addr == -1){
                if(isExpression(symTab[i].name)!=INT_MAX){
                    symTab[i].addr = isExpression(symTab[i].name);
                    symTab[i].relative = absOrRelative(symTab[i].name);
                    symTab[i].isSym = 1;
                }
                else{
                    cout<<"error: Expression "<<symTab[i].name<<" is not a valid expression!\n";
                    returnInt=0;
                }
            }
            else{
                cout<<"error: BT HO GAYI! "<<symTab[i].name<<" HAGG RAHA HAI!\n";            //HATANA HAI!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                returnInt=0;
            }
        }
    }
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].parent != ""){
            symTab[i].addr = symTab[findSymbol(symTab[i].parent)].addr;
            symTab[i].relative = symTab[findSymbol(symTab[i].parent)].relative;
        }
        if(symTab[i].addr==-1){
            cout<<"error: Label "<<symTab[i].name<<" is accessed without definition!\n";
            returnInt=0;
        }
    }
    for(int i=0;i<litTab.size();i++){
        vector<string> vec;
        if(litTab[i].written==0){
            vec.push_back("*");
            vec.push_back('='+litTab[i].name);
            litTab[i].written = 1;
            instructions.insert(instructions.begin()+instructions.size(), vec);
            litTab[i].addr = curr_addr;
            locctr.push_back(curr_addr);
            curr_addr = curr_addr + litTab[i].size;
        }
    }
    locctr.push_back(curr_addr);
    programLength = inttohex(curr_addr - locctr[0]);

    return returnInt;
}

int firstPass3(vector<vector<string>> &instructions){
    int returnInt=1;
    programName=instructions[0][0];
    org_address = -1;
    if(programName.size()>6){
        cout<<"error at line 1: NAME OF PROGRAM ENTERED IS MORE THAN 6 COLUMNS!\n";
        returnInt=0;
    }
    if(instructions[0].size()>1){
        if(instructions[0][1]!="CSECT"){
            cout<<"error at line 1: CSECT assembler directive not found"<<endl;
            returnInt=0;
        }
        else{
            if(instructions[0].size()==3) startAddr=hextoint(instructions[0][2]);
            if(instructions[0].size()>3){
                cout<<"error at line 1\n";
                returnInt=0;
            }
        }
    }
    locctr.push_back(startAddr);
    int curr_addr = startAddr;
    int qq=1;
    if(instructions[qq][0]=="EXTDEF"){
        for(int x=1;x<instructions[qq].size();x++){
            extDef.push_back(instructions[qq][x]);
        }
        locctr.push_back(startAddr);
        qq++;
    }
    if(instructions[qq][0]=="EXTREF"){
        for(int x=1;x<instructions[qq].size();x++){
            extRef.push_back(instructions[qq][x]);
        }
        locctr.push_back(startAddr);
        qq++;
    }
    for(int iterator=qq;iterator<instructions.size()-1;iterator++){
        vector<string> inst=instructions[iterator];
        int format_supported;
        int curr_format = 0;
        int counter_literal=0;
        string opcode;
        bool flag=0;
        bool form4 = 0;
        for(int i=0;i<inst.size();i++){
            if(flag and format_supported==1){
                cout<<"error at line "<<iterator+1<<" : INVALID INSTRUCTION FORMAT!\n";
                returnInt=0;
            }
            form4=0;
            if(flag and inst[i-1]!="BYTE" and inst[i-1]!="EQU"){
                if(findinvec(registers,inst[i])==-1){
                    //NOT A REGISTER
                    if(inst[i][0]=='#' or inst[i][0]=='@'){
                        if(!isNum(inst[i].substr(1))){
                            //#LABEL or @LABEL
                            if(findinvec(extRef, inst[i].substr(1))!=-1){
                                //EXTERNAL REF LABEL
                                if(inst[i-1]!="WORD" and inst[i-1][0]!='+'){
                                    cout<<"error in usage of external reference\n";
                                    returnInt=0;
                                }
                            }
                            else if(findSymbol(inst[i].substr(1))==-1){
                                if(containsArithmetic(inst[i].substr(1))){
                                    if(detectRExp(inst[i].substr(1))==-1){
                                        cout<<"error at line "<<iterator+1<<": invalid expression\n";
                                        returnInt=0;
                                    }
                                    else if(detectRExp(inst[i].substr(1))==1){
                                        vector<string> vv=breakRExp(inst[i].substr(1));
                                        if(vv[0]!=""){
                                            Symbol operand(vv[0]);
                                            symTab.push_back(operand);
                                        }
                                    }
                                    else{
                                        Symbol operand(inst[i].substr(1));
                                        symTab.push_back(operand);
                                    }
                                }
                                else{
                                    Symbol operand(inst[i].substr(1));
                                    symTab.push_back(operand);
                                }
                            }
                        }
                    }
                    else if(inst[i][0]=='='){
                        if(inst[i][1]=='C' or inst[i][1]=='X'){
                            Literal lit(inst[i].substr(1));
                            if(findLiteral(inst[i].substr(1))==-1){
                                litTab.push_back(lit);
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": Invalid Literal definition!\n";
                            returnInt=0; 
                        }
                    }
                    else{
                        if(!isNum(inst[i])){
                            //LABEL
                            if(findinvec(extRef, inst[i])!=-1){
                                //EXTERNAL REF LABEL
                                if(inst[i-1]!="WORD" and inst[i-1][0]!='+'){
                                    cout<<"error in usage of external reference\n";
                                    returnInt=0;
                                }
                            }
                            else if(findSymbol(inst[i])==-1){
                                if(containsArithmetic(inst[i])){
                                    if(detectRExp(inst[i])==-1){
                                        cout<<"error at line "<<iterator+1<<": invalid expression\n";
                                        returnInt=0;
                                    }
                                    else if(detectRExp(inst[i])==1){
                                        vector<string> vv=breakRExp(inst[i]);
                                        if(vv[0]!=""){
                                            Symbol operand(vv[0]);
                                            symTab.push_back(operand);
                                        }
                                    }
                                    else{
                                        Symbol operand(inst[i]);
                                        symTab.push_back(operand);
                                    }
                                }
                                else{
                                    Symbol operand(inst[i]);
                                    symTab.push_back(operand);
                                }
                            }
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="ORG"){
                if(inst.size()!=2){
                    cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT!\n";
                    returnInt=0;
                }
                else{
                    if(isNum(inst[i])==1){
                        curr_format = stoi(inst[i]) - curr_addr;
                        org_address = curr_addr;
                    }
                    else{
                        if(findSymbol(inst[i])!=-1 and symTab[findSymbol(inst[i])].addr != -1){
                            curr_format = symTab[findSymbol(inst[i])].addr - curr_addr;
                            org_address = curr_addr;
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID ORG STATEMENT AS FORWARD REFERENCE IS NOT ALLOWED!\n";
                            returnInt=0;
                        }
                    }
                }
            }
            if(flag and inst[i-1]=="EQU"){
                if((i-2)!=0){
                    cout<<"error at line "<<iterator+1<<": NO SYMBOL NAME!\n";
                    returnInt=0;
                }
                else{
                    if(findSymbol(inst[i-2])!=-1 and symTab[findSymbol(inst[i-2])].addr != curr_addr){
                        cout<<"error at line "<<iterator+1<<": Forward reference in the symbol "<<inst[i-2]<<"!\n";
                        returnInt=0;
                    }
                    else{
                        symTab[findSymbol(inst[i-2])].isSym = 1;
                        if(isNum(inst[i])){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].relative = 0;
                            symTab[findSymbol(inst[i-2])].addr = stoi(inst[i]);
                        }
                        else if(findSymbol(inst[i])!=-1){
                            symTab[findSymbol(inst[i-2])].isSym = 1;
                            symTab[findSymbol(inst[i-2])].parent = inst[i];
                            symTab[findSymbol(inst[i-2])].addr = symTab[findSymbol(inst[i])].addr;
                            symTab[findSymbol(inst[i-2])].relative = symTab[findSymbol(inst[i])].relative;    
                        }
                        else{
                            if(inst[i]=="*"){
                                symTab[findSymbol(inst[i-2])].addr = curr_addr;
                            }
                            else if(containsArithmetic(inst[i])){
                                Symbol expression(inst[i]);
                                symTab[findSymbol(inst[i-2])].parent = inst[i];
                                symTab.push_back(expression);
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": Invalid declaration of symbol!\n";
                                returnInt=0;
                            }    
                        }
                    }
                }
            }
            if(inst[i][0]=='+'){
                form4 = 1;
            }
            if((opCode.find(inst[i]) != opCode.end()) or (form4 and (opCode.find(inst[i].substr(1)) != opCode.end()))){
                if(!flag){
                    flag=1;
                }
                else{
                    cout<<"error at line "<<iterator+1<<"\n";
                    returnInt=0;
                }
                if(inst[i][0]=='+'){
                    opcode = opCode[inst[i].substr(1)].second;
                    format_supported = opCode[inst[i].substr(1)].first;
                }
                else{
                    opcode = opCode[inst[i]].second;
                    format_supported = opCode[inst[i]].first;
                }
                if(inst[i][0] == '+'){
                    if(format_supported == 4){
                        curr_format = 4;
                    }
                    else{
                        cout<<"error at line "<<iterator+1<<": Format 4 not supported for this opcode!\n";
                        returnInt=0;
                    }
                }
                else if(format_supported==4){
                    curr_format = 3;
                }
                else{
                    curr_format = format_supported;
                }
            }
            if(findinvec(assmDir,inst[i])!=-1){
                if(!flag){
                    flag=1;
                    if(inst[i]=="BYTE"){
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                        else{
                            if(inst[i+1][0]=='C' or inst[i+1][0]=='c' or inst[i+1][0]=='X'){
                                if(inst[i+1][0]=='C' or inst[i+1][0]=='c'){
                                    curr_format = inst[i+1].size() - 3;
                                }
                                else{
                                    curr_format = (inst[i+1].size() - 3)/2;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                                returnInt=0;
                            }
                        }
                    }
                    else if(inst[i]=="WORD"){
                        curr_format = 3;
                        //INCLUDE FOR EXPRESSION
                        if(inst.size()!=(i+2)){
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESB"){
                        if(inst.size()==(i+2)) curr_format = stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="RESW"){
                        if(inst.size()==(i+2)) curr_format = 3*stoi(inst[i+1]);
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="BASE"){
                        if(inst.size()==(i+2)) curr_format = 0;
                        else{
                            cout<<"error at line "<<iterator+1<<": INVALID INSTRUCTION!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i]=="ORG"){
                        if(inst.size()!=2){
                            if(org_address==-1){
                                cout<<"error at line "<<iterator+1<<" : ORG statement incomplete!\n";
                                returnInt=0;
                            }
                            else{
                                curr_format = org_address - curr_addr;
                            }
                        }
                    }
                    else if(inst[i]=="LTORG"){
                        if(inst.size()!=1){
                            cout<<"error at line "<<iterator+1<<" : Invalid LTORG statement!\n";
                            returnInt=0;
                        }
                        else{
                            counter_literal = 0;
                            for(int i=0;i<litTab.size();i++){
                                vector<string> vec;
                                if(litTab[i].written==0){
                                    vec.push_back("*");
                                    vec.push_back('='+litTab[i].name);
                                    litTab[i].written = 1;
                                }
                                instructions.insert(instructions.begin()+iterator+1+counter_literal, vec);
                                counter_literal++;
                            }
                        }
                    }
                }
                else{
                    cout<<"error at line "<<iterator+1<<"!\n";
                    returnInt=0;
                }
            }
            if(i==0 and flag==0){
                if(inst[i]!="*"){
                    if(findinvec(extRef, inst[i])!=-1){
                        cout<<"error: external reference symbol defined here!\n";
                        returnInt=0;
                    }
                    else{
                        Symbol label(inst[i],curr_addr);
                        if(findSymbol(inst[i])==-1 or symTab[findSymbol(inst[i])].addr==-1){
                            if(findSymbol(inst[i])==-1){
                                symTab.push_back(label);
                            }
                            else{
                                symTab[findSymbol(inst[i])].addr = curr_addr;
                            }    
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<": Symbol "<<inst[i]<<" has multiple instances!\n";
                            returnInt=0;
                        }
                    }
                }
            }
            if(i==1 and flag==0){
                if(inst[i-1]=="*"){
                    curr_format = litTab[findLiteral(inst[i].substr(1))].size;
                    litTab[findLiteral(inst[i].substr(1))].addr = curr_addr;
                }
                else{
                    cout<<"error at line "<<iterator+1<<": No OPCODE or ASSEMBLER DIRECTIVE FOUND!\n";
                    returnInt=0;
                }
            }

        }
        locctr.push_back(curr_addr);
        curr_addr += curr_format;
    }
    for(int i=0;i<symTab.size();i++){
        if(containsArithmetic(symTab[i].name)){
            if(symTab[i].addr == -1){
                if(isExpression(symTab[i].name)!=INT_MAX){
                    symTab[i].addr = isExpression(symTab[i].name);
                    symTab[i].relative = absOrRelative(symTab[i].name);
                    symTab[i].isSym = 1;
                }
                else{
                    cout<<"error: Expression "<<symTab[i].name<<" is not a valid expression!\n";
                    returnInt=0;
                }
            }
            else{
                cout<<"error: BT HO GAYI! "<<symTab[i].name<<" HAGG RAHA HAI!\n";            //HATANA HAI!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                returnInt=0;
            }
        }
    }
    for(int i=0;i<symTab.size();i++){
        if(symTab[i].parent != ""){
            symTab[i].addr = symTab[findSymbol(symTab[i].parent)].addr;
            symTab[i].relative = symTab[findSymbol(symTab[i].parent)].relative;
        }
        if(symTab[i].addr==-1){
            cout<<"error: Label "<<symTab[i].name<<" is accessed without definition!\n";
            returnInt=0;
        }
    }
    for(int i=0;i<litTab.size();i++){
        vector<string> vec;
        if(litTab[i].written==0){
            vec.push_back("*");
            vec.push_back('='+litTab[i].name);
            litTab[i].written = 1;
            instructions.insert(instructions.begin()+instructions.size()-1, vec);
            litTab[i].addr = curr_addr;
            locctr.push_back(curr_addr);
            curr_addr = curr_addr + litTab[i].size;
        }
    }
    locctr.push_back(curr_addr);
    programLength = inttohex(curr_addr - locctr[0]);

    return returnInt;
}

int secondPass(){
    int returnInt=1;
    int pc = locctr[2];
    bool basedefined = 0;
    int base = 0;
    objCode.push_back("");
    for(int iterator=1;iterator<instructions.size()-1;iterator++){
        bool flag = 0;
        bool form4 = 0;
        bool assDir = 0;
        bool byteorword = 0;    
        string objcode = "";
        string opcode = "";
        string assem_operand = "";
        string ascii_operand = "";
        int opcodeformat = 0;
        string templabel = "";
        int displacement=0;
        string regnum = "";
        int N = 1;
        int I = 1;
        int X = 0;
        int B = 0;
        int P = 0;
        int E = 0;
        vector<string> inst = instructions[iterator];
        for(int i=0;i<inst.size();i++){
            if(inst[i][0] == '+'){
                form4 = 1;
            }
            if(inst[0]=="*"){
                flag=1;
            }
            if((opCode.find(inst[i]) != opCode.end())){
                //OPCODE MATCHED LDA
                flag=1;
                opcodeformat = opCode[inst[i]].first;
                opcode = opCode[inst[i]].second;
                E = 0;//FORMAT 3
            }
            else if(form4 and (opCode.find(inst[i].substr(1)) != opCode.end())){
                //OPCODE MATCHED +LDA
                flag=1;
                opcodeformat = opCode[inst[i].substr(1)].first;
                opcode = opCode[inst[i].substr(1)].second;
                E = 1;
                P = 0;
                B = 0;
                X = 0;
            }
            else if(findinvec(assmDir,inst[i])!=-1){
                flag=1;
                assDir = 1;
                if(inst[i] == "WORD" or inst[i] == "BYTE"){
                    byteorword = 1;
                    if(findSymbol(inst[i+1])!=-1){
                        if(symTab[findSymbol(inst[i+1])].relative==1){
                            modification.push_back(locctr[iterator]);
                            modificationSize.push_back(6);
                            modificationName.push_back("");
                        }
                    }
                }
                else if(inst[i]=="BASE"){
                    basedefined = 1;
                    if(inst.size()<=i+1){
                        cout<<"error at line "<<iterator+1<<" : No label after BASE Assembler Directive!\n";
                        returnInt=0;
                    }
                    base = symTab[findSymbol(inst[i+1])].addr;
                }
                else if(inst[i] == "NOBASE"){
                    basedefined = 0;
                }
            }
            else if(inst[i][0]=='#' or inst[i][0]=='@'){
                if(inst[i][0] == '#'){
                    N = 0;
                }
                else{
                    I = 0;
                }
                templabel = inst[i].substr(1);
                if(flag and (findSymbol(inst[i].substr(1))!=-1)){
                    // #LABEL or @LABEL
                    if(E==1){
                        if(symTab[findSymbol(inst[i].substr(1))].relative==1){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5); 
                            modificationName.push_back("");
                        }
                        displacement = symTab[findSymbol(templabel)].addr;
                        if(displacement >= 1048576){
                            cout<<"error at line "<<iterator+1<<" : Value is too big to represent in 20 bits!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i][0]=='@' and inst[i-1]!="EQU"){
                        displacement = symTab[findSymbol(templabel)].addr - pc;
                        if(abs(displacement)>=2048){
                            if(basedefined){
                                B = 1;
                                displacement = symTab[findSymbol(templabel)].addr - base;
                                if(displacement<0 or displacement>=4096){
                                    cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing1!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            P = 1;
                        }
                    }
                    else if(inst[i][0]=='#' and inst[i-1]!="EQU"){
                        if(symTab[findSymbol(templabel)].isSym==1){
                            displacement = symTab[findSymbol(templabel)].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(3);
                            modificationName.push_back("");
                            if(displacement >= 4096){
                                cout<<"error at line "<<iterator+1<<" : Value of immediate can't be expressed in 12 bits!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            displacement = symTab[findSymbol(templabel)].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = symTab[findSymbol(templabel)].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing2!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }    
                        }
                    }
                }
                else{
                    // #1000 or @1000
                    displacement = stoi(templabel);
                }    
            }
            else if(flag and findSymbol(inst[i])!=-1){
                // LABEL
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(symTab[findSymbol(inst[i])].addr,6);
                }
                else if(E==0 and inst[i-1]!="EQU"){
                    displacement = symTab[findSymbol(inst[i])].addr - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = symTab[findSymbol(inst[i])].addr - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n"<<inst[0]<<" "<<inst[1]<<"\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(E==1){
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("");
                    displacement = symTab[findSymbol(inst[i])].addr;
                    P = 0;
                    B = 0;
                    X = 0;
                }
            }
            else if(flag){
                //1000 or C'EOF' or X'F1' or X,A,...
                if(findLiteral(inst[i].substr(1))!=-1){
                    if(inst[i-1]=="*"){
                        if(inst[i][1]=='C' and inst[i][2]=='\''){
                            for(int iter=3; iter<inst[i].size()-1;iter++){
                                objcode = objcode + inttohex_param(int(inst[i][iter]),2);
                            }
                        }
                        else{
                            objcode = inst[i].substr(3,inst[i].size()-4);
                        }
                    }
                    else{
                        if(E==0){
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = litTab[findLiteral(inst[i].substr(1))].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<litTab[findLiteral(inst[i].substr(1))].addr<<" "<<base<<" "<<displacement<<"\n";
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }
                        }
                        else{
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back("");
                        }
                    }
                }
                else if(isNum(inst[i]) and !assDir){
                    displacement = stoi(inst[i]) - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = stoi(inst[i]) - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(isNum(inst[i]) and assDir){
                    assem_operand = inttohex_param(stoi(inst[i]),6);
                }
                else if(findinvec(registers, inst[i])!=-1){
                    if(opcodeformat==2){
                        regnum = regnum + to_string(findinvec(registers,inst[i]));
                    }
                    else if((form4==1 and opcodeformat==4) or (opcodeformat == 1)){
                        if(form4==1 and opcodeformat==4){
                            if(inst[i]=="X"){
                                X=1;
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                                returnInt=0;        
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                    }
                    else if((opcodeformat==4 and form4==0)){
                        if(inst[i]!="X"){
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                        else{
                            X = 1;
                        }
                    }                      
                }
                else if((inst[i][0]=='C' or inst[i][0]=='X') and (inst[i][1]=='\'')){
                    assem_operand = inst[i].substr(2,inst[i].size()-3);
                    if(inst[i][0]=='C'){
                        for(int j=0;j<assem_operand.size();j++){
                            ascii_operand = ascii_operand + inttohex_param(int(assem_operand[j]),2);
                        }
                        assem_operand = ascii_operand;
                    }
                }
            }
        }
        //WRITE THE OBJECT CODE
        // objcode = "";
        if(assDir){
            if(byteorword){
                objcode = assem_operand;
            }
        }
        else if(opcodeformat==1){
            objcode = opcode;
        }
        else if(opcodeformat==2){
            objcode = opcode + regnum;
            while(objcode.size()<4){
                objcode = objcode + '0';
            }
        }
        else if(opcodeformat==4){
            if(E==0){
                //FORMAT 3
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X + 4*B + 2*P + E) + inttohex_param(displacement,3);
            }
            else{
                //FORMAT 4
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X+E) + inttohex_param(displacement,5);
            }
        }
        objCode.push_back(objcode);
        pc = locctr[2+iterator];
    }
    objCode.push_back("");
    return returnInt;
}

int secondPass1(vector<vector<string>> &instructions){
    int returnInt=1;
    int kk=2;
    if(extDef.size()>0) kk++;
    if(extRef.size()>0) kk++;
    int pc = locctr[kk];
    bool basedefined = 0;
    int base = 0;
    objCode.push_back("");
    // if(extDef.size()>0) objCode.push_back("");
    // if(extRef.size()>0) objCode.push_back("");
    for(int iterator=1;iterator<instructions.size();iterator++){
        bool flag = 0;
        bool form4 = 0;
        bool assDir = 0;
        bool byteorword = 0;    
        string objcode = "";
        string opcode = "";
        string assem_operand = "";
        string ascii_operand = "";
        int opcodeformat = 0;
        string templabel = "";
        int displacement=0;
        string regnum = "";
        int N = 1;
        int I = 1;
        int X = 0;
        int B = 0;
        int P = 0;
        int E = 0;
        vector<string> inst = instructions[iterator];
        for(int i=0;i<inst.size();i++){
            if(inst[i][0] == '+'){
                form4 = 1;
            }
            if(inst[0]=="*"){
                flag=1;
            }
            if((opCode.find(inst[i]) != opCode.end())){
                //OPCODE MATCHED LDA
                flag=1;
                opcodeformat = opCode[inst[i]].first;
                opcode = opCode[inst[i]].second;
                E = 0;//FORMAT 3
            }
            else if(form4 and (opCode.find(inst[i].substr(1)) != opCode.end())){
                //OPCODE MATCHED +LDA
                flag=1;
                opcodeformat = opCode[inst[i].substr(1)].first;
                opcode = opCode[inst[i].substr(1)].second;
                E = 1;
                P = 0;
                B = 0;
                X = 0;
            }
            else if(findinvec(assmDir,inst[i])!=-1){
                flag=1;
                assDir = 1;
                if(inst[i] == "WORD" or inst[i] == "BYTE"){
                    byteorword = 1;
                    if(findinvec(extRef, inst[i+1])!=-1){
                        modification.push_back(locctr[iterator]);
                        modificationSize.push_back(6);
                        modificationName.push_back("+"+inst[i+1]);
                    }
                    else if(findSymbol(inst[i+1])!=-1){
                        if(symTab[findSymbol(inst[i+1])].relative==1){
                            modification.push_back(locctr[iterator]);
                            modificationSize.push_back(6);
                            modificationName.push_back("");
                        }
                    }
                    else{
                        vector<string> vv=breakRExp(inst[i+1]);
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]);
                            modificationSize.push_back(6);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
                else if(inst[i]=="BASE"){
                    basedefined = 1;
                    if(inst.size()<=i+1){
                        cout<<"error at line "<<iterator+1<<" : No label after BASE Assembler Directive!\n";
                        returnInt=0;
                    }
                    base = symTab[findSymbol(inst[i+1])].addr;
                }
                else if(inst[i] == "NOBASE"){
                    basedefined = 0;
                }
            }
            else if(inst[i][0]=='#' or inst[i][0]=='@'){
                if(inst[i][0] == '#'){
                    N = 0;
                }
                else{
                    I = 0;
                }
                templabel = inst[i].substr(1);
                if(flag and findinvec(extRef, templabel)!=-1){
                    displacement=0;
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("+"+templabel);
                }
                else if(flag and (findSymbol(inst[i].substr(1))==-1) and !isNum(templabel) and inst[i-1][0]=='+'){
                    vector<string> vv=breakRExp(templabel);
                    if(vv[0]!=""){
                        displacement=symTab[findSymbol(vv[0])].addr;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                    else{
                        displacement=0;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
                else if(flag and (findSymbol(inst[i].substr(1))!=-1)){
                    // #LABEL or @LABEL
                    if(E==1){
                        if(symTab[findSymbol(inst[i].substr(1))].relative==1){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5); 
                            modificationName.push_back("");
                        }
                        displacement = symTab[findSymbol(templabel)].addr;
                        if(displacement >= 1048576){
                            cout<<"error at line "<<iterator+1<<" : Value is too big to represent in 20 bits!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i][0]=='@' and inst[i-1]!="EQU"){
                        displacement = symTab[findSymbol(templabel)].addr - pc;
                        if(abs(displacement)>=2048){
                            if(basedefined){
                                B = 1;
                                displacement = symTab[findSymbol(templabel)].addr - base;
                                if(displacement<0 or displacement>=4096){
                                    cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing1!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            P = 1;
                        }
                    }
                    else if(inst[i][0]=='#' and inst[i-1]!="EQU"){
                        if(symTab[findSymbol(templabel)].isSym==1){
                            displacement = symTab[findSymbol(templabel)].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(3);
                            modificationName.push_back("");
                            if(displacement >= 4096){
                                cout<<"error at line "<<iterator+1<<" : Value of immediate can't be expressed in 12 bits!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            displacement = symTab[findSymbol(templabel)].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = symTab[findSymbol(templabel)].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing2!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }    
                        }
                    }
                }
                else{
                    // #1000 or @1000
                    displacement = stoi(templabel);
                }    
            }
            else if(flag and findinvec(extRef, inst[i])!=-1){
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(0, 6);
                }
                else{
                    displacement=0;
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("+"+inst[i]);
                }
            }
            else if(flag and findSymbol(inst[i])==-1 and i>0 and !isNum(inst[i]) and findLiteral(inst[i].substr(1))==-1 and (inst[i-1]=="WORD" or inst[i-1][0]=='+')){
                vector<string> vv=breakRExp(inst[i]);
                if(inst[i-1]=="WORD"){
                    if(vv[0]!=""){
                        assem_operand=inttohex_param(symTab[findSymbol(vv[0])].addr, 6);
                    }
                     else assem_operand = inttohex_param(0, 6);
                }
                else{
                    if(vv[0]!=""){
                        displacement=symTab[findSymbol(vv[0])].addr;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                    else{
                        displacement=0;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
            }
            else if(flag and findSymbol(inst[i])!=-1){
                // LABEL
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(symTab[findSymbol(inst[i])].addr,6);
                }
                else if(E==0 and inst[i-1]!="EQU"){
                    displacement = symTab[findSymbol(inst[i])].addr - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = symTab[findSymbol(inst[i])].addr - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n"<<inst[0]<<" "<<inst[1]<<"\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(E==1){
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("");
                    displacement = symTab[findSymbol(inst[i])].addr;
                    P = 0;
                    B = 0;
                    X = 0;
                }
            }
            else if(flag){
                //1000 or C'EOF' or X'F1' or X,A,...
                if(findLiteral(inst[i].substr(1))!=-1){
                    if(inst[i-1]=="*"){
                        if(inst[i][1]=='C' and inst[i][2]=='\''){
                            for(int iter=3; iter<inst[i].size()-1;iter++){
                                objcode = objcode + inttohex_param(int(inst[i][iter]),2);
                            }
                        }
                        else{
                            objcode = inst[i].substr(3,inst[i].size()-4);
                        }
                    }
                    else{
                        if(E==0){
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = litTab[findLiteral(inst[i].substr(1))].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<litTab[findLiteral(inst[i].substr(1))].addr<<" "<<base<<" "<<displacement<<"\n";
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }
                        }
                        else{
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back("");
                        }
                    }
                }
                else if(isNum(inst[i]) and !assDir){
                    displacement = stoi(inst[i]) - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = stoi(inst[i]) - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(isNum(inst[i]) and assDir){
                    assem_operand = inttohex_param(stoi(inst[i]),6);
                }
                else if(findinvec(registers, inst[i])!=-1){
                    if(opcodeformat==2){
                        regnum = regnum + to_string(findinvec(registers,inst[i]));
                    }
                    else if((form4==1 and opcodeformat==4) or (opcodeformat == 1)){
                        if(form4==1 and opcodeformat==4){
                            if(inst[i]=="X"){
                                X=1;
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                    }
                    else if((opcodeformat==4 and form4==0)){
                        if(inst[i]!="X"){
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                        else{
                            X = 1;
                        }
                    }                      
                }
                else if((inst[i][0]=='C' or inst[i][0]=='X') and (inst[i][1]=='\'')){
                    assem_operand = inst[i].substr(2,inst[i].size()-3);
                    if(inst[i][0]=='C'){
                        for(int j=0;j<assem_operand.size();j++){
                            ascii_operand = ascii_operand + inttohex_param(int(assem_operand[j]),2);
                        }
                        assem_operand = ascii_operand;
                    }
                }
            }
        }
        //WRITE THE OBJECT CODE
        // objcode = "";
        if(assDir){
            if(byteorword){
                objcode = assem_operand;
            }
        }
        else if(opcodeformat==1){
            objcode = opcode;
        }
        else if(opcodeformat==2){
            objcode = opcode + regnum;
            while(objcode.size()<4){
                objcode = objcode + '0';
            }
        }
        else if(opcodeformat==4){
            if(E==0){
                //FORMAT 3
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X + 4*B + 2*P + E) + inttohex_param(displacement,3);
            }
            else{
                //FORMAT 4
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X+E) + inttohex_param(displacement,5);
            }
        }
        objCode.push_back(objcode);
        pc = locctr[2+iterator];
    }
    // objCode.push_back("");
    return returnInt;
}

int secondPass2(vector<vector<string>> &instructions){
    int returnInt=1;
    int kk=2;
    if(extDef.size()>0) kk++;
    if(extRef.size()>0) kk++;
    int pc = locctr[kk];
    bool basedefined = 0;
    int base = 0;
    objCode.push_back("");
    // if(extDef.size()>0) objCode.push_back("");
    // if(extRef.size()>0) objCode.push_back("");
    for(int iterator=1;iterator<instructions.size();iterator++){
        bool flag = 0;
        bool form4 = 0;
        bool assDir = 0;
        bool byteorword = 0;    
        string objcode = "";
        string opcode = "";
        string assem_operand = "";
        string ascii_operand = "";
        int opcodeformat = 0;
        string templabel = "";
        int displacement=0;
        string regnum = "";
        int N = 1;
        int I = 1;
        int X = 0;
        int B = 0;
        int P = 0;
        int E = 0;
        vector<string> inst = instructions[iterator];
        for(int i=0;i<inst.size();i++){
            if(inst[i][0] == '+'){
                form4 = 1;
            }
            if(inst[0]=="*"){
                flag=1;
            }
            if((opCode.find(inst[i]) != opCode.end())){
                //OPCODE MATCHED LDA
                flag=1;
                opcodeformat = opCode[inst[i]].first;
                opcode = opCode[inst[i]].second;
                E = 0;//FORMAT 3
            }
            else if(form4 and (opCode.find(inst[i].substr(1)) != opCode.end())){
                //OPCODE MATCHED +LDA
                flag=1;
                opcodeformat = opCode[inst[i].substr(1)].first;
                opcode = opCode[inst[i].substr(1)].second;
                E = 1;
                P = 0;
                B = 0;
                X = 0;
            }
            else if(findinvec(assmDir,inst[i])!=-1){
                flag=1;
                assDir = 1;
                if(inst[i] == "WORD" or inst[i] == "BYTE"){
                    byteorword = 1;
                    if(findinvec(extRef, inst[i+1])!=-1){
                        modification.push_back(locctr[iterator]);
                        modificationSize.push_back(6);
                        modificationName.push_back("+"+inst[i+1]);
                    }
                    else if(findSymbol(inst[i+1])!=-1){
                        if(symTab[findSymbol(inst[i+1])].relative==1){
                            modification.push_back(locctr[iterator]);
                            modificationSize.push_back(6);
                            modificationName.push_back("");
                        }
                    }
                    else{
                        vector<string> vv=breakRExp(inst[i+1]);
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]);
                            modificationSize.push_back(6);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
                else if(inst[i]=="BASE"){
                    basedefined = 1;
                    if(inst.size()<=i+1){
                        cout<<"error at line "<<iterator+1<<" : No label after BASE Assembler Directive!\n";
                        returnInt=0;
                    }
                    base = symTab[findSymbol(inst[i+1])].addr;
                }
                else if(inst[i] == "NOBASE"){
                    basedefined = 0;
                }
            }
            else if(inst[i][0]=='#' or inst[i][0]=='@'){
                if(inst[i][0] == '#'){
                    N = 0;
                }
                else{
                    I = 0;
                }
                templabel = inst[i].substr(1);
                if(flag and findinvec(extRef, templabel)!=-1){
                    displacement=0;
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("+"+templabel);
                }
                else if(flag and (findSymbol(inst[i].substr(1))==-1) and !isNum(inst[i]) and inst[i-1][0]=='+'){
                    vector<string> vv=breakRExp(templabel);
                    if(vv[0]!=""){
                        displacement=symTab[findSymbol(vv[0])].addr;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                    else{
                        displacement=0;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
                else if(flag and (findSymbol(inst[i].substr(1))!=-1)){
                    // #LABEL or @LABEL
                    if(E==1){
                        if(symTab[findSymbol(inst[i].substr(1))].relative==1){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5); 
                            modificationName.push_back("");
                        }
                        displacement = symTab[findSymbol(templabel)].addr;
                        if(displacement >= 1048576){
                            cout<<"error at line "<<iterator+1<<" : Value is too big to represent in 20 bits!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i][0]=='@' and inst[i-1]!="EQU"){
                        displacement = symTab[findSymbol(templabel)].addr - pc;
                        if(abs(displacement)>=2048){
                            if(basedefined){
                                B = 1;
                                displacement = symTab[findSymbol(templabel)].addr - base;
                                if(displacement<0 or displacement>=4096){
                                    cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing1!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            P = 1;
                        }
                    }
                    else if(inst[i][0]=='#' and inst[i-1]!="EQU"){
                        if(symTab[findSymbol(templabel)].isSym==1){
                            displacement = symTab[findSymbol(templabel)].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(3);
                            modificationName.push_back("");
                            if(displacement >= 4096){
                                cout<<"error at line "<<iterator+1<<" : Value of immediate can't be expressed in 12 bits!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            displacement = symTab[findSymbol(templabel)].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = symTab[findSymbol(templabel)].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing2!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }    
                        }
                    }
                }
                else{
                    // #1000 or @1000
                    displacement = stoi(templabel);
                }    
            }
            else if(flag and findinvec(extRef, inst[i])!=-1){
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(0, 6);
                }
                else{
                    displacement=0;
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("+"+inst[i]);
                }
            }
            else if(flag and findSymbol(inst[i])==-1 and i>0 and !isNum(inst[i]) and findLiteral(inst[i].substr(1))==-1 and (inst[i-1]=="WORD" or inst[i-1][0]=='+')){
                vector<string> vv=breakRExp(inst[i]);
                if(inst[i-1]=="WORD"){
                    if(vv[0]!=""){
                        assem_operand=inttohex_param(symTab[findSymbol(vv[0])].addr, 6);
                    }
                     else assem_operand = inttohex_param(0, 6);
                }
                else{
                    if(vv[0]!=""){
                        displacement=symTab[findSymbol(vv[0])].addr;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                    else{
                        displacement=0;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
            }
            else if(flag and findSymbol(inst[i])!=-1){
                // LABEL
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(symTab[findSymbol(inst[i])].addr,6);
                }
                else if(E==0 and inst[i-1]!="EQU"){
                    displacement = symTab[findSymbol(inst[i])].addr - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = symTab[findSymbol(inst[i])].addr - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n"<<inst[0]<<" "<<inst[1]<<"\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(E==1){
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("");
                    displacement = symTab[findSymbol(inst[i])].addr;
                    P = 0;
                    B = 0;
                    X = 0;
                }
            }
            else if(flag){
                //1000 or C'EOF' or X'F1' or X,A,...
                if(findLiteral(inst[i].substr(1))!=-1){
                    if(inst[i-1]=="*"){
                        if(inst[i][1]=='C' and inst[i][2]=='\''){
                            for(int iter=3; iter<inst[i].size()-1;iter++){
                                objcode = objcode + inttohex_param(int(inst[i][iter]),2);
                            }
                        }
                        else{
                            objcode = inst[i].substr(3,inst[i].size()-4);
                        }
                    }
                    else{
                        if(E==0){
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = litTab[findLiteral(inst[i].substr(1))].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<litTab[findLiteral(inst[i].substr(1))].addr<<" "<<base<<" "<<displacement<<"\n";
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }
                        }
                        else{
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back("");
                        }
                    }
                }
                else if(isNum(inst[i]) and !assDir){
                    displacement = stoi(inst[i]) - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = stoi(inst[i]) - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(isNum(inst[i]) and assDir){
                    assem_operand = inttohex_param(stoi(inst[i]),6);
                }
                else if(findinvec(registers, inst[i])!=-1){
                    if(opcodeformat==2){
                        regnum = regnum + to_string(findinvec(registers,inst[i]));
                    }
                    else if((form4==1 and opcodeformat==4) or (opcodeformat == 1)){
                        if(form4==1 and opcodeformat==4){
                            if(inst[i]=="X"){
                                X=1;
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                    }
                    else if((opcodeformat==4 and form4==0)){
                        if(inst[i]!="X"){
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                        else{
                            X = 1;
                        }
                    }                      
                }
                else if((inst[i][0]=='C' or inst[i][0]=='X') and (inst[i][1]=='\'')){
                    assem_operand = inst[i].substr(2,inst[i].size()-3);
                    if(inst[i][0]=='C'){
                        for(int j=0;j<assem_operand.size();j++){
                            ascii_operand = ascii_operand + inttohex_param(int(assem_operand[j]),2);
                        }
                        assem_operand = ascii_operand;
                    }
                }
            }
        }
        //WRITE THE OBJECT CODE
        // objcode = "";
        if(assDir){
            if(byteorword){
                objcode = assem_operand;
            }
        }
        else if(opcodeformat==1){
            objcode = opcode;
        }
        else if(opcodeformat==2){
            objcode = opcode + regnum;
            while(objcode.size()<4){
                objcode = objcode + '0';
            }
        }
        else if(opcodeformat==4){
            if(E==0){
                //FORMAT 3
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X + 4*B + 2*P + E) + inttohex_param(displacement,3);
            }
            else{
                //FORMAT 4
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X+E) + inttohex_param(displacement,5);
            }
        }
        objCode.push_back(objcode);
        pc = locctr[2+iterator];
    }
    // objCode.push_back("");
    return returnInt;
}

int secondPass3(vector<vector<string>> &instructions){
    int returnInt=1;
    int kk=2;
    if(extDef.size()>0) kk++;
    if(extRef.size()>0) kk++;
    int pc = locctr[kk];
    bool basedefined = 0;
    int base = 0;
    objCode.push_back("");
    // if(extDef.size()>0) objCode.push_back("");
    // if(extRef.size()>0) objCode.push_back("");
    for(int iterator=1;iterator<instructions.size();iterator++){
        bool flag = 0;
        bool form4 = 0;
        bool assDir = 0;
        bool byteorword = 0;    
        string objcode = "";
        string opcode = "";
        string assem_operand = "";
        string ascii_operand = "";
        int opcodeformat = 0;
        string templabel = "";
        int displacement=0;
        string regnum = "";
        int N = 1;
        int I = 1;
        int X = 0;
        int B = 0;
        int P = 0;
        int E = 0;
        vector<string> inst = instructions[iterator];
        for(int i=0;i<inst.size();i++){
            if(inst[i][0] == '+'){
                form4 = 1;
            }
            if(inst[0]=="*"){
                flag=1;
            }
            if((opCode.find(inst[i]) != opCode.end())){
                //OPCODE MATCHED LDA
                flag=1;
                opcodeformat = opCode[inst[i]].first;
                opcode = opCode[inst[i]].second;
                E = 0;//FORMAT 3
            }
            else if(form4 and (opCode.find(inst[i].substr(1)) != opCode.end())){
                //OPCODE MATCHED +LDA
                flag=1;
                opcodeformat = opCode[inst[i].substr(1)].first;
                opcode = opCode[inst[i].substr(1)].second;
                E = 1;
                P = 0;
                B = 0;
                X = 0;
            }
            else if(findinvec(assmDir,inst[i])!=-1){
                flag=1;
                assDir = 1;
                if(inst[i] == "WORD" or inst[i] == "BYTE"){
                    byteorword = 1;
                    if(findinvec(extRef, inst[i+1])!=-1){
                        modification.push_back(locctr[iterator]);
                        modificationSize.push_back(6);
                        modificationName.push_back("+"+inst[i+1]);
                    }
                    else if(findSymbol(inst[i+1])!=-1){
                        if(symTab[findSymbol(inst[i+1])].relative==1){
                            modification.push_back(locctr[iterator]);
                            modificationSize.push_back(6);
                            modificationName.push_back("");
                        }
                    }
                    else{
                        vector<string> vv=breakRExp(inst[i+1]);
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]);
                            modificationSize.push_back(6);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
                else if(inst[i]=="BASE"){
                    basedefined = 1;
                    if(inst.size()<=i+1){
                        cout<<"error at line "<<iterator+1<<" : No label after BASE Assembler Directive!\n";
                        returnInt=0;
                    }
                    base = symTab[findSymbol(inst[i+1])].addr;
                }
                else if(inst[i] == "NOBASE"){
                    basedefined = 0;
                }
            }
            else if(inst[i][0]=='#' or inst[i][0]=='@'){
                if(inst[i][0] == '#'){
                    N = 0;
                }
                else{
                    I = 0;
                }
                templabel = inst[i].substr(1);
                if(flag and findinvec(extRef, templabel)!=-1){
                    displacement=0;
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("+"+templabel);
                }
                else if(flag and (findSymbol(inst[i].substr(1))==-1) and !isNum(inst[i]) and inst[i-1][0]=='+'){
                    vector<string> vv=breakRExp(templabel);
                    if(vv[0]!=""){
                        displacement=symTab[findSymbol(vv[0])].addr;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                    else{
                        displacement=0;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
                else if(flag and (findSymbol(inst[i].substr(1))!=-1)){
                    // #LABEL or @LABEL
                    if(E==1){
                        if(symTab[findSymbol(inst[i].substr(1))].relative==1){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5); 
                            modificationName.push_back("");
                        }
                        displacement = symTab[findSymbol(templabel)].addr;
                        if(displacement >= 1048576){
                            cout<<"error at line "<<iterator+1<<" : Value is too big to represent in 20 bits!\n";
                            returnInt=0;
                        }
                    }
                    else if(inst[i][0]=='@' and inst[i-1]!="EQU"){
                        displacement = symTab[findSymbol(templabel)].addr - pc;
                        if(abs(displacement)>=2048){
                            if(basedefined){
                                B = 1;
                                displacement = symTab[findSymbol(templabel)].addr - base;
                                if(displacement<0 or displacement>=4096){
                                    cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing1!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            P = 1;
                        }
                    }
                    else if(inst[i][0]=='#' and inst[i-1]!="EQU"){
                        if(symTab[findSymbol(templabel)].isSym==1){
                            displacement = symTab[findSymbol(templabel)].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(3);
                            modificationName.push_back("");
                            if(displacement >= 4096){
                                cout<<"error at line "<<iterator+1<<" : Value of immediate can't be expressed in 12 bits!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            displacement = symTab[findSymbol(templabel)].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = symTab[findSymbol(templabel)].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing2!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }    
                        }
                    }
                }
                else{
                    // #1000 or @1000
                    displacement = stoi(templabel);
                }    
            }
            else if(flag and findinvec(extRef, inst[i])!=-1){
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(0, 6);
                }
                else{
                    displacement=0;
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("+"+inst[i]);
                }
            }
            else if(flag and findSymbol(inst[i])==-1 and i>0 and !isNum(inst[i]) and findLiteral(inst[i].substr(1))==-1 and (inst[i-1]=="WORD" or inst[i-1][0]=='+')){
                vector<string> vv=breakRExp(inst[i]);
                if(inst[i-1]=="WORD"){
                    if(vv[0]!=""){
                        assem_operand=inttohex_param(symTab[findSymbol(vv[0])].addr, 6);
                    }
                     else assem_operand = inttohex_param(0, 6);
                }
                else{
                    if(vv[0]!=""){
                        displacement=symTab[findSymbol(vv[0])].addr;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                    else{
                        displacement=0;
                        for(int qq=1;qq<vv.size();qq++){
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back(vv[qq]);
                        }
                    }
                }
            }
            else if(flag and findSymbol(inst[i])!=-1){
                // LABEL
                if(inst[i-1]=="WORD"){
                    assem_operand = inttohex_param(symTab[findSymbol(inst[i])].addr,6);
                }
                else if(E==0 and inst[i-1]!="EQU"){
                    displacement = symTab[findSymbol(inst[i])].addr - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = symTab[findSymbol(inst[i])].addr - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n"<<inst[0]<<" "<<inst[1]<<"\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(E==1){
                    modification.push_back(locctr[iterator]+1);
                    modificationSize.push_back(5);
                    modificationName.push_back("");
                    displacement = symTab[findSymbol(inst[i])].addr;
                    P = 0;
                    B = 0;
                    X = 0;
                }
            }
            else if(flag){
                //1000 or C'EOF' or X'F1' or X,A,...
                if(findLiteral(inst[i].substr(1))!=-1){
                    if(inst[i-1]=="*"){
                        if(inst[i][1]=='C' and inst[i][2]=='\''){
                            for(int iter=3; iter<inst[i].size()-1;iter++){
                                objcode = objcode + inttohex_param(int(inst[i][iter]),2);
                            }
                        }
                        else{
                            objcode = inst[i].substr(3,inst[i].size()-4);
                        }
                    }
                    else{
                        if(E==0){
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr - pc;
                            if(abs(displacement)>=2048){
                                if(basedefined){
                                    B = 1;
                                    displacement = litTab[findLiteral(inst[i].substr(1))].addr - base;
                                    if(displacement<0 or displacement>=4096){
                                        cout<<litTab[findLiteral(inst[i].substr(1))].addr<<" "<<base<<" "<<displacement<<"\n";
                                        cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                        returnInt=0;
                                    }
                                }
                                else{
                                    cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                                    returnInt=0;
                                }
                            }
                            else{
                                P = 1;
                            }
                        }
                        else{
                            displacement = litTab[findLiteral(inst[i].substr(1))].addr;
                            modification.push_back(locctr[iterator]+1);
                            modificationSize.push_back(5);
                            modificationName.push_back("");
                        }
                    }
                }
                else if(isNum(inst[i]) and !assDir){
                    displacement = stoi(inst[i]) - pc;
                    if(abs(displacement)>=2048){
                        if(basedefined){
                            B = 1;
                            displacement = stoi(inst[i]) - base;
                            if(displacement<0 or displacement>=4096){
                                cout<<"error at line "<<iterator+1<<" : Can use neither PC nor BASE Relative addressing!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Require base addressing but base undefined!\n";
                            returnInt=0;
                        }
                    }
                    else{
                        P = 1;
                    }
                }
                else if(isNum(inst[i]) and assDir){
                    assem_operand = inttohex_param(stoi(inst[i]),6);
                }
                else if(findinvec(registers, inst[i])!=-1){
                    if(opcodeformat==2){
                        regnum = regnum + to_string(findinvec(registers,inst[i]));
                    }
                    else if((form4==1 and opcodeformat==4) or (opcodeformat == 1)){
                        if(form4==1 and opcodeformat==4){
                            if(inst[i]=="X"){
                                X=1;
                            }
                            else{
                                cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                                returnInt=0;
                            }
                        }
                        else{
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                    }
                    else if((opcodeformat==4 and form4==0)){
                        if(inst[i]!="X"){
                            cout<<"error at line "<<iterator+1<<" : Invalid use of register!\n";
                            returnInt=0;
                        }
                        else{
                            X = 1;
                        }
                    }                      
                }
                else if((inst[i][0]=='C' or inst[i][0]=='X') and (inst[i][1]=='\'')){
                    assem_operand = inst[i].substr(2,inst[i].size()-3);
                    if(inst[i][0]=='C'){
                        for(int j=0;j<assem_operand.size();j++){
                            ascii_operand = ascii_operand + inttohex_param(int(assem_operand[j]),2);
                        }
                        assem_operand = ascii_operand;
                    }
                }
            }
        }
        //WRITE THE OBJECT CODE
        // objcode = "";
        if(assDir){
            if(byteorword){
                objcode = assem_operand;
            }
        }
        else if(opcodeformat==1){
            objcode = opcode;
        }
        else if(opcodeformat==2){
            objcode = opcode + regnum;
            while(objcode.size()<4){
                objcode = objcode + '0';
            }
        }
        else if(opcodeformat==4){
            if(E==0){
                //FORMAT 3
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X + 4*B + 2*P + E) + inttohex_param(displacement,3);
            }
            else{
                //FORMAT 4
                objcode = inttohex_param(hextoint(opcode)+2*N+I,2) + inttohex(8*X+E) + inttohex_param(displacement,5);
            }
        }
        objCode.push_back(objcode);
        pc = locctr[2+iterator];
    }
    objCode.push_back("");
    return returnInt;
}

void generateObjProgram(){
    string header = "H";
    header = header + programName;
    while(header.size()<7){
        header = header + ' ';
    }

    header = header + inttohex_param(startAddr,6) + inttohex_param(hextoint(programLength),6);
    objProgram.push_back(header);
    string textrec = "";
    string modifrec = "M";
    string endrec = "E";
    bool flag = 0;
    int counter = 0;
    int startaddr = startAddr;
    for(int i=1;i<objCode.size()-1;i++){
        flag = 0;
        if(locctr[i]!=locctr[i+1]){
            if(objCode[i]!=""){
                textrec = textrec + objCode[i];
                if(textrec.size()+objCode[i+1].size()>60){
                    flag=1;
                }

            }
            if(flag or objCode[i]==""){
                //PUSH INTO TEXT RECORD VECTOR
                textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
                objProgram.push_back(textrec);
                textrec = "";
                startAddr = locctr[i+1];
                while(objCode[i+counter+1]==""){
                    startAddr = locctr[i+counter+2];
                    counter++;
                }
                i = i + counter;
                counter = 0;
            }
        }
    }
    if(textrec!=""){
        textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
        objProgram.push_back(textrec);
    }
    for(int j=0;j<modification.size();j++){
        modifrec = modifrec + inttohex_param(modification[j],6) + inttohex_param(modificationSize[j],2);
        objProgram.push_back(modifrec);
        modifrec = "M";
    }
    for(int j=0;j<objCode.size();j++){
        if(objCode[j]!=""){
            endrec = endrec + inttohex_param(locctr[j],6);
            objProgram.push_back(endrec);
            break;
        }
    }
}

void generateObjProgram1(){
    int dd=1;
    string header = "H";
    header = header + programName;
    while(header.size()<7){
        header = header + ' ';
    }
    header = header + inttohex_param(startAddr,6) + inttohex_param(hextoint(programLength),6);
    objProgram.push_back(header);
    string definerec="D";
    for(int i=0;i<extDef.size();i++){
        string name=extDef[i];
        while(name.size()<6){
            name = name + ' ';
        }
        definerec=definerec+name+inttohex_param(symTab[findSymbol(extDef[i])].addr, 6);
    }
    if(extDef.size()!=0){
        objProgram.push_back(definerec);
        dd++;
    } 
    string referrec="R";
    for(int i=0;i<extRef.size();i++){
        string name=extRef[i];
        while(name.size()<6){
            name = name + ' ';
        }
        referrec=referrec+name;
    }
    if(extRef.size()!=0){
        objProgram.push_back(referrec);
        dd++;
    } 
    string textrec = "";
    string modifrec = "M";
    string endrec = "E";
    bool flag = 0;
    int counter = 0;
    int startaddr = startAddr;
    for(int i=dd;i<objCode.size();i++){
        flag = 0;
        if(locctr[i]!=locctr[i+1]){
            if(objCode[i]!=""){
                textrec = textrec + objCode[i];
                if(textrec.size()+objCode[i+1].size()>60){
                    flag=1;
                }

            }
            if(flag or objCode[i]==""){
                //PUSH INTO TEXT RECORD VECTOR
                textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
                objProgram.push_back(textrec);
                textrec = "";
                startAddr = locctr[i+1];
                while(objCode[i+counter+1]==""){
                    startAddr = locctr[i+counter+2];
                    counter++;
                }
                i = i + counter;
                counter = 0;
            }
        }
    }
    if(textrec!=""){
        textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
        objProgram.push_back(textrec);
    }
    for(int j=0;j<modification.size();j++){
        modifrec = modifrec + inttohex_param(modification[j],6) + inttohex_param(modificationSize[j],2)+modificationName[j];
        objProgram.push_back(modifrec);
        modifrec = "M";
    }
    for(int j=0;j<objCode.size();j++){
        if(objCode[j]!=""){
            endrec = endrec + inttohex_param(locctr[j],6);
            objProgram.push_back(endrec);
            break;
        }
    }
}

void generateObjProgram2(){
    int dd=1;
    string header = "H";
    header = header + programName;
    while(header.size()<7){
        header = header + ' ';
    }
    header = header + inttohex_param(startAddr,6) + inttohex_param(hextoint(programLength),6);
    objProgram.push_back(header);
    string definerec="D";
    for(int i=0;i<extDef.size();i++){
        string name=extDef[i];
        while(name.size()<6){
            name = name + ' ';
        }
        definerec=definerec+name+inttohex_param(symTab[findSymbol(extDef[i])].addr, 6);
    }
    if(extDef.size()!=0){
        objProgram.push_back(definerec);
        dd++;
    } 
    string referrec="R";
    for(int i=0;i<extRef.size();i++){
        string name=extRef[i];
        while(name.size()<6){
            name = name + ' ';
        }
        referrec=referrec+name;
    }
    if(extRef.size()!=0){
        objProgram.push_back(referrec);
        dd++;
    } 
    string textrec = "";
    string modifrec = "M";
    string endrec = "E";
    bool flag = 0;
    int counter = 0;
    int startaddr = startAddr;
    for(int i=dd;i<objCode.size();i++){
        flag = 0;
        if(locctr[i]!=locctr[i+1]){
            if(objCode[i]!=""){
                textrec = textrec + objCode[i];
                if(textrec.size()+objCode[i+1].size()>60){
                    flag=1;
                }

            }
            if(flag or objCode[i]==""){
                //PUSH INTO TEXT RECORD VECTOR
                textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
                objProgram.push_back(textrec);
                textrec = "";
                startAddr = locctr[i+1];
                while(objCode[i+counter+1]==""){
                    startAddr = locctr[i+counter+2];
                    counter++;
                }
                i = i + counter;
                counter = 0;
            }
        }
    }
    if(textrec!=""){
        textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
        objProgram.push_back(textrec);
    }
    for(int j=0;j<modification.size();j++){
        modifrec = modifrec + inttohex_param(modification[j],6) + inttohex_param(modificationSize[j],2)+modificationName[j];
        objProgram.push_back(modifrec);
        modifrec = "M";
    }
    // for(int j=0;j<objCode.size();j++){
    //     if(objCode[j]!=""){
    //         endrec = endrec + inttohex_param(locctr[j],6);
    //         objProgram.push_back(endrec);
    //         break;
    //     }
    // }
    objProgram.push_back(endrec);
}

void generateObjProgram3(){
    int dd=1;
    string header = "H";
    header = header + programName;
    while(header.size()<7){
        header = header + ' ';
    }
    header = header + inttohex_param(startAddr,6) + inttohex_param(hextoint(programLength),6);
    objProgram.push_back(header);
    string definerec="D";
    for(int i=0;i<extDef.size();i++){
        string name=extDef[i];
        while(name.size()<6){
            name = name + ' ';
        }
        definerec=definerec+name+inttohex_param(symTab[findSymbol(extDef[i])].addr, 6);
    }
    if(extDef.size()!=0){
        objProgram.push_back(definerec);
        dd++;
    } 
    string referrec="R";
    for(int i=0;i<extRef.size();i++){
        string name=extRef[i];
        while(name.size()<6){
            name = name + ' ';
        }
        referrec=referrec+name;
    }
    if(extRef.size()!=0){
        objProgram.push_back(referrec);
        dd++;
    } 
    string textrec = "";
    string modifrec = "M";
    string endrec = "E";
    bool flag = 0;
    int counter = 0;
    int startaddr = startAddr;
    for(int i=1;i<objCode.size()-1;i++){
        flag = 0;
        if(locctr[i]!=locctr[i+1]){
            if(objCode[i]!=""){
                textrec = textrec + objCode[i];
                if(textrec.size()+objCode[i+1].size()>60){
                    flag=1;
                }

            }
            if(flag or objCode[i]==""){
                //PUSH INTO TEXT RECORD VECTOR
                textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
                objProgram.push_back(textrec);
                textrec = "";
                startAddr = locctr[i+1];
                while(objCode[i+counter+1]==""){
                    startAddr = locctr[i+counter+2];
                    counter++;
                }
                i = i + counter;
                counter = 0;
            }
        }
    }
    if(textrec!=""){
        textrec = "T" + inttohex_param(startAddr,6) + inttohex_param((textrec.size()/2),2) + textrec;
        objProgram.push_back(textrec);
    }
    for(int j=0;j<modification.size();j++){
        modifrec = modifrec + inttohex_param(modification[j],6) + inttohex_param(modificationSize[j],2)+modificationName[j];
        objProgram.push_back(modifrec);
        modifrec = "M";
    }
    // for(int j=0;j<objCode.size();j++){
    //     if(objCode[j]!=""){
    //         endrec = endrec + inttohex_param(locctr[j],6);
    //         objProgram.push_back(endrec);
    //         break;
    //     }
    // }
    objProgram.push_back(endrec);
}

void printObjProgram(){
    cout<<"\n\n\t\tOBJECT PROGRAM\n\n";
    for(int i=0;i<objProgram.size();i++){
        cout<<objProgram[i]<<"\n";
    }
}

int main(){
    opCodeMapping();
    getInstructions();
    cout<<"-------------------------------------------------------------------------------*\n";
    if(CSind.size()==1){
        int a = firstPass();
        int b;
        if(a==1){
            b = secondPass();
            if(b==1){
                printListingFile(instructions);
                generateObjProgram();
                printObjProgram();
                cout<<"\n\n";
            }
        }
        // printList(instructions);
        // printLitTab();
        // printSymTab();
    }
    else{
        for(int i=0;i<CSind.size();i++){
            if(i==0){
                vector<vector<string>> instructions1={instructions.begin()+0, instructions.begin()+CSind[i+1]};
                int a = firstPass1(instructions1);
                // printLitTab();
                // printSymTab();
                int b;
                if(a==1){
                    b = secondPass1(instructions1);
                    if(b==1){
                        printListingFile(instructions1);
                        generateObjProgram1();
                        printObjProgram();
                        Finalobj.push_back(objProgram);
                        cout<<"\n\n";
                    }
                }
            }
            else if(i==CSind.size()-1){
                vector<vector<string>> instructions3={instructions.begin()+CSind[i], instructions.end()};
                int a = firstPass3(instructions3);
                // printLitTab();
                // printSymTab();
                int b;
                if(a==1){
                    b = secondPass3(instructions3);
                    if(b==1){
                        printListingFile(instructions3);
                        generateObjProgram3();
                        printObjProgram();
                        Finalobj.push_back(objProgram);
                        cout<<"\n\n";
                    }
                }
            }
            else{
                vector<vector<string>> instructions2={instructions.begin()+CSind[i], instructions.begin()+CSind[i+1]};
                int a = firstPass2(instructions2);
                // printLitTab();
                // printSymTab();
                int b;
                if(a==1){
                    b = secondPass2(instructions2);
                    if(b==1){
                        printListingFile(instructions2);
                        generateObjProgram2();
                        printObjProgram();
                        Finalobj.push_back(objProgram);
                        cout<<"\n\n";
                    }
                }
            }
            locctr.clear();
            objCode.clear();
            objProgram.clear();
            modification.clear();
            modificationSize.clear();
            modificationName.clear();
            programName="";
            programLength=-1;
            startAddr=0;
            org_address = -1;
            symTab.clear();
            litTab.clear();
            extDef.clear();
            extRef.clear();
        }
    }

    return 0;
}