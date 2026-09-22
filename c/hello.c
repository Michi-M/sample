#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/*
 * 16進数かどうかを判定する
 *
 * 例:
 * '0' ～ '9'
 * 'A' ～ 'F'
 * 'a' ～ 'f'
 */
int is_hex(char c)
{
    return
        ('0' <= c && c <= '9') ||
        ('A' <= c && c <= 'F') ||
        ('a' <= c && c <= 'f');
}


/*
 * 16進数1文字を数値に変換する
 *
 * 例:
 * '0' -> 0
 * '9' -> 9
 * 'A' -> 10
 * 'F' -> 15
 */
int hex_to_int(char c)
{
    if ('0' <= c && c <= '9') {
        return c - '0';
    }

    if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }

    if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    }

    return -1;
}


/*
 * URLの%XX形式をデコードする
 *
 * 例:
 * "1%2B2" -> "1+2"
 *
 * 成功: 0
 * 失敗: -1
 */
int url_decode(
    const char *encoded,
    char *decoded,
    size_t decoded_size
)
{
    size_t i = 0;
    size_t j = 0;

    while (encoded[i] != '\0') {

        // decodedの容量を超えないか確認
        if (j >= decoded_size - 1) {
            return -1;
        }

        /*
         * %XX の形式
         */
        if (encoded[i] == '%') {

            // % の後ろに2文字あるか確認
            if (encoded[i + 1] == '\0' ||
                encoded[i + 2] == '\0') {
                return -1;
            }

            // 2文字とも16進数か確認
            if (!is_hex(encoded[i + 1]) ||
                !is_hex(encoded[i + 2])) {
                return -1;
            }

            int high = hex_to_int(encoded[i + 1]);
            int low = hex_to_int(encoded[i + 2]);

            if (high == -1 || low == -1) {
                return -1;
            }

            /*
             * 例えば %2B の場合
             *
             * 2 -> 2
             * B -> 11
             *
             * 2 * 16 + 11 = 43
             *
             * 43 はASCIIの '+'
             */
            decoded[j] = (char)(high * 16 + low);

            i += 3;
            j++;

        } else {

            // 通常の文字
            decoded[j] = encoded[i];

            i++;
            j++;
        }
    }

    decoded[j] = '\0';

    return 0;
}


/*
 * clientfdに指定したバイト数をすべて送信する
 *
 * 成功: 0
 * 失敗: -1
 */
int write_all(
    int clientfd,
    const char *data,
    size_t length
)
{
    size_t total = 0;

    while (total < length) {

        ssize_t n = write(
            clientfd,
            data + total,
            length - total
        );

        if (n == -1) {

            /*
             * シグナルによって中断された場合は
             * もう一度writeする
             */
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        /*
         * 通常write()は0にならないが、
         * 念のため無限ループを防ぐ
         */
        if (n == 0) {
            return -1;
        }

        total += (size_t)n;
    }

    return 0;
}


/*
 * HTTPエラーを返す
 */
void send_error(
    int clientfd,
    const char *status,
    const char *message
)
{
    char body[256];

    int body_len = snprintf(
        body,
        sizeof(body),
        "%s\n",
        message
    );

    if (body_len < 0 ||
        (size_t)body_len >= sizeof(body)) {
        return;
    }

    char response[512];

    int response_len = snprintf(
        response,
        sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status,
        body_len,
        body
    );

    if (response_len < 0 ||
        (size_t)response_len >= sizeof(response)) {
        return;
    }

    /*
     * エラーを送信できなくても、
     * ここではさらにエラー処理をする必要はない。
     *
     * この後clientfdは呼び出し側でcloseする。
     */
    write_all(
        clientfd,
        response,
        (size_t)response_len
    );
}


int main(void)
{
    int sockfd;

    /*
     * --------------------------------------------------
     * 1. ソケットを作る
     * --------------------------------------------------
     */
    sockfd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (sockfd == -1) {
        perror("socket");
        return 1;
    }


    /*
     * --------------------------------------------------
     * 2. IPアドレス・ポート番号を設定する
     * --------------------------------------------------
     */
    struct sockaddr_in addr;

    /*
     * 構造体を0で初期化
     *
     * 未初期化領域が残らないようにする
     */
    memset(
        &addr,
        0,
        sizeof(addr)
    );

    addr.sin_family = AF_INET;

    /*
     * すべてのローカルIPv4インターフェースで
     * 接続を受け付ける
     */
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*
     * ポート8080
     */
    addr.sin_port = htons(8080);


    /*
     * --------------------------------------------------
     * 3. ソケットにIPアドレス・ポート番号を割り当てる
     * --------------------------------------------------
     */
    if (bind(
        sockfd,
        (struct sockaddr *)&addr,
        sizeof(addr)
    ) == -1) {

        perror("bind");

        close(sockfd);

        return 1;
    }


    /*
     * --------------------------------------------------
     * 4. 接続を待ち受ける
     * --------------------------------------------------
     */
    if (listen(sockfd, 10) == -1) {

        perror("listen");

        close(sockfd);

        return 1;
    }


    printf("Server started on port 8080.\n");


    /*
     * --------------------------------------------------
     * 5. クライアントからの接続を繰り返し受け付ける
     * --------------------------------------------------
     */
    while (1) {

        int clientfd;

        clientfd = accept(
            sockfd,
            NULL,
            NULL
        );

        if (clientfd == -1) {

            /*
             * シグナルによってaccept()が
             * 中断された場合
             */
            if (errno == EINTR) {
                continue;
            }

            perror("accept");

            /*
             * accept()失敗でサーバー全体を
             * 終了する必要はないので、
             * 次の接続を待つ
             */
            continue;
        }


        /*
         * --------------------------------------------------
         * 6. HTTPリクエストを読む
         * --------------------------------------------------
         */
        char buffer[4096];

        ssize_t n = read(
            clientfd,
            buffer,
            sizeof(buffer) - 1
        );

        if (n == -1) {

            if (errno == EINTR) {
                close(clientfd);
                continue;
            }

            perror("read");

            close(clientfd);

            continue;
        }

        /*
         * クライアントが接続を閉じた
         */
        if (n == 0) {

            close(clientfd);

            continue;
        }

        /*
         * C文字列として扱えるようにする
         */
        buffer[n] = '\0';


        /*
         * --------------------------------------------------
         * 7. HTTPリクエストからpathを取り出す
         * --------------------------------------------------
         *
         * 例:
         *
         * GET //calc?q=1%2B2 HTTP/1.1
         *
         * ↓
         *
         * path = "//calc?q=1%2B2"
         */
        char path[1024];

        int scanned = sscanf(
            buffer,
            "GET %1023s HTTP/1.1",
            path
        );

        if (scanned != 1) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Bad Request"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 8. pathを確認
         * --------------------------------------------------
         *
         * 今回は /calc と //calc を許可する。
         */
        if (strcmp(path, "/calc") != 0 &&
            strcmp(path, "//calc") != 0) {

            send_error(
                clientfd,
                "404 Not Found",
                "Not Found"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 9. ? を探してqueryを分離する
         * --------------------------------------------------
         *
         * //calc?q=1%2B2
         *
         * ↓
         *
         * path  = "//calc"
         * query = "q=1%2B2"
         */
        char *question = strchr(
            path,
            '?'
        );

        /*
         * ? がなければ計算できない
         */
        if (question == NULL) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Missing query"
            );

            close(clientfd);

            continue;
        }

        /*
         * ? を \0 に置き換える
         */
        *question = '\0';

        /*
         * ? の次の文字からquery
         */
        char *query = question + 1;


        /*
         * --------------------------------------------------
         * 10. = を探す
         * --------------------------------------------------
         *
         * q=1%2B2
         *
         * ↓
         *
         * name  = "q"
         * value = "1%2B2"
         */
        char *equal = strchr(
            query,
            '='
        );

        if (equal == NULL) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Missing parameter"
            );

            close(clientfd);

            continue;
        }

        /*
         * = を \0 に置き換える
         */
        *equal = '\0';

        char *name = query;
        char *value = equal + 1;


        /*
         * パラメータ名を確認
         */
        if (strcmp(name, "q") != 0) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Invalid parameter"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 11. URLデコード
         * --------------------------------------------------
         *
         * "1%2B2"
         *
         * ↓
         *
         * "1+2"
         */
        char decoded[100];

        if (url_decode(
            value,
            decoded,
            sizeof(decoded)
        ) == -1) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Invalid URL encoding"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 12. 「数字+数字」の形式か確認する
         * --------------------------------------------------
         */
        char *plus = strchr(
            decoded,
            '+'
        );

        if (plus == NULL) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Invalid expression"
            );

            close(clientfd);

            continue;
        }


        /*
         * + を \0 に置き換える
         *
         * 例:
         *
         * "1+2"
         *
         * ↓
         *
         * "1"
         * "2"
         */
        *plus = '\0';

        char *left = decoded;
        char *right = plus + 1;


        /*
         * 空文字を拒否
         */
        if (*left == '\0' ||
            *right == '\0') {

            send_error(
                clientfd,
                "400 Bad Request",
                "Invalid expression"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 13. 数値に変換する
         * --------------------------------------------------
         */
        char *endptr;

        errno = 0;

        long a = strtol(
            left,
            &endptr,
            10
        );

        /*
         * 数値の後ろに余計な文字がないか確認
         */
        if (*endptr != '\0') {

            send_error(
                clientfd,
                "400 Bad Request",
                "Invalid number"
            );

            close(clientfd);

            continue;
        }

        /*
         * 範囲外チェック
         */
        if (errno == ERANGE ||
            a < INT_MIN ||
            a > INT_MAX) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Number out of range"
            );

            close(clientfd);

            continue;
        }


        /*
         * 右側の数字
         */
        errno = 0;

        long b = strtol(
            right,
            &endptr,
            10
        );

        if (*endptr != '\0') {

            send_error(
                clientfd,
                "400 Bad Request",
                "Invalid number"
            );

            close(clientfd);

            continue;
        }

        if (errno == ERANGE ||
            b < INT_MIN ||
            b > INT_MAX) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Number out of range"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 14. 足し算のオーバーフローを確認
         * --------------------------------------------------
         */
        if ((b > 0 && a > INT_MAX - b) ||
            (b < 0 && a < INT_MIN - b)) {

            send_error(
                clientfd,
                "400 Bad Request",
                "Result out of range"
            );

            close(clientfd);

            continue;
        }

        int result = (int)(a + b);


        /*
         * --------------------------------------------------
         * 15. 計算結果を文字列にする
         * --------------------------------------------------
         */
        char body[100];

        int body_len = snprintf(
            body,
            sizeof(body),
            "%d",
            result
        );

        if (body_len < 0 ||
            (size_t)body_len >= sizeof(body)) {

            send_error(
                clientfd,
                "500 Internal Server Error",
                "Internal Server Error"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 16. HTTPレスポンスを作る
         * --------------------------------------------------
         */
        char response[1024];

        int response_len = snprintf(
            response,
            sizeof(response),

            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",

            body_len,
            body
        );

        if (response_len < 0 ||
            (size_t)response_len >= sizeof(response)) {

            send_error(
                clientfd,
                "500 Internal Server Error",
                "Internal Server Error"
            );

            close(clientfd);

            continue;
        }


        /*
         * --------------------------------------------------
         * 17. ブラウザに送信する
         * --------------------------------------------------
         */
        if (write_all(
            clientfd,
            response,
            (size_t)response_len
        ) == -1) {

            perror("write");
        }


        /*
         * --------------------------------------------------
         * 18. クライアントとの接続を閉じる
         * --------------------------------------------------
         */
        close(clientfd);
    }


    /*
     * 現在のwhile(1)では通常ここには到達しない。
     *
     * サーバー終了処理を追加した場合に備えて残している。
     */
    close(sockfd);

    return 0;
}