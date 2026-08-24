#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <deque>
#include <sys/wait.h>
#include <fcntl.h>
#define N_HISTORY 8
using namespace std;
enum EXECMODE{
    FOREGROUND,BACKGROUND
};
struct ExecOptions
{
    EXECMODE mode=FOREGROUND;
    string inputFile;
    string outputFile;
};
void parse(string in,vector<string> &out);
void exec_cd(string path);
void exec_ls(string path);
void exec_rm(vector<string> args);
void exec_mkdir(string path);
void exec_command(vector<string> args,const ExecOptions &opt);
void removeRecursive(const string &path, bool force, bool verbose);
void add_history(deque<string> &his,const string &command);
void do_command(const string &command,deque<string> &history);
void parse_options(vector<string> &args,ExecOptions &opt);
int main()
{
    string command;
    deque<string> his;

    while(1)
    {
        getline(cin,command);
        if(command.empty())
            continue;
        do_command(command,his);
    }
}
void do_command(const string &command,deque<string> &his)
{
    vector<string> parsed;
    parse(command,parsed);
    if(parsed.empty())
        return;
    if(parsed[0]=="quit")
        exit(0);
    else if(parsed[0]=="cd")
    {
        if(parsed.size()>1)
        {
            exec_cd(parsed[1]);
            add_history(his,command);
        }
        else cout<<"No path to cd"<<endl;
    }
    else if(parsed[0]=="ls")
    {
        if(parsed.size()>1)
            exec_ls(parsed[1]);
        else
            exec_ls(".");
        add_history(his,command);

    }
    else if(parsed[0]=="rm")
    {
        exec_rm(parsed);
        add_history(his,command);
    }
    else if(parsed[0]=="mkdir")
    {
        if(parsed.size()>1)
        {
            exec_mkdir(parsed[1]);            
            add_history(his,command);
        }
        else
            cout<<"No path to mkdir"<<endl;
    }
    else if(parsed[0]=="history")
    {
        int n;
        try
        {
            n=parsed.size()>1?stoi(parsed[1]):N_HISTORY;
        }
        catch(...)
        {
            cout<<"n not valid"<<endl;
            return;
        }
        if(n<1)
        {
            cout<<"n must be greater than 0";
            return;
        }
        n=n<his.size()?n:his.size();
        int start=max(0,(int)his.size()-n);
        for(int i=start;i<his.size();i++)
            cout<<i+1<<" "<<his[i]<<endl;
    }
    else if(parsed[0]=="issue")
    {
        if(parsed.size()<=1)
        {
            cout<<"n is missing"<<endl;
            return;
        }
        int n;
        try
        {
            n=stoi(parsed[1]);
        }
        catch(...)
        {
            cout<<"n not valid"<<endl;
            return;
        }
        if(n>=1&&n<=his.size())
        {
            string comm=his[n-1];
            if(comm==command)
            {
                cout<<"cannot execute issue"<<endl;
                return;
            }
            do_command(comm,his);
        }
        else
        {
            cout<<"history entry not foun"<<endl;
        }
    }
    else
    {
        ExecOptions opt;
        if(!parsed.empty()&&parsed.back()=="&")
        {
            opt.mode=BACKGROUND;
            parsed.pop_back();
        }
        parse_options(parsed,opt);
        exec_command(parsed,opt);
    }
}
void parse(string in,vector<string> &out)
{
    string temp;
    for(char c:in)
    {
        if(c!=' ' and c!='\n')
            temp+=c;
        else
        {
            if(!temp.empty())
            {
                out.push_back(temp);
                temp.clear();
            }
        }
    }

    if(!temp.empty())
        out.push_back(temp);
}
void exec_cd(string path)
{
    if(chdir(path.c_str())!=0)
        perror("cd not found");
}
void exec_ls(string path)
{
    DIR *d;
    struct dirent *file;
    d=opendir(path.c_str());
    if(d==NULL)
    {
        perror("ls error");
        return;
    }
    while((file=readdir(d))!=NULL)
        cout<<file->d_name<<endl;
    closedir(d);
}
void exec_rm(vector<string> args)
{
    bool recursive = false;
    bool force = false;
    bool verbose = false;

    vector<string> files;

    for(size_t i = 1; i < args.size(); i++)
    {
        if(args[i][0] == '-')
        {
            for(size_t j = 1; j < args[i].size(); j++)
            {
                switch(args[i][j])
                {
                    case 'r':
                        recursive = true;
                        break;

                    case 'f':
                        force = true;
                        break;

                    case 'v':
                        verbose = true;
                        break;

                    default:
                        cout << "rm: opzione sconosciuta -" << args[i][j] << endl;
                }
            }
        }
        else
        {
            files.push_back(args[i]);
        }
    }

    for(const string &file : files)
    {
        struct stat st;

        if(stat(file.c_str(), &st) != 0)
        {
            if(!force)
                perror(file.c_str());
            continue;
        }

        if(S_ISDIR(st.st_mode))
        {
            if(recursive)
            {
                removeRecursive(file, force, verbose);
            }
            else
            {
                cerr << "rm: '" << file
                     << "' è una directory (usa -r)" << endl;
            }
        }
        else
        {
            if(unlink(file.c_str()) != 0)
            {
                if(!force)
                    perror(file.c_str());
            }
            else if(verbose)
            {
                cout << "removed '" << file << "'" << endl;
            }
        }
    }
}
void exec_mkdir(string path)
{
    if(mkdir(path.c_str(),0777)!=0)
        perror("mkdir error");
}
void removeRecursive(const string &path, bool force, bool verbose)
{
    struct stat st;

    if(stat(path.c_str(), &st) != 0)
    {
        if(!force)
            perror(path.c_str());
        return;
    }

    if(S_ISDIR(st.st_mode))
    {
        DIR *dir = opendir(path.c_str());

        if(dir == NULL)
        {
            if(!force)
                perror(path.c_str());
            return;
        }

        struct dirent *entry;

        while((entry = readdir(dir)) != NULL)
        {
            string name = entry->d_name;

            if(name == "." || name == "..")
                continue;

            removeRecursive(path + "/" + name, force, verbose);
        }

        closedir(dir);

        if(rmdir(path.c_str()) != 0)
        {
            if(!force)
                perror(path.c_str());
        }
        else if(verbose)
        {
            cout << "removed directory '" << path << "'" << endl;
        }
    }
    else
    {
        if(unlink(path.c_str()) != 0)
        {
            if(!force)
                perror(path.c_str());
        }
        else if(verbose)
        {
            cout << "removed '" << path << "'" << endl;
        }
    }
}
void add_history(deque<string> &his,const string &command)
{
    if(his.size()>=N_HISTORY)
        his.pop_front();
    his.push_back(command);
}
void exec_command(vector<string> args,const ExecOptions &opt)
{
    if(args.empty())
        return;
    pid_t pid=fork();
    if(pid<0)
    {
        perror("fork");
        return;
    }
    if(pid==0)
    {
        if(!opt.inputFile.empty())
        {
            int fd=open(opt.inputFile.c_str(),O_RDONLY);
            if(fd<0)
            {
                perror("input file error");
                exit(EXIT_FAILURE);
            }
            if(dup2(fd,STDIN_FILENO)<0)
            {
                perror("dup2");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        if(!opt.outputFile.empty())
        {
            int fd=open(opt.outputFile.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
            if(fd<0)
            {
                perror("output file error");
                exit(EXIT_FAILURE);
            }
            if(dup2(fd,STDOUT_FILENO)<0)
            {
                perror("dup2");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        vector<char*> argv;
        for(auto &s:args)
        {
            argv.push_back(const_cast<char*>(s.c_str()));
        }
        argv.push_back(nullptr);
        execvp(argv[0],argv.data());
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {    
        if(opt.mode==FOREGROUND)
            waitpid(pid,nullptr,0);
        else
            cout<<"background process with pid="<<pid<<endl;
    }
}
void parse_options(vector<string> &args,ExecOptions &opt)
{
    vector<string> cleanArgs;
    for(size_t i=0;i<args.size();i++)
    {
        if(args[i]==">")
        {
            if(i+1<args.size())
            {
                opt.outputFile=args[i+1];
                i++;
            }
            else
            {
                cout<<"no output file"<<endl;
                return;
            }
        }
        else if(args[i]=="<")
        {
            if(i+1<args.size())
            {
                opt.inputFile=args[i+1];
                i++;
            }
            else
            {
                cout<<"no input file"<<endl;
                return;
            }
        }
        else
        {
            cleanArgs.push_back(args[i]);
        }
    }
    args=cleanArgs;
}