#include <nacl/crypto_scalarmult_curve25519.h>
#include <nacl/crypto_hash_sha512.h>
#include <nacl/crypto_verify_32.h>
#include <nacl/crypto_sign.h>
#include <nacl/crypto_box.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <strings.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pwd.h>

#include "base16.h"

#define USAGE "./cryptoserv /path/to/sockets/ /path/to/root/\n"

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

void randombytes(char *bytes) {} // override: hack crypto_*_keypair functions

void lower(
 unsigned char *buffer0,
 unsigned char *buffer1,
 int buffer1_len
) {
 int i;
 for(i=0;i<buffer1_len;++i) {
  if ((buffer1[i]>64)&&(buffer1[i]<91)) {
   buffer0[i] = buffer1[i] + 32;
  }
  else buffer0[i] = buffer1[i];
 }
}

main(int argc, char *argv[])
{

 if (argc<3) {
  write(2,USAGE,strlen(USAGE));
  exit(1);
 }

 struct passwd *urcd = getpwnam("urcd");
 struct sockaddr_un s;

 unsigned char buffer2[1024*2] = {0};
 unsigned char buffer1[1024*2] = {0};
 unsigned char buffer0[1024*2] = {0};
 unsigned char identifiednick[256];
 unsigned char path[512];
 unsigned char hex[192];
 unsigned char pk0[32];
 unsigned char pk1[32];
 unsigned char sk[64];

 float LIMIT;

 long starttime;

 int i = strlen(argv[1]);
 int identifiednicklen;
 int identified = 0;
 int informed = 0;
 int nicklen = 0;
 int sfd = -1;
 int NICKLEN;
 int fd;

 fd = open("env/LIMIT",0);
 if (fd>0)
 {
   if (read(fd,buffer0,1024)>0) LIMIT = atof(buffer0);
   else LIMIT = 1.0;
 } else LIMIT = 1.0;
 close(fd);

 bzero(buffer0,1024);

 fd = open("env/NICKLEN",0);
 if (fd>0)
 {
   if (read(fd,buffer0,1024)>0) NICKLEN = atoi(buffer0) & 255;
   else NICKLEN = 32;
 } else NICKLEN = 32;
 close(fd);

 bzero(&s,sizeof(s));
 s.sun_family = AF_UNIX;
 memcpy(s.sun_path,argv[1],i); /* contains potential overflow */

 if (((sfd=socket(AF_UNIX,SOCK_DGRAM,0))<0)
 || (itoa(s.sun_path+i,getppid(),UNIX_PATH_MAX-i)<0)
 || (connect(sfd,(struct sockaddr *)&s,sizeof(s))<0)
 || (setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i))<0))
 {
  write(2,USAGE,strlen(USAGE));
  exit(2);
 }

 if ((!urcd)
 || (chdir(argv[2]))
 || (chroot(argv[2]))
 || (setgroups(0,'\x00'))
 || (setgid(urcd->pw_gid))
 || (setuid(urcd->pw_uid)))
 {
  write(2,USAGE,strlen(USAGE));
  exit(3);
 }

 starttime = time((long *)0);

 fcntl(0,F_SETFL,fcntl(0,F_GETFL,0)&~O_NONBLOCK);

 memcpy(buffer2+2+12+4+8,":CryptoServ!urc@service PRIVMSG ",32);


 while (1)
 {

  for (i=0;i<1024;++i)
  {
    if (read(0,buffer0+i,1)<1) exit(4);
    if (buffer0[i] == '\r') --i;
    if (buffer0[i] == '\n') break;
  } if (buffer0[i] != '\n') continue;
  ++i;

  lower(buffer1,buffer0,i);

  /// NICK
  if ((i>=7)&&(!memcmp("nick ",buffer1,5))) { /* not reliable */
   nicklen=-5+i-1;
   if (nicklen<=NICKLEN) {
    memcpy(buffer2+2+12+4+8+32,buffer1+5,nicklen);
    memcpy(buffer2+2+12+4+8+32+nicklen," :",2);
   }
   else nicklen = 0;
  } else if (nicklen) {
   if ((i>=20)&&(!memcmp("privmsg cryptoserv :",buffer1,20))) {

    usleep((int)(LIMIT*1000000));

    /// IDENTIFY
    if ((i>=20+9+1+1)&&(!memcmp("identify ",buffer1+20,9))) {
     bzero(path,512);
     memcpy(path,"urcsigndb/",10);
     memcpy(path+10,buffer2+2+12+4+8+32,nicklen);
     if (((fd=open(path,O_RDONLY))<0) || (read(fd,hex,64)<64) || (base16_decode(pk0,hex,64)<32)) {
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"URCSIGN Account does not exist.\n",32);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+32);
      close(fd);
      continue;
     }close(fd);
     crypto_hash_sha512(sk,buffer0+20+9,-20-9+i-1); // hashes everything sans \n
     crypto_sign_keypair(pk1,sk);
     if (memcmp(pk0,pk1,32)) {
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"Invalid passwd.\n",16);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
      continue;
     }
     bzero(path,512);
     memcpy(path,"urccryptoboxdir/",16);
     memcpy(path+16,buffer2+2+12+4+8+32,nicklen);
     if (((fd=open(path,O_RDONLY))<0) || (read(fd,hex,64)<64) || (base16_decode(pk0,hex,64)<32)) {
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"URCCRYPTOBOX Account does not exist.\n",37);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+37);
      close(fd);
      continue;
     }close(fd);
     crypto_scalarmult_curve25519_base(pk1,sk);
     if (memcmp(pk0,pk1,32)) {
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"Invalid passwd.\n",16);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
      continue;
     }
     base16_encode(hex,sk,32);
     base16_encode(hex+64,sk,64);
     memcpy(buffer0,"PASS ",5);
     memcpy(buffer0+5,hex,192);
     memcpy(buffer0+5+192,"\n",1);
     if (write(1,buffer0,5+192+1)<=0) exit(5);
     memcpy(buffer2+2+12+4+8+32+nicklen+2,"success\n",8);
     write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
     memcpy(identifiednick,buffer2+2+12+4+8+32,nicklen);
     identifiednicklen = nicklen;
     identified = 1;
     continue;
    }

    /// REGISTER
    if ((i>=20+9+1+1)&&(!memcmp("register ",buffer1+20,9))) {
     if ((identified) || (time((long *)0)-starttime<128)) {
      goto HELP;
     }
     crypto_hash_sha512(sk,buffer0+20+9,-20-9+i-1); // hashes everything sans \n
     REGISTER:
      crypto_sign_keypair(pk0,sk);
      bzero(path,512);
      memcpy(path,"urcsigndb/",10);
      if (identified) memcpy(path+10,identifiednick,identifiednicklen);
      else memcpy(path+10,buffer2+2+12+4+8+32,nicklen);
      fd = open(path,O_CREAT);
      fchmod(fd,S_IRUSR|S_IWUSR);
      close(fd);
      if ((fd=open(path,O_WRONLY))<0) {
       memcpy(buffer2+2+12+4+8+32+nicklen+2,"failure\n",8);
       write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
       close(fd);
       continue;
      }
      base16_encode(hex,pk0,32);
      if (write(fd,hex,64)<64) exit(6);
      close(fd);
      crypto_scalarmult_curve25519_base(pk0,sk);
      bzero(path,512);
      memcpy(path,"urccryptoboxdir/",16);
      if (identified) memcpy(path+16,identifiednick,identifiednicklen);
      else memcpy(path+16,buffer2+2+12+4+8+32,nicklen);
      fd = open(path,O_CREAT);
      fchmod(fd,S_IRUSR|S_IWUSR);
      close(fd);
      if ((fd=open(path,O_WRONLY))<0) {
       memcpy(buffer2+2+12+4+8+32+nicklen+2,"failure\n",8);
       write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
       close(fd);
       continue;
      }
      base16_encode(hex,pk0,32);
      if (write(fd,hex,64)<64) exit(7);
      close(fd);
      base16_encode(hex,sk,32);
      base16_encode(hex+64,sk,64);
      memcpy(buffer0,"PASS ",5);
      memcpy(buffer0+5,hex,192);
      memcpy(buffer0+5+192,"\n",1);
      if (write(1,buffer0,5+192+1)<=0) exit(8);
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"success\n",8);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
      if (!identified) {
       memcpy(identifiednick,buffer2+2+12+4+8+32,nicklen);
       identifiednicklen = nicklen;
       identified = 1;
      }
    continue;
    }

    /// SET PASSWORD
    if ((i>=20+13+1+1)&&(!memcmp("set password ",buffer1+20,13))) {
     if (!identified) goto HELP;
     crypto_hash_sha512(sk,buffer0+20+13,-20-13+i-1); // hashes everything sans \n
     goto REGISTER;
    }

    /// DROP
    if ((i>=20+4)&&(!memcmp("drop",buffer1+20,4))) {
     if (!identified) goto HELP;
     bzero(path,512);
     memcpy(path,"urccryptoboxdir/",16);
     memcpy(path+16,identifiednick,identifiednicklen);
     if (remove(path)<0) {
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"failure\n",8);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
     }
     bzero(path,512);
     memcpy(path,"urcsigndb/",10);
     memcpy(path+10,identifiednick,identifiednicklen);
     if (remove(path)<0) {
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"failure\n",8);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
      continue;
     }
     memcpy(buffer2+2+12+4+8+32+nicklen+2,"success\n",8);
     write(sfd,buffer2,2+12+4+8+32+nicklen+2+8);
     starttime = time((long *)0);
     identified = 0;
     continue;
    }

    /// HELP
    if ((i>=20+4)&&(!memcmp("help",buffer1+20,4))) {
     HELP:
      informed = 1;
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"Usage:\n",7);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+7);
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"`REGISTER <passwd>' after 128 seconds to create an account.\n",60);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+60);
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"`IDENTIFY <passwd>' to login to your account and activate URCSIGN and URCCRYPTOBOX.\n",84);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+84);
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"`SET PASSWORD <passwd>' changes your password after you REGISTER/IDENTIFY.\n",75);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+75);
      memcpy(buffer2+2+12+4+8+32+nicklen+2,"`DROP' removes your account after you REGISTER/IDENTIFY.\n",57);
      write(sfd,buffer2,2+12+4+8+32+nicklen+2+57);
    }

   if (!informed) goto HELP;
   }
  }
 if (write(1,buffer0,i)<=0) exit(9);
 }
}