#include <Windows.h>
#include <stdio.h>
#include <GdiPlus.h>
#include <ShlObj.h>

#include "Log.h"
#include "resource.h"
#pragma comment(lib,"ws2_32.lib")

BOOL GetIpByDomainName(char *, char*);
void sendGetRequest(char*, char*, bool*);
Gdiplus::Image* LoadPNGFromStaticRes(HMODULE, UINT);

char remoteDomain[100];
char remoteApi[100];
char path [100];

ULONG_PTR gdiplusStartupToken;

Gdiplus::Image* cuteImage = 0;

#define IDC_TIMER 0x00000001

void OnBtnCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case IDC_BUTTON1:
		{
			if(strlen(remoteDomain) != 0)
			{
				KillTimer(hwndDlg, IDC_TIMER);
				SetWindowText(
					GetDlgItem(hwndDlg, IDC_BUTTON1), "FlY!");
				remoteDomain[0] = '\0';
				return;
			}
			char domain[100];
			char api[100];
			bool result = false;

			GetWindowText(
				GetDlgItem(hwndDlg, IDC_EDIT1), domain, 100);
			GetWindowText(
				GetDlgItem(hwndDlg, IDC_EDIT2), api, 100);
			
			sendGetRequest(api, domain, &result);
			if(result == true)
			{
				//Log("激活成功");
				SetWindowText(
					GetDlgItem(hwndDlg, IDC_BUTTON1), "激活成功");
				SetTimer(hwndDlg, IDC_TIMER, 50000, NULL);
				strcpy(remoteDomain, domain);
				strcpy(remoteApi, api);
				::SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
			}
			else
			{
				SetWindowText(
					GetDlgItem(hwndDlg, IDC_BUTTON1), "激活失败");
			}
		}
	}
}

void OnPicPaint(HWND hWndDlg, WPARAM wParam, LPARAM lParam)
{

	RECT wndRect;
    ::GetWindowRect(hWndDlg,&wndRect);
    SIZE wndSize = {wndRect.right-wndRect.left,wndRect.bottom-wndRect.top};
	
	//::SendMessage(hWndDlg, WM_ERASEBKGND, (WPARAM)hWndDlg, 0);
	//RedrawWindow(hWndDlg, &wndRect, NULL, RDW_ERASE);
	PAINTSTRUCT pt;
	HDC hDc = BeginPaint(hWndDlg, &pt);
	//HDC hDc = GetDC(hWndDlg);
	//HDC memDC = ::CreateCompatibleDC(hDc);
	HBITMAP memBitmap = ::CreateCompatibleBitmap(hDc,wndSize.cx,wndSize.cy);

	//COLORREF crBkgnd = GetBkColor(); 
    //HBRUSH hbrBkgnd = CreateSolidBrush(GetPixel(GetDC(GetParent(hWndDlg)), 1, 1));
	//HBRUSH hbrBkgnd = CreateSolidBrush(RGB(0,0,0));
	//SelectObject(memDC, hbrBkgnd);
	//RECT tmprd = {0, 0, wndSize.cx, wndSize.cy};

	//FillRect(hDc, &tmprd, hbrBkgnd);
	//Rectangle(memDC, 0, 0, wndSize.cx, wndSize.cy);

	SelectObject(hDc,memBitmap);
	//BitBlt(hDc,0,0,wndSize.cx,wndSize.cy,memDC, 0, 0, SRCCOPY);
	Gdiplus::Graphics graphics(hDc);
	graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
	graphics.DrawImage(cuteImage,0,0,wndSize.cx,wndSize.cy);
	
	//TransparentBlt(hDc,0,0,wndSize.cx,wndSize.cy,memDC, 0, 0, wndSize.cx, wndSize.cy, RGB(255,255,255));
	//DeleteDC(memDC);
	
	//ValidateRect(hWndDlg, &wndRect);
	//ReleaseDC(hWndDlg, hDc);
	DeleteObject(memBitmap);
	//DeleteObject(hbrBkgnd);

	EndPaint(hWndDlg,&pt);
}

void OnRequestTimer(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	if(strlen(remoteDomain) == 0 || strlen(remoteApi) == 0)
	{
		return;
	}
	bool result = false;
	sendGetRequest(remoteApi, remoteDomain, &result);
	if(result == true)
	{
		return;
	}
	else
	{
		SetWindowText(
			GetDlgItem(hwndDlg, IDC_BUTTON1), "激活中断");
		KillTimer(hwndDlg, IDC_TIMER);
	}
}

INT_PTR WINAPI PicProc(
  HWND hWndDlg,
  UINT message,
  WPARAM wParam,
  LPARAM lParam
  ){
	switch(message)
	{
	case WM_PAINT:
		{
			OnPicPaint(hWndDlg, wParam, lParam);
			break;
		}
	}
	return DefWindowProc(hWndDlg, message, wParam, lParam);
}

INT_PTR WINAPI Dlgproc(
  HWND hWndDlg,
  UINT message,
  WPARAM wParam,
  LPARAM lParam
  ){
	
	switch(message)
	{
		case WM_SYSCOMMAND:
		{
			if(SC_CLOSE == wParam)
			{
				if(DestroyWindow(hWndDlg))
				{
					PostQuitMessage(0);
					return TRUE;
				}
			}
			break;
		}
		case WM_INITDIALOG:
		{
			SetWindowLong(GetDlgItem(hWndDlg, IDC_STATIC2), GWL_WNDPROC, (LONG)PicProc);
			//SetWindowLong(GetDlgItem(hWndDlg, IDC_STATIC2), GWL_STYLE, 
			
			HINSTANCE hInstance = ::GetModuleHandle(NULL);
			HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
			::SendMessage(hWndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			::SendMessage(hWndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			break;
		}
		case WM_COMMAND:
		{
			OnBtnCommand(hWndDlg,wParam,lParam);
			return TRUE;
		}
		case WM_TIMER:
		{
			OnRequestTimer(hWndDlg, wParam, lParam);
		}
	}
	
	return FALSE;
}


int WINAPI WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPreInstance,
	LPSTR lpCmdLine,
    int nCmdShow
)
{
	Gdiplus::GdiplusStartupInput gdiInput;
    Gdiplus::GdiplusStartup(&gdiplusStartupToken,&gdiInput,NULL);

	HWND hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, Dlgproc);
	ShowWindow(hwnd, SW_SHOW);

	//解压资源
	/*
	HANDLE hFile = CreateFileW(L"tmp", GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;
	HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(IDB_PNG1), "PNG");
	HGLOBAL hMem = LoadResource(NULL, hRes);
	DWORD dwSize = SizeofResource(NULL, hRes);
	DWORD dwWrite = 0;
	WriteFile(hFile, hMem, dwSize, &dwWrite, NULL);
	CloseHandle(hFile);*/
	
	
	cuteImage = LoadPNGFromStaticRes(hInstance, IDB_PNG3);
	
	//cuteImage = new Gdiplus::Image(L"tmp");
	

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!IsDialogMessage(hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	
	//delete cuteImage;
	//DWORD res2 = GetLastError();
	delete cuteImage;
	Gdiplus::GdiplusShutdown(gdiplusStartupToken);
	
	//DeleteFileW(L"tmp");
	return 0;
}

BOOL GetIpByDomainName(char *szHost, char* szIp)
{
    WSADATA        wsaData;

    HOSTENT   *pHostEnt;
    int             nAdapter = 0;
    struct       sockaddr_in   sAddr;
    if (WSAStartup(0x0101, &wsaData))
    {
        Log(TEXT(" gethostbyname error for host:\n"));
        return FALSE;
    }

    pHostEnt = gethostbyname(szHost);
    if (pHostEnt)
    {
        if (pHostEnt->h_addr_list[nAdapter])
        {
            memcpy(&sAddr.sin_addr.s_addr, pHostEnt->h_addr_list[nAdapter], pHostEnt->h_length);
            sprintf(szIp, "%s", inet_ntoa(sAddr.sin_addr));
        }
    }
    else
    {
        //      DWORD  dwError = GetLastError();
        //      CString  csError;
        //      csError.Format("%d", dwError);
    }
    WSACleanup();
    return TRUE;
}

void sendGetRequest(char* api, char* domain, bool* ifsuc)
{
     //开始进行socket初始化;
    WSADATA wData;  
    ::WSAStartup(MAKEWORD(2,2),&wData);  

    SOCKET clientSocket = socket(AF_INET,1,0);      
    struct sockaddr_in ServerAddr = {0};  
    int Ret=0;  
    int AddrLen=0;  
    HANDLE hThread=0; 

	char bufSendF[100];
    char *bufSend = "GET /%s HTTP/1.1\r\n\r\n";
	sprintf(bufSendF, bufSend, api);

    char addIp[256] = {0};
    GetIpByDomainName(domain , addIp);
	//Log(addIp);
    ServerAddr.sin_addr.s_addr = inet_addr(addIp);  
    ServerAddr.sin_port = htons(80);;  
    ServerAddr.sin_family = AF_INET;  
    char bufRecv[3069] = {0};  
    int errNo = 0;  
    errNo = connect(clientSocket,(sockaddr*)&ServerAddr,sizeof(ServerAddr));  
    if(errNo==0)  
    {  
        //如果发送成功，则返回发送成功的字节数;
        if(send(clientSocket,bufSendF ,strlen(bufSendF),0)>0)  
        {
           //抽象地理解为发送一次激活请求
		}
        //如果接受成功，则返回接受的字节数;
         if(recv(clientSocket,bufRecv,3069,0)>0)  
         {  
            //抽象地理解为激活请求成功
			 
			 if(strstr(bufRecv, "<html>") != NULL)	//如果搜索到关键字，则认为没有激活
			 {
				 *ifsuc = false;
				 return;
			 }
			 *ifsuc = true;
         }  
    }  
    else  
    {  
        errNo=WSAGetLastError();  
    }  
    //socket环境清理;
    ::WSACleanup();  
}

Gdiplus::Image* LoadPNGFromStaticRes(HMODULE hModule, UINT nResId)
{
	HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(nResId), TEXT("PNG"));
	DWORD dwResSize = SizeofResource(hModule, hRes);

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwResSize);
	CopyMemory(GlobalLock(hGlobal), LockResource(LoadResource(hModule, hRes)), dwResSize);

	IStream* pIStream;
	CreateStreamOnHGlobal(hGlobal, FALSE, &pIStream);
	Gdiplus::Image* pImg = Gdiplus::Image::FromStream(pIStream);
	pIStream->Release();
	
	GlobalUnlock(hGlobal);
	GlobalFree(hGlobal);
	return pImg;
}