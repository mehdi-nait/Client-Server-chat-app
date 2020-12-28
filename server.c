#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/select.h>

#define MAX_STR_LEN 9
#define MAX_CLT_NUM 100


int main(){
  char tab_clt[MAX_CLT_NUM][MAX_STR_LEN]; // table des clients
  char *msg_src=calloc(256,sizeof(char));
  char *msg_dst=calloc(256,sizeof(char));

  int n_clt=0;

  fd_set readfds,allfd; // all fd contient tous les descripteurs , readfds contient les descripteurs de lecture

  int max_ds=0; // maximum des descripteurs


  char clt_tag_dest[MAX_STR_LEN]; // client dest pour savoir à qui envoyer le msg
  char clt_tag_src[MAX_STR_LEN]; // etiquette client source , pour la concaténer avec le msg
  char clt_tag_first[MAX_STR_LEN]; // etiquette premiere connexion , pour l'ajouter à la table



  int i;
  struct sockaddr_in adr_srv,adr_clt;
  socklen_t leng_sock=sizeof(struct sockaddr);
  adr_srv.sin_family=AF_INET;
  adr_srv.sin_port=htons(8070);
  inet_aton("127.0.0.1",&(adr_srv.sin_addr));
  memset(&adr_srv.sin_zero,'0',8);
  int sock_serv=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  int sock_comm;
  if(bind(sock_serv,(struct sockaddr*)&adr_srv,leng_sock)==-1){
    printf("BIND ERROR: %s\n",strerror(errno));
    return EXIT_FAILURE;
  }
  if(listen(sock_serv,10)==-1){
    printf("LISTEN ERROR : %s\n",strerror(errno));
    return EXIT_FAILURE;
  }

  FD_ZERO(&readfds);
  FD_ZERO(&allfd);
  FD_SET(sock_serv,&allfd);
  max_ds=sock_serv; //c'est le plus grand des descripteurs

  int c1;

  while(1){
    readfds=allfd; //copie

    if(select(max_ds+1,&readfds,NULL,NULL,NULL)==-1){
      printf("SELECT ERROR : %s\n",strerror(errno));
      return EXIT_FAILURE;
    }

    for(i=sock_serv;i<=max_ds;i++){//boucler sur l'ensemble des descripteurs
                                  //pour savoir la source du déblocage
      if(FD_ISSET(i,&readfds)){

        if(i==sock_serv){
          //si c'est le sock_serv, c'est qu'une nouvelle connexion est reçue
          sock_comm=accept(sock_serv,(struct sockaddr*)&adr_clt,(socklen_t *)&leng_sock);
          if(sock_comm==-1){printf("ACCEPT ERROR :%s\n",strerror(errno));}
          n_clt++;//incrémenter le nombre de clients
          memset(clt_tag_first,'\0',9);
          snprintf(clt_tag_first, 10, "client %d:", n_clt); //écrire le nom du nouveau client connecté

          printf("nouvelle connexion, %s vient de rejoindre le chat\n",clt_tag_first); //afficher la nouvelle connexion

          // l'instruction suivante permet de generer une table de tous les clients connectees
          strcpy(tab_clt[n_clt-1],clt_tag_first);
          //envoyer le num client au client & le nombre de clients connectees


          if(max_ds<sock_comm){max_ds=sock_comm;} //ajustement du max et ajout à la liste des descripteurs
          FD_SET(sock_comm,&allfd);
        }
        else{
          //sinon c'est un message reçu d'un client
          memset(msg_src,'\0',256);
          recv(i,msg_src,256,0);
          printf("source: %s",msg_src); //on affiche le message

//************* Récupération de l'id du recepteur & composition du msg à envoyer au destinataire**********

          c1=0;
          memset(clt_tag_dest,'\0',MAX_STR_LEN);
          strncat(clt_tag_dest,msg_src,8);
          //copie de la partie "client num_cli" du msg
          // selection de la partie num_cli
          c1=atoi(&clt_tag_dest[7]); //conversion du numero du client  en int


//*************** Repérage du début du msg **************************
          memset(msg_dst,'\0',256);
          snprintf(clt_tag_src, 10, "client %d:", i-sock_serv); // préparation de la partie client num_client_src
          strcpy(msg_dst,clt_tag_src); // copie de la partie client source: au buffer pour avoir un message de la forme
                                      //client num_clt: message pour l'envoyer au destinataire
          char *ad1; // adresse du debut du message
          ad1=strchr(msg_src,':');
          if(ad1==NULL){
            printf("errreur\n");
          }
          else{
            strcat(msg_dst,ad1+1);

          }
          //printf("destination : %d\n",c1);

          //printf("msg destination:\n\n%s",msg_dst);


          //printf("client %d : %s\n",i-sock_serv,msg_src);
          // remove this line for broadcast only
          send(c1+sock_serv,msg_dst,strlen(msg_dst),0); //envoi du msg

        }
      }
    }



  }

}
