#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <cassert>
using namespace std;

#define allDelimiters "*+-/ ,.<>[]="
#define registerDelimiters "="
#define operationDelimiters "+-*/="


/*
reads from file and returns file as a stirng
*/

string readFromFile(string file){
    ifstream in(file);
    string ret;

    if(!in.is_open()){
       cout<<"Unable to open file"<<endl;
       exit(0);
    }

    string line;
    while(getline(in,line)){
        ret+=line+"\n";
    }

    in.close();

    return ret;
}
/*
writes given string in file
*/
void writeInFile(string file,string Ccode){
    ofstream out(file);

    if(!out.is_open()){
       cout<<"Unable to open file"<<endl;
       exit(0);
    }

    out<<Ccode<<endl;
}
/*
    returns true if computer is big endian
*/
bool isBigEndian(){
    int a=1;
    return *(char *)&a;
}

string MSZ(){
    return "1000000";
}


#define DEVELOP
#define addPrintf

/* removes comments from assembly code  and returns code without comments */
string removeComment(const string &a){
    string ret;
    bool x=0;
    for(int i=0;i<(int)a.size();i++){
        if(i+1<(int)a.size() && a[i]=='/' && a[i+1]=='/') x=1;
        if(a[i]=='\n') x=0;
        if(!x) ret+=a[i];

    }
    return ret;
}


/* removes any spaces in code */
string parse(const string &a){
    string ret;
    for(int i=0;i<(int)a.size();i++){
        if(!isspace(a[i])) ret+=a[i];
    }
    return ret;
}

/*
    merges string in vector between them inserts char x ( if x = 0 its not inserted)
*/
string merge(const vector<string> &a,char x=0){
    string ret;
    for(int i=0;i<(int)a.size();i++) {
        ret+=a[i];
        if(x && i+1!=(int)a.size()) ret+=x;
    }
    return ret;
}

/*
    splits string on characters in patt . if ki is 1 patt characters ar included in returned vector
*/
vector<string> splitAndParse(const string &s,const char *patt,bool ki=0){
  //  cout<<s<<endl;
    vector<string> lines;
    for(int i=0;i<(int)s.size();){
        int len=s.find_first_of(patt,i)-i+1;
        if(len<=0) len=s.size()-i+1;
        string lin = s.substr(i,len-1);
        string parsedS = parse(lin);
        if(parsedS.size()!=0)
            lines.push_back(parsedS);//,cout<<lin<<' '<<parsedS<<endl;
        i+=len-1;

        if(ki && i<(int)s.size()){
            lin=s.substr(i,1);
            lines.push_back(lin);//,cout<<lin<<' '<<parsedS<<endl;
        }
        i++;

    }
    return lines;

}


/*
    returns a as a string
*/
string toString(int a){
   string s;
   stringstream x;
   x<<a;
   x>>s;
   return s;
}
/* returns true if there are just digits */
bool isConstant(string s){
    for(int i=0;i<(int)s.size();i++) if(!isdigit(s[i])) return 0;
    return 1;
}
/*  retturns true if s is R and then digits*/
bool isRegister(string s){
    s=parse(s);
    if(s[0]!='R' || s.size()==1) return 0;

    return isConstant(s.substr(1));
}


string JMP(string a,int L){
    string x;
    int i;

    for(i=a.size()-1;isdigit(a[i]) || a[i]=='-' || a[i]=='+'; i--) x = a[i]+x;
    int ax=atoi(x.c_str())/4;
    ax=L+ax;
    x=toString(ax);

    x="goto L"+x+';';
   // cout<<x<<endl;
    return x;

}

string branch(string a,int L){
    vector<string> vv=splitAndParse(a," ,");
    //BEQ, BNE, BLT BLE, BGT BGE
    if(vv[0]=="BNE"){
        return "if( "+vv[1]+" != " + vv[2] +") " + JMP(vv.back(),L);
    }else if(vv[0]=="BEQ"){
        return "if( "+vv[1]+" == " + vv[2] +") " + JMP(vv.back(),L);
    }else if(vv[0]=="BLT"){
        return "if( "+vv[1]+" < " + vv[2] +") " + JMP(vv.back(),L);
    }else if(vv[0]=="BLE"){
        return "if( "+vv[1]+" <= " + vv[2] +") " + JMP(vv.back(),L);
    }else if(vv[0]=="BGT"){
        return "if( "+vv[1]+" > " + vv[2] +") " + JMP(vv.back(),L);
    }else if(vv[0]=="BGE"){
        return "if( "+vv[1]+" >= " + vv[2] +") " + JMP(vv.back(),L);
    }else assert(0);


}

string setTo(string a){
    if(a[0]=='R' && a.find_first_of('.')==-1 && a.find_first_of('M')==-1) return a+";\n";
    int X=4;

    if(a.find_first_of('.')!=-1){
        vector<string> vv=splitAndParse(a,". ",1);
        X = atoi(vv[2].c_str());
        vv.erase(vv.begin()+1);vv.erase(vv.begin()+1);vv.erase(vv.begin()+1);

        a=merge(vv);
    }

    string A,B;

    vector<string> vv=splitAndParse(a,"=");
    A=vv[0];B=vv[1];
    string ret;
    if(isConstant(B)){
        ret+="Rbuffer = "+vv[1]+";\n";
        B="Rbuffer";
    }
    if(isRegister(A)){
        ret+=A+"=0;\n";
    }

    if(A[0]=='M'){ //M[bla] = .X R

        ret += "memcpy(&" +  A+ " -"+toString(X-1)+", &" + B+", " +toString(X)+ ");\n";

    }else if(B[0]=='M'){ //R = .X M[bla]

        ret += "memcpy(&"+A+" , &"+B+"-"+toString(X-1)+", " + toString(X)+" );\n";

    }else if(isRegister(A)){ // R = .X R

        ret += "memcpy(&" +  A+ ", &" + B+", " +toString(X)+ ");\n";

    }

    return ret;
}


//M[bla]=...
string write(string a){
    int X=4;
    if(a.find_first_of('.')!=-1){
        vector<string> vv=splitAndParse(a,". ");
        X=atoi(vv[1].c_str());
        vv.erase(vv.begin()+1);
        a=merge(vv);
    }
    vector<string> vv=splitAndParse(a,"=");
    if(isConstant(vv[1])){
        string ret="buffer = "+vv[1]+";\n";
        assert(vv.size()==2);
        vv[1]="buffer";

        ret+=write(merge(vv,'='));
        return ret;
    }else{
        string ret="memcpy(&" +  vv[0]+ " -3, &" + vv[1]+", "+toString(X)+ ");\n";
        return ret;

    }

}

//R=...
string load(string a){
    if(a.find_first_of('.')!=-1){

        vector<string> vv=splitAndParse(a,". ",1);
        int X = atoi(vv[2].c_str());
        vv.erase(vv.begin()+1);vv.erase(vv.begin()+1);vv.erase(vv.begin()+1);

        a=merge(vv);

        string ret=load(a);
        vv[0]=vv[0].substr(0,vv[0].size()-1);
        if(X<4)
            ret+=vv[0] + " = " + "((1<<"+toString(8*X)+")-1) & " + vv[0]+";";

        return ret;
    }else if(a.find_first_of('M')==-1){
        return a+";\n";
    }else{
        vector<string> vv = splitAndParse(a,"=");
        string ret= "memcpy(&"+vv[0]+" , &"+vv[1]+"-3, 4 );\n";
        return ret;
    }


}
/* returns true if line is jump line */
bool isJump(const string &a){
    return a.substr(0,3)=="JMP";
}
/* returns true if its branch line*/
bool isBranch(const string &a){
    return a[0]=='B';
}
/* returns true if its M[bla]=... */
bool isWrite(const string &a){
    return a[0]=='M';
}

/* returns true if its R=... */
bool isLoad(const string &a){
    return a[0]=='R' || a[0]=='S';
}

/* translates one line */
string translate(string a,int L){
    if(a.substr(0,4)=="CALL") return "SP-=4;/*vitom PC block*/ \n"+splitAndParse(a,"<>")[1]+"();";
#ifdef addPrintf
    if(a=="RET") return "SP+=4;/*vitom PC block*/ \n printf(\"%d\\n\",RV);\n return;";
#else
    if(a=="RET") return "SP+=4;/*vitom PC block*/ \n  return;";
#endif // addPrintf

    if(isJump(a)) return JMP(a,L);
    if(isBranch(a)) return branch(a,L);
    if(isWrite(a)) return write(a);
    if(isLoad(a)) return load(a);
    cout<<"wtf is it master gio? "<<a<<endl;
    exit(-1);
}
/* translates 1 function */
string translate(vector<string> &lines){
    bool ismain = lines[0]=="~main~";

    vector<string> ret;
    for(int i=1;i<(int)lines.size()-ismain;i++){
        string tmp="L"+toString(i)+":    ";
        tmp+=translate(lines[i],i);

        ret.push_back(tmp);
    }
    if(ismain){
        return "int "+lines[0].substr(1,lines[0].size()-2)+"(){\n" + merge(ret,'\n')+"\n return 0;\n}\n\n\n";

    }else
        return "void "+lines[0].substr(1,lines[0].size()-2)+"(){\n" + merge(ret,'\n')+"\n}\n\n\n";
}

/* returns c code */
string translate(const vector<vector<string> > &functions){
    vector<string> ret;
    for(auto a : functions) ret.push_back(translate(a));
    return merge(ret,'\n');
}

void listRegisters(string a,vector<string> &ret){
    vector<string> v;
   // cout<<a<<endl;
    v=splitAndParse(a,registerDelimiters);
    for(auto x:v) if(isRegister(x)) ret.push_back(x);//,cout<<x<<' ';
   // cout<<endl;
}


string listRegisters(const vector<string> &lines){
    vector<string> ret;
    for(const auto &a : lines){
        listRegisters(a,ret);
    }
    sort(ret.begin(),ret.end());
    ret.resize(unique(ret.begin(),ret.end())-ret.begin());
    string re="int "+merge(ret,',')+";\n";
   // cout<<re<<endl;
    return re;

}
void reparse(string &a){
    if(a[0]=='B') a=a.substr(0,3)+" "+a.substr(3);
    for(int i=0;i<(int)a.size();i++){
        if(a[i]=='.')
            a=a.substr(0,i+2)+" "+a.substr(i+2);
    }
}
void reparse(vector<string> &a){
    for(auto &x : a) reparse(x);
}


vector<vector<string> > getFunctions(const vector<string> &lines){
    vector<vector<string> > ans;
    vector<string> tmp;
    for(int i=0;i<(int)lines.size();i++){
        if(lines[i][0]=='~' && i!=0) ans.push_back(tmp),tmp.clear();

        tmp.push_back(lines[i]);
    }
    if(tmp.size()) ans.push_back(tmp);

    return ans;
}


string getHeaderFunctions(const vector<vector<string> > &functions){
    string ret="#include<string.h>\n#include<stdio.h>\n";
    for(int i=0;i<(int)functions.size();i++){
        if(functions[i][0]=="~main~") ret+="int main();\n";
        else ret+="void "+functions[i][0].substr(1,functions[i][0].size()-2)+"();\n";
    }
    ret+="\n\n\n\n";
    return ret;
}
string getVariables(const vector<string> &lines){
    string ret;
    ret+="unsigned char M["+MSZ()+"];\n";
    ret+="int SP="+toString(atoi(MSZ().c_str())-1)+",RV,buffer;\n";
    ret+=listRegisters(lines)+"\n\n\n\n";

    return ret;
}

string getTopCode(const vector<vector<string> > &functions, const vector<string> lines){
    return getHeaderFunctions(functions)+getVariables(lines);
}

string solve(string Acode){
    vector<string> lines;//delimiter is ;
    Acode = removeComment(Acode);


    lines=splitAndParse(Acode,";");
    reparse(lines);

    vector<vector<string> > functions = getFunctions(lines);
    string ret=getTopCode(functions,lines);


    ret+=translate(functions);



    return ret;
}


int main()
{


    string file;
#ifdef DEVELOP
    file = "in.txt";
#else
    cout<<"write in file"<<endl;
    cin>>file;
#endif // DEVELOP

    string Acode = readFromFile(file);
    string Ccode = solve(Acode);



#ifdef DEVELOP
    file = "out.txt";
#else

    cout<<"write out file"<<endl;
    cin>>file;
#endif // DEVELOP

    writeInFile(file,Ccode);


    return 0;
}
