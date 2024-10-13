#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <sstream>
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <mysql_driver.h>
#include <mysql_connection.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENT_COUNT 3
#define MAX_WORKTHREAD 2
#define PORTNUM 9002
#define MAX_BUF_SIZE 1024

// Operation types for handling different IO requests
enum OP_TYPE { OP_READ, OP_WRITE };

// Buffer structure for IO operations
struct Buffer 
{
    OVERLAPPED overlapped;
    char buffer[MAX_BUF_SIZE];
    WSABUF wsabuf;
    OP_TYPE opType;

    Buffer()
    {
        ZeroMemory(&overlapped, sizeof(OVERLAPPED));
        ZeroMemory(buffer, MAX_BUF_SIZE);  // buffer 배열 초기화
        wsabuf.buf = buffer;
        wsabuf.len = MAX_BUF_SIZE;
        opType = OP_READ;
    }
};

// Constants for handling packet types
#define LOGIN_CHECK 7500
#define SIGN_UP 7502
#define CREATE_CHARACTOR 7504

// Packet processing singleton
#define _Packek Packet::Instance()
// DB Manager singleton
#define _DB_Manager DB_Manager::Instance()
// IOCP server singleton
#define _IOCP_Server IOCP_Server::Instance()
