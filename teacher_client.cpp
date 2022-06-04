#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>

#define PORT     8080
#define MAXMESSIZE 7000



int get_answer(){
    int answer;
    std::cout<< "Your answer (only number) : \n";
    std::cin >> answer;
    while(std::cin.fail()){
        std::cin.clear();
        std::cin.ignore(256,'\n');
        std::cout<< "Your answer (only number) : \n";
        std::cin >> answer;
    }
    
    return answer;
}

void create_test_Teacher(int sockfd){
    std::vector<int> answers;
    int st[]={0,1};
    write(sockfd, st, sizeof(st));
    char mes[MAXMESSIZE];
    
    read(sockfd, (char *)mes, sizeof(mes));
    printf("%s\n", mes);
    
    int size[1];
    std::cout<< "What is the amount of questions in test?(max 50)\n";
    std::cin >> size[0];
    write(sockfd, size, sizeof(size));
    
    
    for(int i = 0; i < size[0]; i ++){
        std::string question;
        std::cout<< "Write Question number "<< i+1 <<" :";
        std::cin.ignore();
        std::getline (std::cin,question);
        //std::cin>>question;
        char const *q = question.c_str();
        write(sockfd, q, strlen(q));
        std::cout<< "Write Possible answers "<< i+1 <<" : \n";
        for(int i = 0; i < 3; i++){
            std::string answ;
            std::cout<< i+1 <<") ";
            std::getline (std::cin,answ);
            char const *a = answ.c_str();
            write(sockfd, a, strlen(a));
        }
        std::cout<< "Write ID NUMBER of the correct answer from the possible options:\n";
        int correct[1];
        std::cin >> correct[0];
        write(sockfd, correct, sizeof(correct));
        }
    int id[1];
    read(sockfd, id, sizeof(id));
    std::cout<<"ID of your test is "<<id[0]<<". Give this ID to your students\n";
    
}


int main(int argc, const char * argv[]) {

    int sockfd;
    
    struct sockaddr_in     servaddr;
    
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Can't create a socket. Some error has occured");
        exit(EXIT_FAILURE);
    }
 
    
     servaddr.sin_family = AF_INET;
     servaddr.sin_addr.s_addr = INADDR_ANY;
     servaddr.sin_port = htons(PORT);
    
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    
    create_test_Teacher(sockfd);
    
    close(sockfd);
    
     return 0;
}
