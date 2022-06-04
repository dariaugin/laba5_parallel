#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>


#define PORT 8080


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

void start_test_session_Student(int sockfd){
    int id_test;
    std::cout<< "Enter the id of your test:";
    std::cin>> id_test;
    int st[2]={1};
    st[1] =  id_test;
    write(sockfd, st, sizeof(st));
    int status[1];
    read(sockfd, status, sizeof(status));
    if(status[0] == 1){
    std::cout<<"Test session has started... \n";

    
    int amount_q[1];
    int answer[1];
    char results_buffer[10000];
    char mes[5000];
    
    read(sockfd, amount_q, sizeof(amount_q));
    for(int i = 0; i < amount_q[0]; i ++){
        int c = read(sockfd, mes, sizeof(mes));
        mes[c] = '\0';
        std::cout<<mes<<"\n";
        answer[0] = get_answer();
        std::cout<< "Your answer is - " << answer[0]<<"\n\n";
        write(sockfd, answer, sizeof(answer));
        }
    
    int b = read(sockfd, (char *)results_buffer, sizeof(results_buffer));
    results_buffer[b] = '\0';
    std::cout<< results_buffer << "\n";
    }else{
       std::cout<< "Sorry, this test doesn`t exist.\n";
    }
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
    
    start_test_session_Student(sockfd);
    close(sockfd);
    return 0;
}

