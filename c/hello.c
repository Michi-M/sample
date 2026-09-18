#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(void) {
  int sockfd;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd == -1) {
    return 1;
  }

  printf("ソケットを作成しました: %d\n", sockfd);

  close(sockfd);

  printf("ソケットを閉じました\n");

return 0;
}