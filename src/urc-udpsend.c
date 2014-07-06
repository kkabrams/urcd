#define USAGE "Usage: urc-udpsend addr port /path/to/sockets/\n"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <sys/un.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <stdio.h>
#include <poll.h>
#include <pwd.h>

#ifndef UNIX_PATH_MAX
 #ifdef __NetBSD__
  #define UNIX_PATH_MAX 104
 #else
  #define UNIX_PATH_MAX 108
 #endif
#endif

int itoa(char *s, int n, int slen)
{
  if (snprintf(s,slen,"%d",n)<0) return -1;
  return 0;
}

main(int argc, char **argv)
{

 int udpfd;
 struct sockaddr_in udp;
 bzero(&udp,sizeof(udp));
 udp.sin_family = AF_INET;

 if (
    (argc<4)
 || (!(udp.sin_port=htons(atoi(argv[2]))))
 || (!inet_pton(AF_INET,argv[1],&udp.sin_addr))
 || ((udpfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))<0)
 || (connect(udpfd,(struct sockaddr *)&udp,sizeof(udp))<0)
    )
 {
  write(2,USAGE,strlen(USAGE));
  exit(64);
 }

 char buffer[1024] = {0};
 char user[UNIX_PATH_MAX] = {0};
 if (itoa(user,getpid(),UNIX_PATH_MAX)<0) exit(1);

 if (chdir(argv[3])) exit(64);
 struct passwd *urcd = getpwnam("urcd");
 if ((!urcd) || ((chroot(argv[3])) || (setgid(urcd->pw_gid)) || (setuid(urcd->pw_uid)))) exit(64);

 int sockfd;
 struct sockaddr_un sock;
 bzero(&sock,sizeof(sock));
 sock.sun_family = AF_UNIX;

 void sock_close(int signum)
 {
   unlink(sock.sun_path);
   exit(signum);
 } signal(SIGINT,sock_close); signal(SIGHUP,sock_close); signal(SIGTERM,sock_close); 

 if ((sockfd=socket(AF_UNIX,SOCK_DGRAM,0))<0) exit(2);
 int n = 1;
 if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&n,sizeof(n))<0) exit(3);
 n = strlen(user);
 if (n > UNIX_PATH_MAX) exit(4);
 memcpy(&sock.sun_path,user,n);
 unlink(sock.sun_path);
 if (bind(sockfd,(struct sockaddr *)&sock,sizeof(sock))<0) exit(5);

 struct sockaddr_un path;
 path.sun_family = AF_UNIX;
 socklen_t path_len = sizeof(struct sockaddr_un);

 while (1)
 {
  bzero(path.sun_path,UNIX_PATH_MAX);
  n = recvfrom(sockfd,buffer,1024,0,(struct sockaddr *)&path,&path_len);
  if (!n) continue;
  if (n<0) sock_close(6);
  if (!path_len) continue;
  if (buffer[n-1] != '\n') continue;
  write(udpfd,buffer,n);
 }
}
