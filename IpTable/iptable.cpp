// IpTable.cpp : 定义控制台应用程序的入口点。
#include "stdafx.h"

#include <Winsock2.h>
#include <Iphlpapi.h>
#include <psapi.h>
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")
#include "iptable.h"


void TcpTable::flushTable() {
    initTable();
}

const bool TcpTable::initTable() {
    vct_tcptable.clear();
    //get size
    DWORD tcptablesize = 0;
    GetExtendedTcpTable( NULL, &tcptablesize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0 );
    //get info
    PMIB_TCPTABLE_OWNER_PID ptowp = (PMIB_TCPTABLE_OWNER_PID)new byte[tcptablesize];
    GetExtendedTcpTable( ptowp, &tcptablesize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0 );

    //loop all info
    std::vector<TcpTableStruct> *v_tcptable = &vct_tcptable;
    for( int i = 0; i < ptowp->dwNumEntries; i++ ) {
        IN_ADDR localAddr, remoteAddr;
        TcpTableStruct tcptable = {0};
        tcptable.pid = ptowp->table[i].dwOwningPid;
        localAddr.s_addr = ptowp->table[i].dwLocalAddr;
        strcpy( tcptable.localip, inet_ntoa( localAddr ) );
        tcptable.localport = htons( ptowp->table[i].dwLocalPort );
        remoteAddr.s_addr = ptowp->table[i].dwRemoteAddr;
        strcpy( tcptable.remoteip, inet_ntoa( remoteAddr ) );
        tcptable.remoteport = htons( ptowp->table[i].dwRemotePort );
        HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, tcptable.pid );
        if( hProcess ) {
            GetModuleFileNameExA( hProcess, NULL, tcptable.dir, MAX_PATH );
            for( int j = strlen( tcptable.dir ); j > 0; j-- ) {
                if( tcptable.dir[j] == '\\' ) {
                    strcpy( tcptable.name, tcptable.dir + j + 1 );
                    tcptable.dir[j + 1] = 0;
                    break;
                }
            }
        }
        v_tcptable->push_back( tcptable );
    }
    delete ptowp;
    return true;
}

std::vector<TcpTable::TcpTableStruct>& TcpTable::getTableVector() {
    return vct_tcptable;
}
