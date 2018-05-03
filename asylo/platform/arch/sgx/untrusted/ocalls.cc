/*
 *
 * Copyright 2018 Asylo authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// Stubs invoked by edger8r generated bridge code for ocalls.

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>

#include "absl/memory/memory.h"
#include "asylo/platform/arch/sgx/untrusted/generated_bridge_u.h"
#include "asylo/platform/common/bridge_proto_serializer.h"
#include "asylo/platform/common/bridge_types.h"
#include "asylo/platform/core/enclave_manager.h"
#include "asylo/platform/core/shared_name.h"

#include "asylo/util/logging.h"

// Threading implementation-defined untrusted thread donate routine.
extern "C" int __asylo_donate_thread(const char *name);

//////////////////////////////////////
//              IO                  //
//////////////////////////////////////

int ocall_enc_untrusted_puts(const char *str) {
  int rc = puts(str);
  // This routine is intended for debugging, so flush immediately to ensure
  // output is written in the event the enclave aborts with buffered output.
  fflush(stdout);
  return rc;
}

void *ocall_enc_untrusted_malloc(bridge_size_t size) {
  void *ret = malloc(static_cast<size_t>(size));
  return ret;
}

int ocall_enc_untrusted_open(const char *path_name, int flags, uint32_t mode) {
  int host_flags = FromBridgeFileFlags(flags);
  int ret = open(path_name, host_flags, mode);
  return ret;
}

int ocall_enc_untrusted_fcntl(int fd, int cmd, int64_t arg) {
  int ret;
  switch (cmd) {
    case F_SETFL:
      ret = fcntl(fd, cmd, FromBridgeFileFlags(arg));
      break;
    case F_SETFD:
      ret = fcntl(fd, cmd, FromBridgeFDFlags(arg));
      break;
    case F_GETFL:
      ret = fcntl(fd, cmd, arg);
      if (ret != -1) {
        ret = ToBridgeFileFlags(ret);
      }
      break;
    case F_GETFD:
      ret = fcntl(fd, cmd, arg);
      if (ret != -1) {
        ret = ToBridgeFDFlags(ret);
      }
      break;
    default:
      return -1;
  }
  return ret;
}

int ocall_enc_untrusted_stat(const char *file, struct bridge_stat *st) {
  struct stat tmp;
  int ret = stat(file, FromBridgeStat(st, &tmp));
  ToBridgeStat(&tmp, st);
  return ret;
}

int ocall_enc_untrusted_fstat(int fd, struct bridge_stat *st) {
  struct stat tmp;
  int ret = fstat(fd, FromBridgeStat(st, &tmp));
  ToBridgeStat(&tmp, st);
  return ret;
}

bridge_ssize_t ocall_enc_untrusted_write_with_untrusted_ptr(int fd,
                                                            const void *buf,
                                                            int size) {
  return static_cast<bridge_ssize_t>(write(fd, buf, size));
}

bridge_ssize_t ocall_enc_untrusted_read_with_untrusted_ptr(int fd, void *buf,
                                                           int size) {
  return static_cast<bridge_ssize_t>(read(fd, buf, size));
}

//////////////////////////////////////
//             Sockets              //
//////////////////////////////////////

int ocall_enc_untrusted_connect(int sockfd, const struct bridge_sockaddr *addr,
                                bridge_size_t addrlen) {
  struct bridge_sockaddr tmp;
  int ret = connect(
      sockfd,
      FromBridgeSockaddr(addr, reinterpret_cast<struct sockaddr *>(&tmp)),
      static_cast<socklen_t>(addrlen));
  return ret;
}

int ocall_enc_untrusted_bind(int sockfd, const struct bridge_sockaddr *addr,
                             bridge_size_t addrlen) {
  struct bridge_sockaddr tmp;
  int ret =
      bind(sockfd,
           FromBridgeSockaddr(addr, reinterpret_cast<struct sockaddr *>(&tmp)),
           static_cast<socklen_t>(addrlen));
  return ret;
}

int ocall_enc_untrusted_accept(int sockfd, struct bridge_sockaddr *addr,
                               bridge_size_t *addrlen) {
  struct sockaddr_storage tmp;
  socklen_t tmp_len = sizeof(tmp);
  int ret = accept(sockfd, reinterpret_cast<struct sockaddr *>(&tmp), &tmp_len);
  if (ret == -1) {
    return ret;
  }
  ToBridgeSockaddr(reinterpret_cast<struct sockaddr *>(&tmp), addr);
  *addrlen = static_cast<bridge_size_t>(tmp_len);
  return ret;
}

bridge_ssize_t ocall_enc_untrusted_sendmsg(int sockfd,
                                           const struct bridge_msghdr *msg,
                                           int flags) {
  struct msghdr tmp;
  if (!FromBridgeMsgHdr(msg, &tmp)) {
    errno = EFAULT;
    return -1;
  }
  auto buf = absl::make_unique<struct iovec[]>(msg->msg_iovlen);
  for (int i = 0; i < msg->msg_iovlen; ++i) {
    if (!FromBridgeIovec(&msg->msg_iov[i], &buf[i])) {
      errno = EFAULT;
      return -1;
    }
  }
  tmp.msg_iov = buf.get();
  bridge_ssize_t ret =
      static_cast<bridge_ssize_t>(sendmsg(sockfd, &tmp, flags));
  return ret;
}

bridge_ssize_t ocall_enc_untrusted_recvmsg(int sockfd,
                                           struct bridge_msghdr *msg,
                                           int flags) {
  struct msghdr tmp;
  if (!FromBridgeMsgHdr(msg, &tmp)) {
    errno = EFAULT;
    return -1;
  }
  auto buf = absl::make_unique<struct iovec[]>(msg->msg_iovlen);
  for (int i = 0; i < msg->msg_iovlen; ++i) {
    if (!FromBridgeIovec(&msg->msg_iov[i], &buf[i])) {
      errno = EFAULT;
      return -1;
    }
  }
  tmp.msg_iov = buf.get();
  bridge_ssize_t ret =
      static_cast<bridge_ssize_t>(recvmsg(sockfd, &tmp, flags));
  if (!ToBridgeIovecArray(&tmp, msg)) {
    errno = EFAULT;
    return -1;
  }
  return ret;
}

char *ocall_enc_untrusted_inet_ntop(int af, const void *src,
                                    bridge_size_t src_size, char *dst,
                                    bridge_size_t buf_size) {
  // src_size is needed so edgr8r copes the correct number of bytes out of the
  // enclave. This suppresses unused variable errors.
  (void)src_size;
  const char *ret = inet_ntop(af, src, dst, static_cast<size_t>(buf_size));
  // edgr8r does not support returning const char*
  return const_cast<char *>(ret);
}

int ocall_enc_untrusted_getaddrinfo(const char *node, const char *service,
                                    const char *serialized_hints,
                                    bridge_size_t serialized_hints_len,
                                    char **serialized_res_start,
                                    bridge_size_t *serialized_res_len) {
  struct addrinfo *hints;
  std::string tmp_serialized_hints(serialized_hints,
                              static_cast<size_t>(serialized_hints_len));
  if (!asylo::DeserializeAddrinfo(&tmp_serialized_hints, &hints)) {
    return -1;
  }

  struct addrinfo *res;
  int ret = getaddrinfo(node, service, hints, &res);
  if (ret != 0) {
    return ret;
  }
  asylo::FreeDeserializedAddrinfo(hints);

  std::string tmp_serialized_res;
  if (!asylo::SerializeAddrinfo(res, &tmp_serialized_res)) {
    return -1;
  }
  freeaddrinfo(res);

  // Allocate memory for the enclave to copy the result; enclave will free this.
  size_t tmp_serialized_res_len = tmp_serialized_res.length();
  char *serialized_res = static_cast<char *>(malloc(tmp_serialized_res_len));
  memcpy(serialized_res, tmp_serialized_res.c_str(), tmp_serialized_res_len);
  *serialized_res_start = serialized_res;
  *serialized_res_len = static_cast<bridge_size_t>(tmp_serialized_res_len);
  return ret;
}

int ocall_enc_untrusted_getsockopt(int sockfd, int level, int optname,
                                   void *optval, unsigned int optlen_in,
                                   unsigned int *optlen_out) {
  int ret = getsockopt(sockfd, level, optname, optval,
                       reinterpret_cast<socklen_t *>(&optlen_in));
  *optlen_out = optlen_in;
  return ret;
}

int ocall_enc_untrusted_getsockname(int sockfd, struct bridge_sockaddr *addr,
                                    bridge_size_t *addrlen) {
  struct sockaddr_storage tmp;
  socklen_t tmp_len = sizeof(tmp);
  int ret =
      getsockname(sockfd, reinterpret_cast<struct sockaddr *>(&tmp), &tmp_len);
  ToBridgeSockaddr(reinterpret_cast<struct sockaddr *>(&tmp), addr);
  *addrlen = static_cast<bridge_size_t>(tmp_len);
  return ret;
}

//////////////////////////////////////
//           poll.h                 //
//////////////////////////////////////

int ocall_enc_untrusted_poll(struct bridge_pollfd *fds, unsigned int nfds,
                             int timeout) {
  auto tmp = absl::make_unique<pollfd[]>(nfds);
  for (int i = 0; i < nfds; ++i) {
    if (!FromBridgePollfd(&fds[i], &tmp[i])) {
      errno = EFAULT;
      return -1;
    }
  }
  int ret = poll(tmp.get(), nfds, timeout);
  for (int i = 0; i < nfds; ++i) {
    if (!ToBridgePollfd(&tmp[i], &fds[i])) {
      errno = EFAULT;
      return -1;
    }
  }
  return ret;
}

//////////////////////////////////////
//           sched.h                //
//////////////////////////////////////

int ocall_enc_untrusted_sched_getaffinity(int64_t pid, BridgeCpuSet *mask) {
  cpu_set_t host_mask;
  if (BRIDGE_CPU_SET_MAX_CPUS != CPU_SETSIZE) {
    LOG(ERROR) << "sched_getaffinity: CPU_SETSIZE (" << CPU_SETSIZE
               << ") is not equal to " << BRIDGE_CPU_SET_MAX_CPUS;
    errno = ENOSYS;
    return -1;
  }

  int ret =
      sched_getaffinity(static_cast<pid_t>(pid), sizeof(cpu_set_t), &host_mask);

  // Translate from host cpu_set_t to bridge_cpu_set_t.
  int total_cpus = CPU_COUNT(&host_mask);
  BridgeCpuSetZero(mask);
  for (int cpu = 0, cpus_so_far = 0; cpus_so_far < total_cpus; ++cpu) {
    if (CPU_ISSET(cpu, &host_mask)) {
      BridgeCpuSetAddBit(cpu, mask);
      ++cpus_so_far;
    }
  }

  return ret;
}

//////////////////////////////////////
//           time.h                 //
//////////////////////////////////////

int ocall_enc_untrusted_nanosleep(const struct bridge_timespec *req,
                                  struct bridge_timespec *rem) {
  int ret = nanosleep(reinterpret_cast<const struct timespec *>(req),
                      reinterpret_cast<struct timespec *>(rem));
  return ret;
}

int ocall_enc_untrusted_clock_gettime(bridge_clockid_t clk_id,
                                      struct bridge_timespec *tp) {
  int ret = clock_gettime(static_cast<clockid_t>(clk_id),
                          reinterpret_cast<struct timespec *>(tp));
  return ret;
}

//////////////////////////////////////
//           sys/time.h             //
//////////////////////////////////////

int ocall_enc_untrusted_gettimeofday(struct bridge_timeval *tv, void *tz) {
  return gettimeofday(reinterpret_cast<struct timeval *>(tv), nullptr);
}

//////////////////////////////////////
//            unistd.h              //
//////////////////////////////////////

int ocall_enc_untrusted_pipe(int pipefd[2]) {
  int ret = pipe(pipefd);
  return ret;
}

int ocall_enc_untrusted_sysconf(enum SysconfConstants name) {
  int tmp = FromSysconfConstants(name);
  int ret = -1;
  if (tmp != -1) {
    ret = sysconf(tmp);
  }
  return ret;
}

uint32_t ocall_enc_untrusted_sleep(uint32_t seconds) { return sleep(seconds); }

//////////////////////////////////////
//           Runtime support        //
//////////////////////////////////////

void *ocall_enc_untrusted_acquire_shared_resource(SharedNameKind kind,
                                                  const char *name) {
  asylo::SharedName shared_name(kind, std::string(name));
  auto manager_result = asylo::EnclaveManager::Instance();
  if (manager_result.ok()) {
    return manager_result.ValueOrDie()
        ->shared_resources()
        ->AcquireResource<void>(shared_name);
  } else {
    return nullptr;
  }
}

int ocall_enc_untrusted_release_shared_resource(SharedNameKind kind,
                                                const char *name) {
  asylo::SharedName shared_name(kind, std::string(name));
  auto manager_result = asylo::EnclaveManager::Instance();
  if (manager_result.ok()) {
    return manager_result.ValueOrDie()->shared_resources()->ReleaseResource(
        shared_name);
  }
  return false;
}

//////////////////////////////////////
//           Debugging              //
//////////////////////////////////////

void ocall_enc_untrusted_hex_dump(const void *buf, int nbytes) {
  const char *x = reinterpret_cast<const char *>(buf);
  for (int i = 0; i < nbytes; i++) {
    fprintf(stderr, "%x:", x[i]);
  }
  fprintf(stderr, "\n");
}

//////////////////////////////////////
//           Threading              //
//////////////////////////////////////

int ocall_enc_untrusted_thread_create(const char *name) {
  __asylo_donate_thread(name);
  return 0;
}