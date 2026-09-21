#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  int sockfd;

  // ソケットを作る
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
    close(sockfd); // 確保したリソースはプログラム終了前に片付ける
    return 1;
  }

  // 接続を待ち受ける
  if(listen(sockfd, 10) == -1) {
    close(sockfd);
    return 1;
  }

  // クライアントとの接続用ソケットを作る
  while(1) {
    int clientfd;
    clientfd = accept(sockfd, NULL, NULL);

    if(clientfd == -1) {
      continue;
    }

    // ブラウザからHTTPリクエストを読む
    char buffer[4096];

    ssize_t n = read(clientfd, buffer, sizeof(buffer) - 1);

    if(n <= 0){
      close(clientfd);
      continue;
    }

    buffer[n] = '\0';

    // リクエストターゲットを取り出す
    char path[1024];

    sscanf(buffer, "GET %s HTTP/1.1", path);

    char *question = strchr(path, '?');

    if(question != NULL) {
      *question = '\0';

      char *query = question + 1;

      char *equal = strchr(query, '=');

      if(equal != NULL) {
        *equal = '\0';

        char *name = query;
        char *value = equal + 1;

        // value（1%2B2）をデコードする
        char *encoded = value;
        char decoded[100];

        int i = 0;
        int j = 0;

        while(encoded[i] != '\0') {
          if(encoded[i] == '%') {
            char hex[3];

            hex[0] = encoded[i + 1];
            hex[1] = encoded[i + 2];
            hex[2] = '\0';

            long value = strtol(hex, NULL, 16);

            decoded[j] = (char)value;

            i += 3;
            j++;
          } else {
            decoded[j] = encoded[i];

            i++;
            j++;
          }
        }
        decoded[j] = '\0';

        printf("%s\n", decoded);
      }
    }

    close(clientfd);
  }

  close(sockfd);

  return 0;
}