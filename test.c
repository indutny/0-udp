#include <arpa/inet.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define TEST_ADDR "127.0.0.1"
#define TEST_PORT 4242

static struct {
  int fd;
  struct sockaddr_in addr;
} server;

void server_listen() {
  int r;

  server.fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  assert(server.fd != -1);

  memset(&server.addr, 0, sizeof(server.addr));
  server.addr.sin_family = AF_INET;
  server.addr.sin_port = htons(TEST_PORT);

  r = inet_pton(AF_INET, TEST_ADDR, &server.addr.sin_addr);
  assert(r == 1);

  r = bind(server.fd, (struct sockaddr*) &server.addr, sizeof(server.addr));
  assert(r == 0);
}

void send_message() {
  int fd;
  int r;
  struct msghdr msg;
  struct iovec iov;

  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  assert(fd != -1);

  memset(&msg, 0, sizeof(msg));

  msg.msg_name = &server.addr;
  msg.msg_namelen = sizeof(server.addr);

  iov.iov_base = "";
  iov.iov_len = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  r = sendmsg(fd, &msg, 0);
  assert(r == (int) iov.iov_len);

  close(fd);
}

void server_poll() {
  int kq;
  struct kevent event;
  int r;

  kq = kqueue();
  assert(kq != -1);

  /* Add and poll */
  EV_SET(&event, server.fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  r = kevent(kq, &event, 1, &event, 1, NULL);
  assert(r == 1);

  close(kq);
}

void cleanup() {
  close(server.fd);
}

int main() {
  fprintf(stderr, "Starting...\n");

  /* Listen on UDP port */
  server_listen();

  fprintf(stderr, "Server bound to [%s]:%d\n", TEST_ADDR, TEST_PORT);

  /* Send 0-UDP packet to that port */
  send_message();

  fprintf(stderr, "Sent message to server\n");

  /* Poll kqueue events */
  server_poll();

  fprintf(stderr, "Got kqueue event\n");

  cleanup();

  return 0;
}
