
#include "stdafx.h"

#include <windows.h>
#include <commctrl.h>
#include <algorithm>
#include "resource.h"
#include "iptable.h"
#include "getip.h"
#pragma  comment(lib,   "comctl32.lib ")



//
//common
//
inline void FsListView_InsertColum( HWND hwnd, PCH str, int width ) {
    LV_COLUMNA colmn = {0};
    colmn.mask = LVCF_WIDTH | LVCF_TEXT;
    colmn.pszText = str;
    colmn.cx = width;
    SendMessageA( (hwnd), LVM_INSERTCOLUMNA, (WPARAM)(int)(0), (LPARAM)(const LV_COLUMN *)(&colmn) );
}
inline void FsListView_SetItemText( HWND hwndLV, int i, int iSubItem_, PCH pszText_ ) {
    LV_ITEMA _macro_lvi;
    _macro_lvi.iSubItem = (iSubItem_);
    _macro_lvi.pszText = (pszText_);
    SendMessageA( hwndLV, LVM_SETITEMTEXTA, (WPARAM)(i), (LPARAM)(LV_ITEM *)&_macro_lvi );
}
inline void selectFile(PCH filename) {
    //open folder and select file
    char szParam[_MAX_PATH] = {0};
    strcpy( szParam, "/e,/select, " );
    strcat( szParam, filename );
    ShellExecuteA( NULL, "open", "explorer", szParam, NULL, SW_SHOW );
}
inline bool ListView_GetSelectStr( HWND hList, int isubItem, OUT PCH retbuf, IN int retbuflen ) {
    int sfind = ListView_GetSelectionMark( hList );
    if( sfind != -1 ) {
        LV_ITEMA _macro_lvi; 
        _macro_lvi.iSubItem = isubItem;
        _macro_lvi.cchTextMax = retbuflen;
        _macro_lvi.pszText = retbuf;
        SendMessageA( hList, LVM_GETITEMTEXTA, (WPARAM)sfind, (LPARAM)(LV_ITEM *)&_macro_lvi );
        return true;
    }
    return false;
}


//
//others
//
HINSTANCE hinst;
HWND MainWnd;
#define IP_DATA_NAME "qqwry.dat"          //纯真数据库
#define hMainList GetDlgItem( MainWnd, IDC_LIST1 )

int lastcolumnclick = -1;                 //记录上次点击的列头
bool blastcolumnclick = false;            //用于判断是否应该翻转排序

void FillOnce( TcpTable::TcpTableStruct& v ) {
    LV_ITEMA item = {0};
    int cis = 0;
    char tmp[10];

    item.mask = LVIF_TEXT | LVIF_IMAGE;
    wsprintfA( tmp, "%04d", v.pid );
    item.pszText = tmp;
    item.iItem = cis;
    cis = SendMessageA( hMainList, LVM_INSERTITEMA, 0, (LPARAM)(const LV_ITEM *)(&item) );

    //1 name
    if( v.name[0] == 0 )
        strcpy( v.name, "[ no access ]" );
    FsListView_SetItemText( hMainList, cis, 1, v.name );
    //2 address
    //直接用纯真库查询
    CIpFinder ipfinder;
    if( ipfinder.Open( IP_DATA_NAME ) ) {
        std::string country, location;
        ipfinder.GetAddressByIp( v.remoteip, country, location );
        country += " ";
        country += location;
        FsListView_SetItemText( hMainList, cis, 2, (PCH)country.c_str() );
    }
    //3 ip
    FsListView_SetItemText( hMainList, cis, 3, v.remoteip );
    //4 port
    wsprintfA( tmp, "%d", v.remoteport );
    FsListView_SetItemText( hMainList, cis, 4, tmp );
    //5 path
    FsListView_SetItemText( hMainList, cis, 5, v.dir );
}

void FillInfo( int icloumsort = -1 ) {

    std::vector<TcpTable::TcpTableStruct> curshow;
    auto findpid = [&]( DWORD pid )->bool {
        for( auto v : curshow ) {
            if( v.pid == pid )
                return true;
        }
        return false;
    };
    auto isok = []( TcpTable::TcpTableStruct v )->bool {
        return v.remoteport && (strstr( v.remoteip, "127.0.0.1" ) == 0);
    };

    TcpTable tt;
    //TcpTable排序
    if( icloumsort != -1 ) {
        if( lastcolumnclick == icloumsort )
            blastcolumnclick = !blastcolumnclick;
        lastcolumnclick = icloumsort;
    } else {
        //last time choice
        icloumsort = lastcolumnclick;
    }

    switch( icloumsort ) {
        case 0:
            sort( tt.getTableVector().begin(), tt.getTableVector().end(),
                [&]( TcpTable::TcpTableStruct x, TcpTable::TcpTableStruct y )->bool {
                if( blastcolumnclick )
                    return x.pid < y.pid;     //第二次点击 降序
                return x.pid > y.pid;         //第一次点击 升序
            } );
            break;
        case 1:
            sort( tt.getTableVector().begin(), tt.getTableVector().end(),
                [&]( TcpTable::TcpTableStruct x, TcpTable::TcpTableStruct y )->bool {
                if( blastcolumnclick )
                    return strcmp( x.name, y.name ) < 0;
                return strcmp( x.name, y.name ) > 0;
            } );
            break;
        case 2:             //do nothing
            break;
        case 3:
            sort( tt.getTableVector().begin(), tt.getTableVector().end(),
                [&]( TcpTable::TcpTableStruct x, TcpTable::TcpTableStruct y )->bool {
                if( blastcolumnclick )
                    return strcmp( x.remoteip, y.remoteip ) < 0;
                return strcmp( x.remoteip, y.remoteip ) > 0;
            } );
            break;
        case 4:
            sort( tt.getTableVector().begin(), tt.getTableVector().end(),
                [&]( TcpTable::TcpTableStruct x, TcpTable::TcpTableStruct y )->bool {
                if( blastcolumnclick )
                    return x.remoteport < y.remoteport;     //第二次点击 降序
                return x.remoteport > y.remoteport;         //第一次点击 升序
            } );
            break;
        case 5:
            sort( tt.getTableVector().begin(), tt.getTableVector().end(),
                [&]( TcpTable::TcpTableStruct x, TcpTable::TcpTableStruct y )->bool {
                if( blastcolumnclick )
                    return strcmp( x.dir, y.dir ) < 0;
                return strcmp( x.dir, y.dir ) > 0;
            } );
            break;
    }
    ListView_DeleteAllItems( hMainList );
    curshow.clear();

    //b_unique_pid
    if( IsDlgButtonChecked( MainWnd, IDC_CHECK1 ) ) {
        for( auto v : tt.getTableVector() ) {
            if( !findpid( v.pid ) ) {
                if( isok( v ) ) {
                    curshow.push_back( v );
                    FillOnce( v );
                }
            }
        }
    } else {
        for( auto v : tt.getTableVector() ) {
            if( isok( v ) )
                FillOnce( v );
        }
    }
}

bool selectFileByMainListChoice( ) {
    char path[260], name[100];
    ListView_GetSelectStr( hMainList, 5, path, 260 );
    if( path[0] ) {
        ListView_GetSelectStr( hMainList, 1, name, 100 );
        strcat( path, name );
        selectFile( path );
        return true;
    }
    return false;
}

void terminateSelectProcess() {
    char spid[10];
    ListView_GetSelectStr( hMainList, 0, spid, 10 );
    char cmd[100];
    wsprintfA( cmd, "taskkill /f /pid %s", spid );
    system( cmd );
}

//
//运行框Login
//
BOOL CALLBACK MainWindowProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch( uMsg ) {
        case WM_INITDIALOG:
            MainWnd = hDlg;
            ListView_SetExtendedListViewStyleEx( hMainList, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES  );               //style ex
            ListView_SetImageList( hMainList, ImageList_Create( 1, 16, ILC_COLOR24 | ILC_MASK, 1, 1 ), LVSIL_SMALL );	//改格子高度
            FsListView_InsertColum( hMainList, "所在路径", 360 );
            FsListView_InsertColum( hMainList, "端口", 60 );
            FsListView_InsertColum( hMainList, "远程IP", 120 );
            FsListView_InsertColum( hMainList, "远程地址", 180 );
            FsListView_InsertColum( hMainList, "名称", 120 );
            FsListView_InsertColum( hMainList, "PID", 60 );
            FillInfo();
            break;

        case WM_COMMAND:
            switch( LOWORD( wParam ) ) {
                case IDC_BUTTON1:
                    FillInfo();
                    break;

                //menu
                case ID_40001:
                    FillInfo();
                    break;
                case ID_40002:
                    selectFileByMainListChoice();
                    break;
                case ID_40004:
                    terminateSelectProcess();
                    FillInfo();
                    break;
            }
            return 1;

        case WM_NOTIFY:
            //点击列头部排序消息
            if( (((LPNMHDR)lParam)->idFrom == IDC_LIST1) && (((LPNMHDR)lParam)->code == LVN_COLUMNCLICK) ) {
                FillInfo( ((LPNMLISTVIEW)lParam)->iSubItem );
            }
            //右击菜单
            if( ((LPNMHDR)lParam)->code == NM_RCLICK  && LOWORD( wParam ) == IDC_LIST1 ) {
                POINT pt;
                GetCursorPos( &pt );
                HMENU hmu = GetSubMenu( LoadMenuA( hinst, MAKEINTRESOURCEA( IDR_MENU1 ) ), 0 );
                TrackPopupMenu( hmu, TPM_LEFTALIGN, pt.x, pt.y, 0, MainWnd, 0 );
                return 1;
            }
            return TRUE;

        case WM_CLOSE:
            EndDialog( hDlg, 0 );
            return 1;
    }

    return 0;
}
VOID ShowMainWnd() {
    DialogBoxA( hinst, MAKEINTRESOURCEA( IDD_DIALOG1 ), (HWND)0, MainWindowProc );
}

//
//Main
//
void main() {
    hinst = GetModuleHandleA( 0 );
    ShowMainWnd();
}

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nShowCmd ) {

    hinst = hInstance;
    ShowMainWnd();
}