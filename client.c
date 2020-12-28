#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>

int main(){
//********************* déclaration & Initialisation des variables *************
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  fd_set readfds,allfds; // déclaration des listes des descripteurs
  int max_ds=0; //varibale indiquant le maximum des descripteurs à surveiller
                //par select

  struct sockaddr_in addr_srv;
  char *buffer=(char*)calloc(256,sizeof(char)); //buffer qui sera utilisee pour
                                                //stocker le message source

  char *buffer2=(char*)calloc(256,sizeof(char)); //buffer qui sera utilisee pour
                                                //stocker le message à envoyer

//initialisation de la structure qui contient l'@ serveur

  addr_srv.sin_family=AF_INET;
  addr_srv.sin_port=htons(8070);
  inet_aton("127.0.0.1",&addr_srv.sin_addr);
  memset(&addr_srv.sin_zero,'0',8);
//creation du socket
  int sock_serv=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
//test de connexion
  if(connect(sock_serv,(struct sockaddr*)&addr_srv,sizeof(struct sockaddr))==-1){
    printf("CONNECT ERROR: %s",strerror(errno));
    return EXIT_FAILURE;
  }


//initialisation des listes de descripteurs
  FD_ZERO(&readfds);
  FD_ZERO(&allfds);

//ajout des descripteurs
  FD_SET(sock_serv,&allfds); //ajout du descripteur socket serveur ,
                            //il sert à indiquer une nouvelle connexion

  FD_SET(STDIN_FILENO,&allfds); //STDIN_FILENO descripteur utilisé pour savoir
                                //si des donnes peuvent etre lues du terminal


  max_ds=sock_serv; //pour l'instant le max est le descripteur du socket serveur
                    //(numero 3 ) il est le max de la liste all fd_set puisque
                    //allfds contient STDIN_FILENO (dont le num est 0) et sock_serv
    while(1){
      readfds=allfds; //copie de allfds dans readfds

      if(select(max_ds+1,&readfds,NULL,NULL,&timeout)==-1){ //select test
        printf("SELECT ERROR : %s\n",strerror(errno));
        return EXIT_FAILURE;
      // ici select surveille l'enemble readfds pour savoir si un descripteur
      //contient des donnees qui peuvent etre lus, avec un timeout de 0 secondes

      }
      //vérification de la source du deblocage
      if(FD_ISSET(sock_serv,&readfds)){  // si le socket serveur est la source
                                        //donc on a un message reçu
        memset(buffer,'\0',256);
        read(sock_serv,buffer,256);
        printf("->%s",buffer);
      }
      if(FD_ISSET(STDIN_FILENO,&readfds)){ // si c'est le terminal, c'est que
                                          // l'utilisateur saisit un message

        memset(buffer2,'\0',256);
        read(STDIN_FILENO,buffer2,256);
        send(sock_serv,buffer2,strlen(buffer2),0);
      }

  }

  close(sock_serv);
  return EXIT_SUCCESS;


}
