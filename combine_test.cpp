#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>                         
#include <unistd.h>               
#include <pthread.h>
#include <iostream> 
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sstream>
#include <queue>
#include <vector>
static int  count=0;
int Threadnum=0;
char *buf=NULL;
char *buf1=NULL;
int bs=0;
std::queue<std::string> Targetpath;
std::vector<pthread_t> IdVec;
pthread_mutex_t mutex;

void *Thread_throughput(void *arg)
{
    std::stringstream th;
    th<<Threadnum;
    std::stringstream s;
    s<<bs;
    std::string ss="read_throughput.log_";
    std::string log=ss+th.str()+"_"+s.str();
    int fd = open(log.c_str(),O_CREAT | O_WRONLY | O_APPEND);
    off_t offset = 0;
    size_t rw = 0;
    ssize_t ret = 0;
    
    while(1)
    {
       int temp=count;
       sleep(10);
       //std::cout <<"c:"<<count<<std::endl;
       double coverage=double(bs*(count-temp))/10;
       std::stringstream ss;
       ss<<coverage;
       //std::string s=ss.str()+"KB\n   ";
       std::string s="echo "+ss.str()+"KB  >> "+log;
       system(s.c_str());
       //std::cout <<"c:"<<coverage<< ","<<s<< std::endl;
       #if 0
       //buf1=const_cast<char*>(s.c_str());
       //size_t size=64;
       
       write(fd,s.c_str(),sizeof(s));  
       #endif
       
       if(temp==count)
       {
         sleep(300);
         std::cout <<"e"<<std::endl;
         break;
       }
    }
    close(fd);
}

static void *Thread_read(void *arg)  
{   
   
    while(!Targetpath.empty())
    {
       std::string file=Targetpath.front();
       Targetpath.pop();
       //std::cout <<"open:'" << file << std::endl; 
       int fd=open(file.c_str(),O_RDONLY);
       size_t size= bs*1024;
       size_t len=0;
       while(1)
       {
           len=read(fd,buf,size);
           pthread_mutex_lock(&mutex);
           ++count;
           pthread_mutex_unlock(&mutex);
           //std::cout <<"c" << std::endl;
           if(len<=0)
             break;
       }
       
       //Targetpath.pop();
       close(fd);
    }  
    //std::cout <<"stop thread " << std::endl;   
 
}
void Wait()
{
   #if 1
   for(std::vector<pthread_t>::iterator it = IdVec.begin();it!=IdVec.end();++it)
   {
      if(*it !=-1)
      {
        pthread_join(*it,NULL);
        *it =-1;
      }
   }
   #endif
   IdVec.clear();
}
int main(int argc, char *argv[])
{
   if(argc!=4)
   {
      std::cout <<"argc invalid, example: -d -threadnum -bs " << std::endl;
   }
   
   //std::cout <<argv[1]<<","<<argv[2]<<","<<argv[3]<<std::endl;
   Threadnum=atoi(argv[2]);
   bs=atoi(argv[3]);
   std::string s=argv[1];
   pthread_mutex_init(&mutex,NULL);
   pthread_t thread_id;
   DIR * dir;
   struct dirent * ptr;
   dir = opendir(argv[1]);
   while((ptr = readdir(dir)) != NULL)
   {
      if(ptr->d_name[0]=='.')
         continue;
      
      std::string ss="";
      ss=s+ptr->d_name;
      //std::cout <<"d:" << ss <<std::endl;
      Targetpath.push(ss);
   }
   closedir(dir);
   
   //std::cout <<"bs: " << bs << ",pthreadnum: " << Threadnum<<std::endl;
   buf=new char[bs*1024+1];
   buf1=new char[64];
   pthread_create(&thread_id, NULL, Thread_throughput, NULL);
   IdVec.push_back(thread_id);
   //idx = new struct index ; 
   for(int i = 0; i<Threadnum;++i)
   {
      pthread_create(&thread_id, NULL, Thread_read, NULL);
      IdVec.push_back(thread_id);
   }
   Wait();
   pthread_mutex_destroy(&mutex);
   if(buf1!=NULL)
   {
      delete []buf1;
      buf1=NULL;
   }
   if(buf!=NULL)
   {
      delete []buf;
      buf=NULL;
   }
   //Targetpath.clear();
   return 0;
}
