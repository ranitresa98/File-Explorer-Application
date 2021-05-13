#include <unistd.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <dirent.h>
#include <termios.h>
#include <time.h>
#include <sys/ioctl.h>
#include<sys/wait.h> 

using namespace std;

string root;
char * current_path;
struct termios initial_settings, new_settings;
int rows,cols;
int file_start,file_end;
int cursor_position=1;
vector<string> dir_list;
stack<string> back_trace;
// stack<string> forward_trace;
int command_line_print=0;


void clear_scr();
void non_canonical();
void restore();
void moveTo(int row, int col);
void print_list_dirs();
void print_dirs();
void normal_navigation();
void getWindowSize();
void command_mode();
void command_processing(string command);
string path_form(string path);
void make_dir(vector<string> commands_with_args);
void make_file(vector<string> commands_with_args);
void change_directory(vector<string> commands_with_args);
void remove_directory(vector<string> commands_with_args);
void remove_file(vector<string> commands_with_args);
void remove_directories(char *  path_dir);
void rename_file(vector<string> commands_with_args);
void copy_files(vector<string> commands_with_args);
void copy_file(char * from_path_dir, char * to_path_dir);
void copy_dir(char * from_path_dir, char * to_path_dir);
void move_files(vector<string> commands_with_args);
void move_file(char * from_path_dir, char * to_path_dir);
void move_dir(char * from_path_dir, char * to_path_dir);
string get_file_name(string path);
bool search_dirs(string search_path,string search_file);
bool search(vector<string> commands_with_args);
void window_resized(int sig);
void endsession(int sig);
void next_directory();



int main()
{
  signal(SIGWINCH, window_resized);  //signal on window resize
  signal(SIGINT, endsession);        //signal on interrupt ctrl+c to restore settings and clear screen
  non_canonical();                   //switch to non canonical mode
   char path[1024];
  
  if (getcwd(path, sizeof(path)) != NULL) {   //get pwd
         
   root=path;                         //set root
   current_path=path;               //set it as current path

 print_list_dirs();               //diaplay the directories

   normal_navigation();           //switch to normal navigation
 
 restore();                      //restore system settings
 }

else {
       perror("pwd");return 0;   //error on pwd
          }
   return 0;
}

void clear_scr() 
{cout<<"\x1b[2J";}     //clear screen

void getWindowSize() {   //get windows dimension
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col < 1) 
  { 
   perror("small screen");return; //error in ioctl or window size too small
  } else {
    cols = ws.ws_col; //get windows dimension
    rows = ws.ws_row;
   
  }
  
}

void window_resized(int sig) {  //window resized
   
    print_list_dirs();             //redisplay screen accordingly
     
}
void endsession(int sig) { //signal on interrupt ctrl+c to restore settings and clear screen
     restore();
    clear_scr();          
    moveTo(1,1);
    exit(sig);  
}


void moveTo(int row, int col) {   //seting cursor post
cout<<"\x1b["<<row<<";"<<col<<"H";
}



void non_canonical()               //set non cannonical mode
{
  tcgetattr(0,&initial_settings);     //save initial settings
   atexit(restore);
new_settings = initial_settings;         
new_settings.c_lflag &= ~ICANON;        //disable canonical mode
new_settings.c_lflag &= ~ECHO;            //switch off echo

if(tcsetattr(0, TCSANOW, &new_settings) != 0) {  
perror ("non_canonical");return;
}
}

void restore()
{
 clear_scr();
 tcsetattr(0,TCSANOW,&initial_settings);   //restore to original
}


void print_list_dirs()
{

DIR* dir;
struct dirent *direntries;

  
dir = opendir(current_path);         //open directory of current path
if (dir == NULL) {
      perror ("no directory entry");   //error opening directory
      return;    
 
}

dir_list.clear();                                  //clear previous list entries
while ((direntries=readdir(dir)) != NULL) {         //read the next directoty entry
  dir_list.push_back(direntries->d_name);          //push file names into list
    }
    if(closedir(dir)==-1){                     //close dir
       perror ("close dir");return;
      
    }

  file_start=0;file_end=dir_list.size();     //initialise start and ending of file positioning in gui
  print_dirs();                         //print dircetories
cursor_position=1;                             //set cursor position to initial ie 1
    moveTo(cursor_position,65);                 //set cursor on name of file
     
}



void print_dirs()
{
  clear_scr();         //clear screen
  getWindowSize();           //get window dimensions
  moveTo(1,1);
string dirname;
struct stat dirinfo;

 
 if(rows<dir_list.size() )             //if screen size is less than decide the end point of file i.e last file
 {
    file_end=file_start+rows-2;
 }
 
 sort(dir_list.begin(), dir_list.end());       //sort

 for(int i=file_start;i<file_end && i<dir_list.size();i++)       
 {
  dirname = string(current_path) + "/" + string(dir_list[i]);  //file path get
      char dirname_array[dirname.length()+1]; 
      strcpy(dirname_array, dirname.c_str()); 
      if(stat (dirname_array, &dirinfo)<0)         //get info of the file
      {perror(dirname_array);return;}

    char dir_name[dir_list[i].length()+1];    
      strcpy(dir_name, dir_list[i].c_str());   //file name
       
    

    char file_type='s';                                    //get file type
    file_type=(S_ISREG(dirinfo.st_mode)) ? '-' : file_type;
    file_type=(S_ISDIR(dirinfo.st_mode))? 'd' : file_type;
    file_type=(S_ISCHR(dirinfo.st_mode)) ? 'c' : file_type;
    file_type=(S_ISFIFO(dirinfo.st_mode))? 'p' : file_type;
    file_type=(S_ISLNK(dirinfo.st_mode))? 'l' : file_type;
    file_type=(S_ISBLK(dirinfo.st_mode))? 'b' : file_type;
    cout<<file_type;


    //file permissions                                  
    printf( (dirinfo.st_mode & S_IRUSR) ? "r" : "-");
    printf( (dirinfo.st_mode & S_IWUSR) ? "w" : "-");
    printf( (dirinfo.st_mode & S_IXUSR) ? "x" : "-");
    printf( (dirinfo.st_mode & S_IRGRP) ? "r" : "-");
    printf( (dirinfo.st_mode & S_IWGRP) ? "w" : "-");
    printf( (dirinfo.st_mode & S_IXGRP) ? "x" : "-");
    printf( (dirinfo.st_mode & S_IROTH) ? "r" : "-");
    printf( (dirinfo.st_mode & S_IWOTH) ? "w" : "-");
    printf( (dirinfo.st_mode & S_IXOTH) ? "x" : "-");


    cout<<"\t"<<setw(10)<<left<<dirinfo.st_size;  //size
      
         char *time_m= ctime(&dirinfo.st_mtime);   //modification tiem
     time_m[strlen(time_m)-1] = '\0';
      cout<<"\t"<<setw(25)<<left<<time_m; 
cout<<"\t"<<dir_name<<endl;                  //file name
  
      
 
    }
   }








  void normal_navigation()
  { moveTo(rows,1);
 cout<<"\x1b[0K"; 
    cout<<" NORMAl MODE"<<current_path;
    moveTo(cursor_position,65);  
    char ch;
    while(ch=cin.get()){
      
      
     if(ch==':') 
    {  
     
     command_mode(); 
    
     print_list_dirs();    
    }
    
     if (ch == 27) // up down left right
     {
      ch = cin.get();
      if (ch == '[') // up down left right
     {
      ch= cin.get();
    
         if(ch=='A') //up
        {
          if(cursor_position>1 )
         { 
          
          cursor_position--;       //update cursor position
          cout<<"\x1b[1A";  //move up
          }
        
        }
        
          else if(ch=='B')//down
          { 
            
          if(cursor_position<(rows-2) && cursor_position<dir_list.size()  )
         { 
          
          cursor_position++;        //update cursor position
            cout<<"\x1b[1B";  //move down
          }
       
          }
          
        else if(ch=='C')//right
          {
          
         
          next_directory();
          

          }
        
        else if(ch=='D')//left
          {
         

           if(back_trace.empty()==false)   //check stack if not empty
            { string path=back_trace.top();
              back_trace.pop();
              if(path==current_path)          //popped element current loc then pop again
                {if(back_trace.empty()==false)
                  {
                  path=back_trace.top();
                  back_trace.pop();} }
              strcpy(current_path, path.c_str());   //go to popped location
              print_list_dirs();
          }
          }     
      }
    }
     else if(ch=='h')//home
         {
        
       strcpy(current_path, root.c_str());   //go to root make in pwd
       print_list_dirs();
             
       }
       
    else   if(ch==127)//backspace
       { 
         
          string str=current_path;
           size_t found = str.find_last_of("/");  
          string parent=str.substr(0,found);          //get parent path i.e. before last /
          strcpy(current_path, parent.c_str());    //update pwd
          print_list_dirs();
          
       }
       


     else  if(ch==10)//enter 
      {
 
      string dirname;
      struct stat dirinfo; 
      string dir_name=string(dir_list[cursor_position+file_start-1]);  //get file name with help of cursor position and start
      if(dir_name==".")
      {back_trace.push(current_path); //push in stack
        continue;}
     else if(dir_name=="..")
    {
      
      string str=current_path;
      size_t found = str.find_last_of("/");
          string parent=str.substr(0,found);   //get parent directory
          strcpy(current_path, parent.c_str()); 
          print_list_dirs();
          back_trace.push(current_path);     //push in stack
    }
     else  { dirname = string(current_path) + "/" + dir_name;
       char child_path[dirname.length()+1]; 
      strcpy(child_path, dirname.c_str());    //get file path
      if(stat (child_path, &dirinfo)<0)     //get file status
      {perror(child_path);return;}
      if(S_ISDIR(dirinfo.st_mode))          //check if directory
        { 
           strcpy(current_path, dirname.c_str());  
          print_list_dirs();                    //go to dir
           back_trace.push(current_path);       //push in stack
        }
        else if(S_ISREG(dirinfo.st_mode)){           //check if file
        pid_t pid = fork();           //fork
          if (pid == 0) {                       //child
 
           if(dirname.find(".pdf")!= string::npos)        //if pdf file open default app
            execl("/usr/bin/xdg-open", "xdg-open", dirname.c_str(), NULL);

          else                                           //else open with vi
            execl("/usr/bin/vi", "vi", dirname.c_str(), NULL);
      exit(0);
      
    }
    else{
      wait(NULL);          //wait till child process finishes ie vi termination
    }
    
    }
    }
  }

   else if(ch=='k'){ 
       if(file_start>0 )
        {
          file_start-=(rows-2);  //update start and end of display
          file_start=file_start>=0?file_start:0;        
          file_end=file_start+(rows-2);
          print_dirs();              //reprint it
          cursor_position=1;         
          moveTo(cursor_position,65);
        } 
         }
    else  if(ch=='l'){ 
         if(file_end<dir_list.size()  )
        {
          file_start+=(rows-2);           //update start and end of display
          file_end+=(rows-2);
          file_end=file_end>=dir_list.size()?dir_list.size()-1:file_end;
          print_dirs();           //reprint it
          cursor_position=1; 
          moveTo(cursor_position,65);
        }
      }
    moveTo(rows,1);
    cout<<"\x1b[0K";
    cout<<" NORMAl MODE "<<current_path/*<<(int)ch*/;
    moveTo(cursor_position,65);  
  }
}




void next_directory()
{    
  string dirname;
  struct stat dirinfo;
  DIR* dir;
struct dirent *direntries;
 string str=current_path;
  size_t found = str.find_last_of("/");   //get parent dir
  string parent=str.substr(0,found);
  string file_name=str.substr(found+1);         //get dir name 
  vector<string> list_files;

char parent_array[parent.length()+1]; 
      strcpy(parent_array, parent.c_str());          
  dir = opendir(parent_array);                //open parent dir
if (dir == NULL) {
      perror ("no directory entry");return;
   }
 while ((direntries=readdir(dir)) != NULL) {
  list_files.push_back(direntries->d_name);         //get all parent dir files
    }
    if(closedir(dir)==-1){
       perror ("close dir");return;
      
    }
sort(list_files.begin(), list_files.end());
auto itr = find (list_files.begin(), list_files.end(), file_name);  //get dir present post
  
itr++;
    while (itr != list_files.end())    
    { 

      dirname = string(parent) + "/" + string(*itr);   //get next file/dir
      char dirname_array[dirname.length()+1]; 
      
      strcpy(dirname_array, dirname.c_str()); 
      if(stat (dirname_array, &dirinfo)<0)
      {perror(dirname_array);return;}
     if(S_ISDIR(dirinfo.st_mode))     //if next file dir open it and exit
        { 
          strcpy(current_path, dirname.c_str());
          print_list_dirs();
          return;
        }


    itr++; //else continue till end reached or next dir found
    } 
  

}





void command_mode()
{
  string command="";
  moveTo(rows,1);
 cout<<"\x1b[K";
    cout<< ":" ;

    char ch;
    while(ch=cin.get()){
      print_list_dirs();
      
    
    
            
    if(ch==27)            //esc
    { 
    
      normal_navigation();       //go to normal mode
    }
    
    else  if(ch==127)//backspace
       {
        if(!command.empty()) 
        command.pop_back();      //delete command last alphabet
       }
  else if(ch==10) //enter
{
if(!command.empty())              
{command_processing(command);     //process command
command="";
 
}
}
else{
command+=ch;
}


if(command_line_print==0)      //check if execution of other commands are interlaeved or not
  {moveTo(rows,1);cout<<"\x1b[K"; cout<<":"<<command;}  //dispaly command
else {command_line_print=0;}
   
}
}



void command_processing(string command)
{
vector<string> commands_with_args;
stringstream s (command);
    string item;

    while (getline (s, item,' ')) {
        commands_with_args.push_back (item);     //convert to list of words
          }

//goto respective command
if(commands_with_args[0]=="copy")
{copy_files(commands_with_args);return;}
else if(commands_with_args[0]=="move")
{move_files(commands_with_args);return;}
else if(commands_with_args[0]=="rename")
{rename_file(commands_with_args);return;}
else if(commands_with_args[0]=="create_file")
{make_file(commands_with_args);return;}
else if(commands_with_args[0]=="create_dir")
{make_dir(commands_with_args);return;}
else if(commands_with_args[0]=="delete_file")
{remove_file(commands_with_args);return;}
else if(commands_with_args[0]=="delete_dir")
{remove_directory(commands_with_args);return;}
else if(commands_with_args[0]=="goto")
{change_directory(commands_with_args);return;}
else if(commands_with_args[0]=="search")
{bool search_ans=search(commands_with_args);
  moveTo(rows,1);cout<<"\x1b[0K";cout<<":"<<boolalpha<<search_ans; command_line_print=1;return;}
else  //if incorrect command found intimate the user
  {moveTo(rows,1); cout<<"\x1b[0K"; cout<<":wrong command";command_line_print=1;return;}
}



string path_form(string path)
{
string final_path;
if(path[0]=='~' || path[0]=='/' )    //first ~ or / i.e. root
{final_path=root+ "/"; 
path=path.substr(1);
}   
else if(path[0]=='.')       //first . i.e. root
{final_path=string(root)+ "/";path=path.substr(1);
    if(path[0]=='.')           //2 . i.e. root parent 
  { string str=current_path;
    size_t found = str.find_last_of("/");     //get parent i.e. before last /      
     final_path=str.substr(0,found);      
      path=path.substr(1);            
    }
}
else
  {
final_path=string(root)+ "/";}    //append the file name to base to get the full path        
if(path[0]=='/')
{path=path.substr(1);}

final_path+=path;             //append the file name to base to get the full path 
return final_path;

}


void make_dir(vector<string> commands_with_args)
{
if (commands_with_args.size()!=3)    //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K";
    cout<<":invalid no of arguments"; 
    command_line_print=1;
    return;}
string path=path_form(commands_with_args[2]);   //get full path of destination
path+="/";
path+=commands_with_args[1];   //append new dir name to get dest

char path_dir[path.length()+1]; 
      strcpy(path_dir, path.c_str());
if( mkdir(path_dir, S_IRWXU | S_IRWXG | S_IXOTH| S_IROTH| S_IWOTH )<0) //make dir with 777 permissions
  {perror("cant make directory");return;}          //error mzg

}



void make_file(vector<string> commands_with_args)
{
if (commands_with_args.size()!=3)     //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K";
    cout<<":invalid no of arguments";
    command_line_print=1;
    return;}
string path=path_form(commands_with_args[2]); //get full path of destination
path+="/";
path+=commands_with_args[1];    //append new file name to get dest

char path_dir[path.length()+1];  
      strcpy(path_dir, path.c_str());
int fd=open(path_dir, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IXUSR|S_IWGRP|S_IXGRP|S_IRGRP|S_IROTH|S_IWOTH |S_IXOTH); //make file with 777 permissions
if( fd<0)
  {perror("cant make file");return;}         //error mzg
if(close(fd)<0){perror("cant close file");return;}          //close file
}



void change_directory(vector<string> commands_with_args)
{
  if (commands_with_args.size()!=2)     //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K";
    cout<<":invalid no of arguments";command_line_print=1;
    return;}
string path=path_form(commands_with_args[1]); //full path of dest



    DIR* dir; 
    char path_dir[path.length()+1]; 
     strcpy(path_dir, path.c_str());     
    dir = opendir(path_dir);   //open dir
if (dir == NULL) {
      perror ("no directory entry");       //error mzg
 return;
}
else  //if dir is valid
{
   strcpy(current_path, path.c_str());  //copy dest address in current path
        //back_trace.push(current_path);
          print_list_dirs();  return;                  //reprint 
}
}

void remove_directory(vector<string> commands_with_args)
{
  if (commands_with_args.size()!=2)      //check if no of arguments is right or not
{
  moveTo(rows,1);
cout<<"\x1b[0K"; 
    cout<<":invalid no of arguments";command_line_print=1;
    return;}
string path=path_form(commands_with_args[1]);    //get full path of dir to be deleted
char path_dir[path.length()+1]; 
      strcpy(path_dir, path.c_str());
      remove_directories(path_dir);        //call  remove_directories
}


void remove_directories(char * path_dir)
{struct stat dirinfo; 
  DIR* dir;
struct dirent *direntries;

  dir = opendir(path_dir);   //open dir
if (dir == NULL) {
      perror ("no directory entry");       //error mzg
 return;
}

while ((direntries=readdir(dir)) != NULL) {       //read dir entries one by one
  if(string(direntries->d_name)=="." || string(direntries->d_name)=="..")
    continue;                                        //if . or .. i.e self or parent ignore deleting
 string dirname = string(path_dir) + "/" + string(direntries->d_name);    //get full path of dir indide file file/dir
  char path[dirname.length()+1]; 
      strcpy(path, dirname.c_str()); 
      if(stat (path, &dirinfo)<0)   ///get status of internal file
      {perror(path);return;}
      if(S_ISDIR(dirinfo.st_mode))     //if directory recursively call remove_directories
        { 
          remove_directories(path);
        }
        else {
        if( remove(path)<0)              //if file delete file
  {perror(" can't delete file");return;          //error message
}
}
    }
    if(closedir(dir)==-1){            //close dir
       perror ("close dir");return;      //error message
    }
    if( rmdir(path_dir)<0)                  //as all inside contents now deleted can remove dir
  {perror(" can't delete directory");return;       //error message
}
}





void remove_file(vector<string> commands_with_args)
{
  if (commands_with_args.size()!=2)           //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K"; 
    cout<<":invalid no of arguments";
    command_line_print=1;
    return;}
string path=path_form(commands_with_args[1]);    //get full path of file
char path_dir[path.length()+1]; 
      strcpy(path_dir, path.c_str());
if( remove(path_dir)<0)                  //delete file
  {perror(" can't delete file");return;   //error mzg
}


}


void rename_file(vector<string> commands_with_args)
{
if (commands_with_args.size()!=3)    //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K"; 
    cout<<":invalid no of arguments";command_line_print=1;
    return;}
string from_path=path_form(commands_with_args[1]);     //get full path of file
size_t found = from_path.find_last_of("/");       
  string parent=from_path.substr(0,found);           //get parent of source
string to_path=parent+"/"+commands_with_args[2];        //get dest path by appending renamed name to parent

char from_path_dir[from_path.length()+1]; 
 strcpy(from_path_dir, from_path.c_str());
 char to_path_dir[to_path.length()+1]; 
 strcpy(to_path_dir, to_path.c_str());
if(rename(from_path_dir, to_path_dir) <0)          //rename from source to dest
  {perror("rename");return;}                //error mzg

}


string get_file_name(string path)   //to get file name
{
  size_t found = path.find_last_of("/");  //get last /
  string file_name=path.substr(found+1);   //get file name
  return file_name;
}


void copy_files(vector<string> commands_with_args)
{
  int n=commands_with_args.size();
if (n<3)              //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K"; 
    cout<<":invalid no of arguments";command_line_print=1;
    return;}


 for(int i=1;i<n-1;i++)        //for all source files
 {

   string from_path=path_form(commands_with_args[i]);  //get source path
  char from_path_dir[from_path.length()+1]; 
 strcpy(from_path_dir, from_path.c_str());

string to_path=path_form(commands_with_args[n-1]); //get dest path
   string to_name=get_file_name(from_path);
to_path=to_path+"/"+to_name;                  //apend source file name to dest to get final dest
char to_path_dir[to_path.length()+1]; 
 strcpy(to_path_dir, to_path.c_str());


copy_dir(from_path_dir, to_path_dir);        //call copy_dir

 

}

}




void copy_dir(char * from_path_dir, char * to_path_dir)
{
  struct stat dirinfo; 
  DIR* dir;
struct dirent *direntries;

if(stat (from_path_dir, &dirinfo)<0)       //get status of source
      {perror(from_path_dir);return;}

      if(S_ISDIR(dirinfo.st_mode))               //if dir
        {
          if( mkdir(to_path_dir, S_IRWXU | S_IRWXG | S_IXOTH| S_IROTH| S_IXOTH )<0)            //create dir of dest
              {perror("cant make directory");return;}    //error mzg
           dir = opendir(from_path_dir);           //open dir
            if (dir == NULL) {
                     perror ("no directory entry");return;}
       while ((direntries=readdir(dir)) != NULL) {    //read dir/files one by one
    if(string(direntries->d_name)=="." || string(direntries->d_name)=="..")
    continue;   //if self or parent continue
 string from_dirname = string(from_path_dir) + "/" + string(direntries->d_name);        //append dir name to dest
  char from_path[from_dirname.length()+1]; 
      strcpy(from_path, from_dirname.c_str());  
string to_dirname = string(to_path_dir) + "/" + string(direntries->d_name);     //append dir name to source
  char to_path[to_dirname.length()+1]; 
      strcpy(to_path, to_dirname.c_str());  
            copy_dir(from_path, to_path);       //call  copy_dir recursively
  
       }
if(closedir(dir)==-1){         //close dir
       perror ("close dir");return;   //error mzg
    }
               }


       else {  //if file
       copy_file(from_path_dir, to_path_dir);   //if file call copy_file
    

}
  }



void copy_file(char * from_path_dir, char * to_path_dir)
{
char copy_content[1024];
int from,to;
int nread;
from = open(from_path_dir, O_RDONLY);   //open the source file on read only mode

to = open(to_path_dir, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);             //open and ctreate dest file

while((nread = read(from,copy_content,sizeof(copy_content))) > 0)        //raed chars from source file
write(to,copy_content,nread);        //write read contents into dest file
if(close(from)<0){perror("cant close file");return;}         //error mzg
if(close(to)<0){perror("cant close file");return;}             //error mzg

}















void move_files(vector<string> commands_with_args)
{
  int n=commands_with_args.size();
if (n<3)           //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K"; 
    cout<<":invalid no of arguments";command_line_print=1;
    return;}

  

 for(int i=1;i<n-1;i++)
 {

   string from_path=path_form(commands_with_args[i]);  //get full path of source
  char from_path_dir[from_path.length()+1]; 
 strcpy(from_path_dir, from_path.c_str());

string to_path=path_form(commands_with_args[n-1]);   //get  path of dest
 string to_name=get_file_name(from_path);   
to_path=to_path+"/"+to_name;           //append file name to dest path to get full path
char to_path_dir[to_path.length()+1];

 strcpy(to_path_dir, to_path.c_str());


move_dir(from_path_dir, to_path_dir);  //call move_dir

 

}

}




void move_dir(char * from_path_dir, char * to_path_dir)
{
  struct stat dirinfo; 
  DIR* dir;
struct dirent *direntries;

if(stat (from_path_dir, &dirinfo)<0)   //get status of dest
      {perror(from_path_dir);return;}

      if(S_ISDIR(dirinfo.st_mode))          //if dir
        {
          if( mkdir(to_path_dir, S_IRWXU | S_IRWXG | S_IXOTH| S_IROTH| S_IXOTH )<0)      //make dir
              {perror("cant make directory");return;}         //error mzg
           dir = opendir(from_path_dir);          //open dir
            if (dir == NULL) {
                     perror ("no directory entry");return;}     //error mzg
       while ((direntries=readdir(dir)) != NULL) {         //read dir entries one by one
    if(string(direntries->d_name)=="." || string(direntries->d_name)=="..")
    continue;  //if parent or self ignore
 string from_dirname = string(from_path_dir) + "/" + string(direntries->d_name);  //apend file name to source
  char from_path[from_dirname.length()+1]; 
      strcpy(from_path, from_dirname.c_str());  
string to_dirname = string(to_path_dir) + "/" + string(direntries->d_name);      //apend file name to dest
  char to_path[to_dirname.length()+1]; 
      strcpy(to_path, to_dirname.c_str());  
            move_dir(from_path, to_path);          //call move_dir recursively
  
       }
if(closedir(dir)==-1){      //close dir
       perror ("close dir");return;  //error mzg
    }
    if( rmdir(from_path_dir)<0)            //remove dir
  {perror(" can't delete directory");return;    //error mzg
}
               }


       else {
       move_file(from_path_dir, to_path_dir);     //if file call move_file
    
}
  }



void move_file(char * from_path_dir, char * to_path_dir)
{
char copy_content[1024];
int from,to;
int nread;
from = open(from_path_dir, O_RDONLY);      //open source file
to = open(to_path_dir, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);     //open and create dest file
while((nread = read(from,copy_content,sizeof(copy_content))) > 0)     //read contents of source file into buffer
write(to,copy_content,nread);     //write read file into dest
if(close(from)<0){perror("cant close file");return;}
if(close(to)<0){perror("cant close file");return;}

if( remove(from_path_dir)<0)         //delete file
  {perror(" can't delete file");return;     //error mzg
}
}



bool search(vector<string> commands_with_args)
{
  if (commands_with_args.size()!=2)           //check if no of arguments is right or not
{moveTo(rows,1);
cout<<"\x1b[0K";
    cout<<":invalid no of arguments";command_line_print=1;
    return false;}
    
string search_path=path_form(".");    //get pwd
 return search_dirs(search_path,commands_with_args[1]);  //call search_dirs
 }





 bool search_dirs(string search_path,string search_file)
 {
queue<string> dir_searched_names;
dir_searched_names.push(search_path);    //push path into stack

 struct stat dirinfo; 
  DIR* dir;
struct dirent *direntries;
while(!dir_searched_names.empty())      //bfs  //while not empty queue
{
  string searching_path=dir_searched_names.front();    //get queue elemnet front
  dir_searched_names.pop();           //pop from queue
char searching_path_char[searching_path.length()+1]; 
      strcpy(searching_path_char, searching_path.c_str());

if(stat (searching_path_char, &dirinfo)<0)    //get status
 {perror(searching_path_char);return false;}     //error mzg

  if(S_ISDIR(dirinfo.st_mode))     //if dir
     {
     
        dir = opendir(searching_path_char);   //open dir
            if (dir == NULL) {
                     perror ("no directory entry");   //error mzg
                     return false;}
       while ((direntries=readdir(dir)) != NULL) {    //read file one by one
    if(string(direntries->d_name)=="." || string(direntries->d_name)=="..")
    continue;   //if self or parent skip
 
  if(search_file==direntries->d_name)  //check if name matches
    {return true;   }   
  string search_dirname = string(searching_path) + "/" + string(direntries->d_name);   //append file name
  char search_dir_path[search_dirname.length()+1]; 
      strcpy(search_dir_path, search_dirname.c_str());
      dir_searched_names.push(search_dir_path);   //push to queue
  
       }
if(closedir(dir)==-1){         //close dir
       perror ("close dir");return false;  //error mzg
    }
               }


       else {  //if filr
      
  string file_name=get_file_name(searching_path);  //get file name
  if(search_file==file_name)   //check if same
    {return true;   }    
  }

}

return false;        //not found
 }