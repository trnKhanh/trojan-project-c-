#include <cstring>

#include "Response.h"
#include "../Utils/MsgTransport.h"
#include "../Utils/ConvertUtils.h"

Response::Response(uint8_t type, uint32_t errcode, uint64_t length, void *data){
    this->header.type = type;
    this->header.errcode = errcode;
    this->header.length = length;
    if (data != NULL){
        this->data = malloc(length);
        memcpy(this->data, data, length);
    } else this->data = NULL;
}

Response::Response(){
    this->header.type = 0;
    this->header.errcode = 0;
    this->header.length = 0;
    this->data = NULL;
}

Response::~Response(){
    if (this->data != NULL)
        free(this->data);
}

Response::Response(const Response &r){
    this->header.type = r.header.type;
    this->header.errcode = r.header.errcode;
    this->header.length = r.header.length;
    this->data = malloc(this->header.length);
    memcpy(this->data, r.data, this->header.length);
}

Response& Response::operator = (const Response &r){
    this->header.type = r.header.type;
    this->header.errcode = r.header.errcode;
    this->header.length = r.header.length;
    this->data = malloc(this->header.length);
    memcpy(this->data, r.data, this->header.length);
    return *this;
}

uint8_t Response::type(){
    return this->header.type;
}

int sendResponse(int sockfd, const Response &msg, int flag){
    struct {
        uint8_t type;
        uint32_t errcode; 
        uint64_t length;
    } header;
    header.type = msg.header.type;
    header.errcode = htonl(msg.header.errcode);
    header.length = my_htonll(msg.header.length);
    
    int retCode = sendAll(sockfd, &header, sizeof(header), 0);
    if (retCode == -1)
        return -1;
    
    retCode = sendAll(sockfd, msg.data, msg.header.length, 0);
    if (retCode == -1)
        return -1;

    return 0;
}

int recvResponse(int sockfd, Response &msg, int flag){
    if (recvAll(sockfd, &msg.header, sizeof(msg.header), 0) == -1)
        return -1;

    msg.header.errcode = ntohl(msg.header.errcode);
    msg.header.length = my_ntohll(msg.header.length);

    if (msg.data != NULL) free(msg.data);
    msg.data = malloc(msg.header.length);

    int retCode = recvAll(sockfd, msg.data, msg.header.length, 0);
    if (retCode == -1)
        return retCode;

    return 0;
}

int sendtoResponse(int sockfd, const Response &msg, int flag, const sockaddr *addr, socklen_t addrlen){
    struct {
        uint8_t type;
        uint32_t errcode; 
        uint64_t length;
    } header;
    header.type = msg.header.type;
    header.errcode = htonl(msg.header.errcode);
    header.length = my_htonll(msg.header.length);

    int retCode = sendtoAll(sockfd, &header, sizeof(header), 0, addr, addrlen);
    if (retCode == -1)
        return retCode;
    
    retCode = sendtoAll(sockfd, msg.data, msg.header.length, 0, addr, addrlen);
    if (retCode == -1)
        return retCode;

    return 0;
}

int recvfromResponse(int sockfd, Response &msg, int flag, sockaddr *addr, socklen_t *addrlen){
    int retCode = recvfromAll(sockfd, &msg.header, sizeof(msg.header), 0, addr, addrlen);
    if (retCode == -1)
        return retCode;

    msg.header.errcode = ntohl(msg.header.errcode);
    msg.header.length = my_ntohll(msg.header.length);

    if (msg.data != NULL) 
        free(msg.data);
    msg.data = malloc(msg.header.length);

    retCode = recvfromAll(sockfd, msg.data, msg.header.length, 0, addr, addrlen);
    if (retCode == -1)
        return retCode;

    return 0;
}