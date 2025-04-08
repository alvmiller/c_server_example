#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <syslog.h>

#define TAG_NAME "[Client] "
#define PRINT_DEBUG(...) printf(TAG_NAME __VA_ARGS__)
#define PRINT_ERROR(...) fprintf(stderr, TAG_NAME __VA_ARGS__)

#define PORT 4242  // server port to connect to
#define TAG_NAME "[Client] "
#define ERROR " Error: "

//  gcc client.c -o client
// https://www.codequoi.com/en/sockets-and-network-programming-in-c/

static inline int get_client_id(char *str, size_t str_len)
{
	memset(str, 0x00, str_len);

	time_t sec = (time_t)-1;
    if (time(&sec) == (time_t)-1) {
		PRINT_DEBUG("Error: Can`t get time!\n");
		return (-1);
	}
	srand(time(NULL));
    int number = rand();
	int len = snprintf(str, str_len,
					   "ClientID-%ld-%d",
	                   sec, number);
    if (len < 0 || (size_t)len >= str_len) {
		PRINT_ERROR("Error: Bad string!\n");
		return (-1);
    }

	return 0;
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

    PRINT_DEBUG("---- CLIENT ----\n\n");
    struct sockaddr_in sa;
    int socket_fd;
    int status;
    char buffer[BUFSIZ];
    int bytes_read;
    char *msg;
    int msg_len;
    int bytes_sent;

    char client_message_str[200] = {};
	size_t client_message_str_len = sizeof(client_message_str) - 1;
	if (get_client_id(client_message_str, client_message_str_len) != 0) {
		PRINT_ERROR("Error: Can`t create pseudo name!\n");
		return (1);
	}
	PRINT_DEBUG("Current Client pseudo name is : %s\n", client_message_str);

    // Prepare the address and port for the server socket
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET; // IPv4
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1, localhost
    sa.sin_port = htons(PORT);

    // Create socket, connect it to remote server
    socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        PRINT_ERROR("socket fd error: %s\n", strerror(errno));
        return (2);
    }
    PRINT_DEBUG("Created socket fd: %d\n", socket_fd);

    status = connect(socket_fd, (struct sockaddr *)&sa, sizeof sa);
    if (status != 0) {
        PRINT_ERROR("connect error: %s\n", strerror(errno));
        return (3);
    }
    PRINT_DEBUG("Connected socket to localhost port %d\n", PORT);

    // Send a message to server
	msg = client_message_str;
    msg_len = strlen(msg);
    bytes_sent = send(socket_fd, msg, msg_len, 0);
    if (bytes_sent == -1) {
        PRINT_ERROR("send error: %s\n", strerror(errno));
    } else if (bytes_sent == msg_len) {
        PRINT_DEBUG("Sent full message: \"%s\"\n", msg);
    } else {
        PRINT_DEBUG("Sent partial message: %d bytes sent.\n", bytes_sent);
    }

    // Wait for message from server via the socket
    bytes_read = 1;
    while (bytes_read >= 0) {
        bytes_read = recv(socket_fd, buffer, BUFSIZ, 0);
        if (bytes_read == 0) {
            PRINT_DEBUG("Server closed connection.\n");
            break;
        }
        else if (bytes_read == -1) {
            PRINT_ERROR("recv error: %s\n", strerror(errno));
            break;
        }
        else {
            // We got a message, print it
            buffer[bytes_read] = '\0';
            PRINT_DEBUG("Message received: \"%s\"\n", buffer);
            break;
        }
    }

    PRINT_DEBUG("Closing socket\n");
    close(socket_fd);

    return (0);
}