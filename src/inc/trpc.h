/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TDENGINE_TRPC_H
#define TDENGINE_TRPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "taosdef.h"

#define TAOS_CONN_SERVER   0
#define TAOS_CONN_CLIENT   1

extern int tsRpcHeadSize;

typedef struct SRpcIpSet {
  int8_t    inUse; 
  int8_t    numOfIps;
  uint16_t  port[TSDB_MAX_REPLICA];
  char      fqdn[TSDB_MAX_REPLICA][TSDB_FQDN_LEN];
} SRpcIpSet;

typedef struct SRpcConnInfo {
  uint32_t  clientIp;
  uint16_t  clientPort;
  uint32_t  serverIp;
  char      user[TSDB_USER_LEN];
} SRpcConnInfo;

typedef struct SRpcMsg {
  uint8_t msgType;
  void   *pCont;
  int     contLen;
  int32_t code;
  void   *handle;
  void   *ahandle;  //app handle set by client, for debug purpose
} SRpcMsg;

typedef struct SRpcInit {
  uint16_t localPort; // local port
  char  *label;        // for debug purpose
  int    numOfThreads; // number of threads to handle connections
  int    sessions;     // number of sessions allowed
  int8_t connType;     // TAOS_CONN_UDP, TAOS_CONN_TCPC, TAOS_CONN_TCPS
  int    idleTime;     // milliseconds, 0 means idle timer is disabled

  // the following is for client app ecurity only
  char *user;         // user name
  char  spi;          // security parameter index
  char  encrypt;      // encrypt algorithm
  char *secret;       // key for authentication
  char *ckey;         // ciphering key

  // call back to process incoming msg, code shall be ignored by server app
  void (*cfp)(SRpcMsg *, SRpcIpSet *);  

  // call back to retrieve the client auth info, for server app only 
  int  (*afp)(char *tableId, char *spi, char *encrypt, char *secret, char *ckey);
} SRpcInit;

void *rpcOpen(const SRpcInit *pRpc);
void  rpcClose(void *);
void *rpcMallocCont(int contLen);
void  rpcFreeCont(void *pCont);
void *rpcReallocCont(void *ptr, int contLen);
void  rpcSendRequest(void *thandle, const SRpcIpSet *pIpSet, const SRpcMsg *pMsg);
void  rpcSendResponse(const SRpcMsg *pMsg);
void  rpcSendRedirectRsp(void *pConn, const SRpcIpSet *pIpSet); 
int   rpcGetConnInfo(void *thandle, SRpcConnInfo *pInfo);
void  rpcSendRecv(void *shandle, SRpcIpSet *pIpSet, const SRpcMsg *pReq, SRpcMsg *pRsp);
void  rpcReportProgress(void *pConn, char *pCont, int contLen);

#ifdef __cplusplus
}
#endif

#endif  // TDENGINE_TRPC_H
