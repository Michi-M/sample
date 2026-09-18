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

  // IPアドレス・ポート番号を設定する
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(8080);

  // ソケットにIPアドレス・ポート番号を割り当てる
  if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    return 1;
  }

  printf("8080番ポートに割り当てました\n");

  close(sockfd);

  return 0;
}