#pragma once
#include <vector>

class TcpTable
{
public:
    TcpTable() {
        initTable();
    }
    ~TcpTable() {
        vct_tcptable.clear();
    }

    //others
    struct TcpTableStruct
    {
        UINT pid;
        char name[60];
        char dir[260];

        char localip[16];
        UINT localport;
        char remoteip[16];
        UINT remoteport;
    };

    void flushTable();
    const bool initTable();
    std::vector<TcpTableStruct>& getTableVector();

private:
    std::vector<TcpTableStruct> vct_tcptable;
};
