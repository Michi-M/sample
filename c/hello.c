#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(void) {
  int sockfd;

  // ソケットを作る
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd == -1) {
    perror("socket");
    return 1;
  }

  // IPアドレス・ポート番号を設定する
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(8080);

  // ソケットにIPアドレス・ポート番号を割り当てる
  if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    close(sockfd); // 確保したリソースはプログラム終了前に片付ける
    return 1;
  }

  // 接続を待ち受ける
  if(listen(sockfd, 10) == -1) {
    perror("listen");
    close(sockfd);
    return 1;
  }

  // クライアントとの接続用ソケットを作る
  while(1) {
    int clientfd;
    clientfd = accept(sockfd, NULL, NULL);

    if(clientfd == -1) {
      perror("accept");
      continue;
    }

    // ブラウザからHTTPリクエストを読む
    char buffer[4096];

    ssize_t n = read(clientfd, buffer, sizeof(buffer) - 1);

    if(n == -1){
      perror("read");
      close(clientfd);
      continue;
    }

    if(n == 0) {
      close(clientfd);
      continue;
    }

    buffer[n] = '\0';

    // リクエストターゲットを取り出す
    char path[1024];

    if(sscanf(buffer, "GET %s HTTP/1.1", path) != 1){
      close(clientfd);
      continue;
    };

    char *question = strchr(path, '?');

    if(question != NULL) {
      *question = '\0';

      char *query = question + 1;

      char *equal = strchr(query, '=');

      if(equal != NULL) {
        *equal = '\0';

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

        // 計算する
        int a;
        int b;

        if(sscanf(decoded, "%d+%d", &a, &b) == 2){
          int result = a + b;

          // 計算結果を文字列にする
          char body[100];
          snprintf(body, sizeof(body), "%d", result);

          // レスポンスを作る
          char response[1024];

          snprintf(
            response,
            sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            strlen(body),
            body
          );

          // ブラウザに送る
          write(clientfd, response, strlen(response));
        }
      }
    }

    close(clientfd);
  }

  close(sockfd);

  return 0;
}