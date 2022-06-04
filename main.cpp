
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>


std::mutex mut;
#define PORT 8080

struct Test;


std::vector<Test> myTests;
struct Test{
    int sizeT = 0;
    std::string possible_answers[150];
    std::string questions[50];
    int correct_answers[50];
    
    Test (){};
    
    public:

    void createTest( int sd){
        std::cout<< "--- createTest() started...\n";
        int idd[1];
        int size[1];
        const char* mes = "Server waiting for your Test Session data...";
        write( sd, mes , strlen(mes));
        read( sd, size, sizeof(size));
        sizeT = size[0];
        int i;
        for(i = 0; i< size[0]; i++){
            char question[5000];
            read(sd, question, sizeof(question));

            std::string stri(question);
            questions[i] = stri;
            memset(question, 0, sizeof(question));
            for(int k = 0; k <3; k++){
            char possible_answer[5000];
            read(sd, possible_answer, sizeof(possible_answer));
            std::string str_(possible_answer);
            int ind = i*3 +k;
            possible_answers[ind] = str_;
            memset(possible_answer, 0, sizeof(possible_answer));
            }
            
            int correct_answer[1];
            read(sd, correct_answer, sizeof(correct_answer));
            correct_answers[i] = correct_answer[0];
        }
        mut.lock();
        myTests.push_back(*this);
        idd[0] = myTests.size() - 1;
        mut.unlock();
        write( sd,idd,sizeof(idd));
        close( sd );
        std::cout<< "--- Test has been created...\n";
    }
    
};

struct Student_Test_Session{
    int sd;
    Test test;

    
    Student_Test_Session(  int sd_, Test test_): sd(sd_), test(test_) {};
    
    int calculate_mark(){
        return 0;
    }
    std::string compile_answers(int q){
        std::string options;
        int start = q*3 - 3;
        int id = 1;
        for(int i = start; i < start+3; i++ ){
            options += (std::to_string(id)+") "+ test.possible_answers[i] + "\n");
            id++;
        }
        return options;
    }
    
    void  start_Test_Session(){
        
        std::cout<<"testing has started\n";
        std::vector<int> student_answers;
        int answ[1];
        int am[1];
        am[0] = test.sizeT;
        write(sd, am, sizeof(am));
        for(int i = 0; i < test.sizeT; i++){
            
            std::string aa = test.questions[i]+ "\nOptions:\n"+compile_answers(i+1);
            char const *num_char = aa.c_str();
            write( sd, num_char , strlen(num_char));
            read( sd, answ, sizeof(answ));
            student_answers.push_back(answ[0]);

        }

        check_send_result(student_answers);
        std::cout<<"testing has finished\n";
        close( sd );
    }
    
    
    void check_send_result(std::vector<int> student_answers){

        std::string result = "";
        int score = 0;
        for(int i = 0; i < test.sizeT; i++){
            result += ("Question number " + std::to_string(i));
            if(student_answers[i] == test.correct_answers[i]){
                result += " - correct\n";
                score++;
            }else{
                result += " - wrong\n";
            }
        }
        result += "Your result is " + std::to_string(score) + "/" + std::to_string(test.sizeT) + "\nMark: " + std::to_string(score*100/test.sizeT) +"%";
        char const *res = result.c_str();
        send( sd, res , strlen(res), 0 );
    }
    
};



struct Server{
    int master_socket , addrlen , new_socket , client_socket[30] ,
    max_clients = 50 , activity, valread , sd, maxsd;
    struct sockaddr_in address;
    fd_set readfds;
    std::vector<std::thread> threads;
    
    void start(){
        bind_and_llisten();
        handleConnection();
    }
    
    void bind_and_llisten(){
        if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
            {
                perror("socket failed");
                exit(EXIT_FAILURE);
            }
        
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons( PORT );
            
            
            if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
            {
                perror("bind failed");
                exit(EXIT_FAILURE);
            }
            printf("PORT %d \n", PORT);
            
            if (listen(master_socket, 3) < 0)
            {
                perror("listen failed");
                exit(EXIT_FAILURE);
            }

            addrlen = sizeof(address);
            printf("SERVER waiting for connections ...");
    }
    void handleConnection(){
        for (int i = 0; i < max_clients; i++)
        {
            client_socket[i] = 0;
        }
        while(true)
        {
            FD_ZERO(&readfds);
            FD_SET(master_socket, &readfds);
            maxsd = master_socket;
            
            for ( int i = 0 ; i < max_clients ; i++)
            {
                sd = client_socket[i];

                if(sd > 0)
                    FD_SET( sd , &readfds);
                if(sd > maxsd)
                    maxsd = sd;
            }
            
            activity = select( maxsd + 1 , &readfds , NULL , NULL , NULL);
            
            if ((activity < 0) && (errno!=EINTR)){
                    printf("select failed");
            }
                
            
            if (FD_ISSET(master_socket, &readfds))
                {
                    handleNewConnection();
                }
                
            }
        
        
    }
    void startNewThreadTeacher(int ns){

        Test t;
        auto th = std::thread(&Test::createTest,t,ns);
        th.detach();
        
    }
    void startNewThreadStudent(int ns, int id){
        
        if(id < myTests.size() && id >= 0){
        int status[1] = {1};
        write(ns, status, sizeof(status));
        Test t = myTests[id];
        Student_Test_Session student(ns, myTests[id]);
        auto th = std::thread(&Student_Test_Session::start_Test_Session,student);
        th.detach();
        client_socket[ns] = 0;
        }else{
            int status[1] = {0};
            write(ns, status, sizeof(status));
            close( sd );
            client_socket[ns] = 0;
        }
        
    }
    
    
    void handleNewConnection(){
        if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            if (new_socket > maxsd) {
                maxsd = new_socket;
                
            }
        
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs
                (address.sin_port));
                
            int status[2];
        
                if(read( new_socket, status, sizeof(status))){
                   
                    if(status[0] == 0){
                        
                        startNewThreadTeacher(new_socket);
                    }else{
                        startNewThreadStudent(new_socket, status[1]);
                    }
                }else{
                    perror("unknown connection");
                    exit(EXIT_FAILURE);
                }

        }
};

int main(int argc , char *argv[])
{
    Server server;
    server.start();
    return 0;
}
