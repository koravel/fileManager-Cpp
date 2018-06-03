#include <windows.h>         // подключение библиотеки с функциями API
#include <stdio.h>
#include <commctrl.h>
#include <string>
#include "resource.h"
#include "LogicalDriveInfo.h"
#include "Utility.h"
#include <Shlwapi.h>
#include <Shobjidl.h>
#pragma comment (lib, "comctl32")
#pragma comment( lib, "shlwapi.lib")


using namespace std;

// Глобальные переменные:
HINSTANCE hInst; 	// Указатель приложения
LPCTSTR szWindowClass = "Explorer";
LPCTSTR szTitle = "Explorer";
HWND hWnd;
// Предварительное описание функций

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#define WM_CP_TEXTBOXLEFT_DBLCLICK 200
#define WM_CP_TEXTBOXRIGHT_DBLCLICK 201

#define ID_LISTVIEWLEFT 1
#define ID_LISTVIEWRIGHT 2
#define ID_COPYBUTTON 3
#define ID_RENAMEBUTTON 4
#define ID_CP_TEXTBOXLEFT 5
#define ID_CP_TEXTBOXRIGHT 6
#define ID_RENAMEEDITCTRL 7
#define ID_RENAMECONFIRMBTN 8
#define ID_MOVEBTN 9
#define ID_CREATEFOLDERBTN 10
#define ID_DELETEBTN 11

HWND ListViewLeft;
HWND ListViewRight;
HWND CurPathTextBoxLeft;
HWND CurPathTextBoxRight;
HWND CopyButton;
HWND RenameButton;
HWND RenameEditControl;
HWND RenameConfirmButton;
HWND MoveButton;
HWND CreateFolderButton;
HWND DeleteButton;

// Основная программа 
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	MSG msg;

	// Регистрация класса окна 
	MyRegisterClass(hInstance);

	// Создание окна приложения
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Цикл обработки сообщений
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_LBUTTONDBLCLK)
		{
			if (msg.hwnd == CurPathTextBoxLeft) WndProc(hWnd, WM_CP_TEXTBOXLEFT_DBLCLICK, 0, (LPARAM)&msg);
			else if (msg.hwnd == CurPathTextBoxRight) WndProc(hWnd, WM_CP_TEXTBOXRIGHT_DBLCLICK, 0, (LPARAM)&msg);
			
			
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

//  FUNCTION: MyRegisterClass()
//  Регистрирует класс окна 

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;	// стиль окна
	wcex.lpfnWndProc = (WNDPROC)WndProc; // оконная процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;		// указатель приложения
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);		// определение иконки
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);    // опре-деление курсора
	wcex.hbrBackground = GetSysColorBrush(0);   // установка фона
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);		// определение меню
	wcex.lpszClassName = szWindowClass;	// имя класса
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); // регистрация класса окна
}

// FUNCTION: InitInstance(HANDLE, int)
// Создает окно приложения и сохраняет указатель приложения в переменной hInst

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // сохраняет указатель приложения в переменной hInst

	hWnd = CreateWindow(szWindowClass, // имя класса окна
		szTitle,   // имя приложения
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, //| WS_SIZEBOX, // стиль окна
		0,	// положение по Х
		0, 	// положение по Y
		1320,    // размер по Х
		760,    // размер по Y
		NULL,	// описатель родительского окна
		NULL,       // описатель меню окна
		hInstance,  // указатель приложения
		NULL);     // параметры создания.

	if (!hWnd) // Если окно не создалось, функция возвращает FALSE
	{
		return FALSE;
	}
	ShowWindow(hWnd, SW_NORMAL);		// Показать окно
	UpdateWindow(hWnd);			// Обновить окно
	return TRUE;				//Успешное завершение функции
}

LogicalDriveInfo* drivesInfo;

string ListViewLeftCurrentDirectory = "";
string ListViewRightCurrentDirectory = "";

LPNMHDR pnmh;

bool listViewLeftFocus = false,
	listViewRightFocus = false,
	replacementConfirm = false,
	replacementConfirmAll = false,
	createFolder = false;

void AddColumn(HWND listView, LVCOLUMN lvc, int index, int size, char* text)
{
	lvc.cx = size;
	lvc.pszText = text;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(listView, index, &lvc);
}

void AddColumns(HWND listView)
{
	LVCOLUMN lvc;

	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	
	AddColumn(listView, lvc, 0, 100, "Name");
	AddColumn(listView, lvc, 1, 100, "Type");
	AddColumn(listView, lvc, 2, 200, "Space");
}

void AddSingleLVItem(HWND listView, int index, int subIndex, char* text)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE;
	lvi.iImage = index;
	lvi.iItem = index;
	lvi.iSubItem = subIndex;
	lvi.pszText = text;
	if(!subIndex) ListView_InsertItem(listView, &lvi);
	else ListView_SetItem(listView, &lvi);
}

void AddMultiLVItem(HWND listView, int index, char** text, int singleElementsCount)
{
	for (int i = 0; i < singleElementsCount; i++)
	{
		AddSingleLVItem(listView, index, i, text[i]);
	}
}

void AddIcon(HIMAGELIST hSmall, DWORD fileType, char* path)
{
	SHFILEINFO SHFileIfo;

	SHGetFileInfo(path, fileType, &SHFileIfo, sizeof(SHFileIfo), SHGFI_ICONLOCATION | SHGFI_ICON | SHGFI_SMALLICON);
	ImageList_AddIcon(hSmall, SHFileIfo.hIcon);
	DestroyIcon(SHFileIfo.hIcon);
}

void InitializeListViewItemImages(HWND listView, char* path)
{
	int itemsNumber = ListView_GetItemCount(listView);

	HIMAGELIST hSmall;
	
	hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, itemsNumber, 1);

	int begin = 0;//for logical drives
	if (strlen(path))
	{
		begin = 1;//for directories
		AddIcon(hSmall, FILE_ATTRIBUTE_DIRECTORY, "");// for ".." shortcut
	}

	for (int i = begin; i < itemsNumber + 1; i++)
	{
		char buffer[MAX_PATH];
		
		ListView_GetItemText(listView, i, 0, buffer, 256);
		
		char item[MAX_PATH]{ '\0' };

		strcat_s(item, strlen(path) + strlen(item) + 1, path);
		strcat_s(item, strlen(buffer) + strlen(item) + 1, buffer);

		DWORD type = GetFileAttributes(item);

		AddIcon(hSmall, type, item);
	}

	ListView_SetImageList(listView, hSmall, LVSIL_SMALL);
}

void InitializeListViewItems(HWND listView)
{
	SendMessage(listView, LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0);

	int i = 0;

	drivesInfo[i - 1].lastDrive = false;//on some PCs drivesInfo[i - 1].lastDrive initialize as true

	while (!drivesInfo[i - 1].lastDrive)
	{
		char* freeSpace = new char[16];
		char* allSpace = new char[16];
		LogicalDriveInfo::GetLogicalDriveFreeSpace(&drivesInfo[i], _Mb, freeSpace);
		LogicalDriveInfo::GetLogicalDriveAllSpace(&drivesInfo[i], _Mb, allSpace);
		string spaceInfo;

		spaceInfo.push_back('<');
		spaceInfo.append(freeSpace);
		spaceInfo.append(" Mb / ");
		spaceInfo.append(allSpace);
		spaceInfo.append(" Mb >");

		char** text = new char*[3];
		text[0] = drivesInfo[i].drivePath;
		text[1] = LogicalDriveInfo::GetLogicalDriveType(&drivesInfo[i]);
		text[2] = (char*)spaceInfo.c_str();

		AddMultiLVItem(listView, i, text, 3);

		i++;
	}

	InitializeListViewItemImages(listView, "");
}

HWND InitializeList(HWND hwnd, int id, int X, int Y, int width, int height)
{
 return CreateWindowEx(0, WC_LISTVIEW,
		"List View",
		LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_AUTOARRANGE,
		X, Y,
		width, height,
		hwnd,
		(HMENU)id,
		hInst,
		NULL);
}

void AboutUs(HWND hwnd)
{
	MessageBox(hwnd, "Editor v1.0 by Dimon_dx", "About us", MB_OK);
}

void ScanDirectory(HWND listView, char* path)
{
	SendMessage(listView, LVM_DELETEALLITEMS, NULL, NULL);//wParam and lParam must be zero

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	int i = 0;
	string sharingPath;
	
	sharingPath.append(path);
	sharingPath.append("*.*");
	hFind = FindFirstFile(sharingPath.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (isRootFolder(drivesInfo, path))
		{
			AddSingleLVItem(listView, i, 0, "..");
			AddSingleLVItem(listView, i, 1, "< Папка >");
			i++;
		}
		do
		{
			if (FindFileData.cFileName[strlen(FindFileData.cFileName) - 1] != '.' || FindFileData.cFileName[strlen(FindFileData.cFileName) - 2] == '.')
			{
				AddSingleLVItem(listView, i, 0, FindFileData.cFileName);

				string fileDestination;
				fileDestination.append(path);
				fileDestination.append(FindFileData.cFileName);
				
				if (isDirectory(fileDestination))
					AddSingleLVItem(listView, i, 1, "< Папка >");
				else
				{
					AddSingleLVItem(listView, i, 1, "< Файл >");
					unsigned long long intFileSize = ((FindFileData.nFileSizeHigh * (MAXDWORD + 1)) + FindFileData.nFileSizeLow) / 1024;
					string fileSize;
					fileSize += to_string(intFileSize) + " Kb";
					AddSingleLVItem(listView, i, 2, (char*)fileSize.c_str());
				}

				++i;
			}
			
		} while (FindNextFile(hFind, &FindFileData) != 0);

		FindClose(hFind);
		InitializeListViewItemImages(listView, path);
	}
}

string* ScanDirectory(string path)
{
	if (isDirectory(path) && path[path.length() - 1] == '\\')
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;

		int i = 0;
		int itemsCount = 0;
		string sharingPath;

		sharingPath.append(path);
		sharingPath.append("*.*");
		hFind = FindFirstFile(sharingPath.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				itemsCount++;
			} while (FindNextFile(hFind, &FindFileData) != 0);
			
			FindClose(hFind);

			hFind = FindFirstFile(sharingPath.c_str(), &FindFileData);

			FindNextFile(hFind, &FindFileData);//skip '.' folder
			FindNextFile(hFind, &FindFileData);//skip '..' folder

			string* result = new string[itemsCount];

			do
			{
				result[i] = FindFileData.cFileName;
				i++;
			} while (FindNextFile(hFind, &FindFileData) != 0);

			FindClose(hFind);
			return result;
		}
		FindClose(hFind);
	}
	
	return NULL;
}

string ChangeDirectory(string currentDirectory, LPNMHDR pnmh, LPNMLISTVIEW pnmLV)
{
	char* currentObject = new char[MAX_PATH];
	ListView_GetItemText(pnmh->hwndFrom, pnmLV->iItem, pnmLV->iSubItem, currentObject, MAX_PATH);
	currentObject[strlen(currentObject)] = '\0';
	if (currentObject[0] != '.' && currentObject[1] != '.')
	{
		currentDirectory.append(currentObject);
		if(!isRootFolder(drivesInfo, currentDirectory)) currentDirectory.append("\\");
		return currentDirectory;
	}
	else
		return currentDirectory = EraseLastDirectory(currentDirectory);
}

void RefreshListViewItems(HWND listView, string currrentDirectoryPointer)
{
	if (currrentDirectoryPointer == "") InitializeListViewItems(listView);
	else ScanDirectory(listView, (char*)currrentDirectoryPointer.c_str());
}

void ShareProcessing(HWND listView, LPNMHDR pnmh, LPNMLISTVIEW pnmLV, string& currrentDirectoryPointer)
{
	char* currentObject = new char[MAX_PATH];
	ListView_GetItemText(pnmh->hwndFrom, pnmLV->iItem, pnmLV->iSubItem, currentObject, MAX_PATH);
	string fullPath = currrentDirectoryPointer.c_str();
	fullPath.append(currentObject);
	if (isDirectory(fullPath))
	{
		currrentDirectoryPointer = ChangeDirectory(currrentDirectoryPointer, pnmh, pnmLV);

		RefreshListViewItems(listView, currrentDirectoryPointer);
		
	}
	else ShellExecute(0, "open", fullPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

HWND CreateEditControl(HWND hwnd, DWORD style, int id, bool readonly, int x, int y, int width, int height)
{
	if (readonly) style = style | ES_READONLY;

	return CreateWindowEx(NULL, TEXT("Edit"), NULL,
		style,
		x, y, width, height,
		hwnd, (HMENU)id, NULL, NULL);
}

HWND CreateTextBox(HWND hwnd, int id, int x, int y, int width, int height)
{
	DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER;

	return CreateEditControl(hwnd, style, id, true, x, y, width, height);
}

void CheckPathTexBoxStyle(HWND hwnd, int id)
{
	HWND textBox = GetDlgItem(hwnd, id);
	long style = GetWindowLong(textBox, GWL_STYLE);
	if (style & ES_READONLY) SendDlgItemMessage(hwnd, id, EM_SETREADONLY, FALSE, NULL);
}

void PathTextBoxOnKillFocus(HWND hwnd, HWND listView, int textBoxID, string& bindString)
{
	HWND textBox = GetDlgItem(hwnd, textBoxID);
		char userDefinedPath[MAX_PATH];
		GetWindowText(textBox, userDefinedPath, MAX_PATH);
		if (!PathFileExists(userDefinedPath)) SetWindowText(textBox, bindString.c_str());
		else
		{
			if (userDefinedPath[strlen(userDefinedPath) - 1] != '\\')
			{
				strcat_s(userDefinedPath, MAX_PATH, "\\");
				SetWindowText(textBox, userDefinedPath);
			}
			if (bindString != userDefinedPath)
			{
				bindString = userDefinedPath;
				RefreshListViewItems(listView, bindString);
			}
		}

		SendDlgItemMessage(hwnd, textBoxID, EM_SETREADONLY, TRUE, NULL);
}

HWND CreateButton(HWND hwnd, int id, DWORD style, char* const text, int x, int y, int width, int height)
{
	return CreateWindow(
	"BUTTON",  // Predefined class; Unicode assumed
	text,      // Button text
	style,  // Styles
	x,         // x position
	y,         // y position
	width,        // Button width
	height,        // Button height
	hwnd,     // Parent window
	(HMENU)id,       // No menu.
	(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
	NULL);
}

string FullPathFrom, FullPathTo;
char NewDirectory[MAX_PATH];

BOOL CALLBACK ReplacmentDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), FullPathTo.c_str());
		SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT2), FullPathFrom.c_str());
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			replacementConfirm = true;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			DestroyWindow(hwndDlg);
		}
		break;
	}
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		DestroyWindow(hwndDlg);
		return TRUE;
	case WM_DESTROY:
		UINT checked = SendMessage(GetDlgItem(hwndDlg, IDC_CHECK1), BM_GETCHECK, NULL, NULL);
		if (checked) replacementConfirmAll = true;
		break;
	}
	return FALSE;
}

BOOL CALLBACK CreateFolderDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			createFolder = true;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			DestroyWindow(hwndDlg);
		}
		break;
	}
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		DestroyWindow(hwndDlg);
		return TRUE;
	case WM_DESTROY:
		GetWindowText(GetDlgItem(hwndDlg, IDC_CREATEFODER), NewDirectory, MAX_PATH);
		break;
	}
	return FALSE;
	return false;
}

bool CreateFolder(HWND hwnd, HWND listView, string directory)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_CREATEFOLDER_DIALOG), hwnd, (DLGPROC)CreateFolderDialogProc);
	if (createFolder)
	{
		if (PathFileExists(NewDirectory)) MessageBox(hwnd, "Папка уже существует", "Предупреждение", MB_OK);
		else
		{
			string fullPath = directory;
			fullPath.append(NewDirectory);
			CreateDirectory(fullPath.c_str(), NULL);
			RefreshListViewItems(listView, directory);
		}
		createFolder = false;
	}
	NewDirectory[0] = '\0';
}

bool Move()
{
	bool result = CopyFile(FullPathFrom.c_str(), FullPathTo.c_str(), TRUE);
	DeleteFile(FullPathFrom.c_str());
	return result;
}

string RecFileExists(string& obj, string directory)
{
	if (!PathFileExists((directory + obj).c_str())) return obj;
	else return RecFileExists("[copy]" + obj, directory);

}

bool Copy(string directoryFrom, string directoryTo, string fileName, HWND hwnd, bool moveItem, bool copyInSamePlace)
{
	bool objExists = PathFileExists((directoryTo + fileName).c_str());
	string taggedName = fileName;
	if (copyInSamePlace)
	{	
		objExists = false;
		taggedName = RecFileExists(taggedName, directoryTo);
	}

	FullPathFrom.clear();
	FullPathTo.clear();

	FullPathFrom = directoryFrom;
	FullPathFrom.append(fileName);
	FullPathTo = directoryTo;
	if (copyInSamePlace) FullPathTo.append(taggedName);
	else FullPathTo.append(fileName);

	if (objExists && !replacementConfirmAll)
	{
		DialogBox(hInst, MAKEINTRESOURCE(IDD_REPLACMENT_DIALOG), hwnd, (DLGPROC)ReplacmentDialogProc);

		if (replacementConfirm)
		{
			replacementConfirm = false;
			
			DeleteFile(FullPathTo.c_str());

			if (moveItem) return Move();

			return CopyFile(FullPathFrom.c_str(), FullPathTo.c_str(), TRUE);
		}
		return false;
	}
	else
	{
		if (moveItem) return Move();
		return CopyFile(FullPathFrom.c_str(), FullPathTo.c_str(), TRUE);
	}
}

void CopyDirectory(string directoryFrom, string directoryTo, string dirName, HWND hwnd, bool moveItem, bool copyInSamePlace)
{
	string objFullPathFrom = directoryFrom + dirName + "\\";
	string objFullPathTo = directoryTo + dirName + "\\";
	string* directoryContent = ScanDirectory(objFullPathFrom);

	if (copyInSamePlace) objFullPathTo = directoryTo + RecFileExists(dirName, directoryTo) + "\\";

	if (!PathFileExists(objFullPathTo.c_str())) CreateDirectory(objFullPathTo.c_str(), NULL);

	for (unsigned int i = 0; directoryContent[i] != ""; i++)
	{
		if (directoryContent[i] == "..") continue;
		if (isDirectory(objFullPathFrom + directoryContent[i])) CopyDirectory(objFullPathFrom, objFullPathTo, directoryContent[i], hwnd, moveItem, false);
		else Copy(objFullPathFrom, objFullPathTo, directoryContent[i], hwnd, moveItem, false);
		continue;
	}
}

bool Rename(string directory, string fileNameOld, string fileNameNew)
{
	string oldName = directory, newName = directory;
	oldName.append(fileNameOld);
	newName.append(fileNameNew);
	return MoveFile(oldName.c_str(), newName.c_str());
}

void CopyItems(HWND hwnd, HWND listViewFrom, HWND listViewTo, string directoryFrom, string directoryTo, bool moveItems)
{
	int iPos = ListView_GetNextItem(listViewFrom, -1, LVNI_SELECTED);

	bool refresh = false;
	bool copyInSamePlace = false;

	while (iPos != -1)
	{
		char objName[MAX_PATH];
		ListView_GetItemText(listViewFrom, iPos, 0, objName, MAX_PATH);
		
		string newName = directoryTo;
		newName.append(objName);

		string oldName = directoryFrom;
		oldName.append(objName);

		

		if (directoryFrom == directoryTo)
		{
			if (moveItems)
			{
				char fileNameCheck[MAX_PATH];
				ListView_GetItemText(listViewFrom, iPos, 0, fileNameCheck, MAX_PATH);

				string fullObjNameCheck = "Запрещено перемещение самого в себя для ";
				fullObjNameCheck += directoryTo;
				fullObjNameCheck.append(fileNameCheck);

				MessageBox(hwnd, fullObjNameCheck.c_str(), "Предупреждение", MB_OK);

				fullObjNameCheck.clear();
				return;
			}
			else copyInSamePlace = true;
		}

		if (isDirectory(oldName))
		{
			CopyDirectory(directoryFrom, directoryTo, objName, hwnd, moveItems, copyInSamePlace);
			if (moveItems)
			{
				string fullPath = directoryFrom + objName;
				RemoveDirectory(fullPath.c_str());
			}
			refresh = !PathFileExists(newName.c_str());
		}
		else if (Copy(directoryFrom, directoryTo, objName, hwnd, moveItems, copyInSamePlace)) refresh = true;

		iPos = ListView_GetNextItem(listViewFrom, iPos, LVNI_SELECTED);
	}

	if (refresh) RefreshListViewItems(listViewTo, directoryTo);
	if (moveItems || copyInSamePlace)
	{
		RefreshListViewItems(listViewTo, directoryTo);
		RefreshListViewItems(listViewFrom, directoryFrom);
	}

	replacementConfirmAll = false;
}

void RecDelete(string directory, string obj)
{
	string dir = directory + obj + "\\";
	string* directoryContent = ScanDirectory(dir);
	for (unsigned int i = 0; directoryContent[i] != ""; i++)
	{
		string fullPath = dir + directoryContent[i];
		if (directoryContent[i] == "..") continue;
		if (isDirectory(fullPath))
		{
			RecDelete(dir, directoryContent[i]);
			RemoveDirectory(fullPath.c_str());
		}
		else DeleteFile(fullPath.c_str());
		continue;
	}
}

void Delete(HWND hwnd, HWND listView, string directory)
{
	int iPos = ListView_GetNextItem(listView, -1, LVNI_SELECTED);
	bool refresh = false;
	while (iPos != -1)
	{
		char objName[MAX_PATH];
		ListView_GetItemText(listView, iPos, 0, objName, MAX_PATH);
		string fullPath = directory;
		fullPath.append(objName);
		string text = "Вы уверены, что хотите удалить " + fullPath + "?";
		if (MessageBox(hwnd, text.c_str(), "Удаление", MB_OK) == IDOK)
		{
			if (isDirectory(fullPath))
			{
				RecDelete(directory, objName);
				RemoveDirectory(fullPath.c_str());
			}
			else DeleteFile(fullPath.c_str());
			refresh = true;
		}
		iPos = ListView_GetNextItem(listView, iPos, LVNI_SELECTED);
	}
	if (refresh) RefreshListViewItems(listView, directory);
}

void RenameByPath(HWND hwnd, RECT* hwndRect, HWND listView, string directory, bool renameConfirm)
{
	char fileNameNew[MAX_PATH];
	int iPos = ListView_GetNextItem(listView, -1, LVNI_SELECTED);

	if (ListView_GetSelectedCount(listView) == 1 && !renameConfirm)
	{
		char oldName[MAX_PATH];
		ListView_GetItemText(listView, iPos, 0, oldName, MAX_PATH);
		SetWindowText(RenameEditControl, oldName);

		POINT* itemPos = new POINT();
		RECT* listViewRect = new RECT();
		ListView_GetItemPosition(listView, iPos, itemPos);
		GetWindowRect(listView, listViewRect);
		SetWindowPos(RenameEditControl, HWND_TOP, listViewRect->left + 10, itemPos->y + 20, 250, 20, SWP_SHOWWINDOW);
		SetWindowPos(RenameConfirmButton, HWND_TOP, listViewRect->left + 260, itemPos->y + 20, 20, 20, SWP_SHOWWINDOW);
		InvalidateRect(hwnd, hwndRect, NULL);
		return;
	}
	if (renameConfirm)
	{
		SetWindowPos(RenameEditControl, HWND_BOTTOM, 0, 0, 250, 20, SWP_HIDEWINDOW);
		SetWindowPos(RenameConfirmButton, HWND_BOTTOM, 0, 0, 20, 20, SWP_HIDEWINDOW);
		InvalidateRect(hwnd, hwndRect, NULL);
		GetWindowText(RenameEditControl, fileNameNew, MAX_PATH);
	}

	if (ListView_GetSelectedCount(listView) > 1)
	{
		//Dialog for multiple renames
	}
	
	while (iPos != -1)
	{
		char fileName[MAX_PATH];
		ListView_GetItemText(listView, iPos, 0, fileName, MAX_PATH);
		if (Rename(directory, fileName, fileNameNew)) ListView_SetItemText(listView, iPos, 0, fileNameNew);

		iPos = ListView_GetNextItem(listView, iPos, LVNI_SELECTED);
	}
	
}

DWORD standartButtonStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rt;

	switch (message)
	{
		case WM_CREATE:
			drivesInfo = LogicalDriveInfo::SetLogicalDrivesInfo();

			ListViewLeft = InitializeList(hWnd, ID_LISTVIEWLEFT, 0, 20, 630, 660);
			ListViewRight = InitializeList(hWnd, ID_LISTVIEWRIGHT, 650, 20, 650, 660);
		
			AddColumns(ListViewLeft);
			AddColumns(ListViewRight);
		
			InitializeListViewItems(ListViewLeft);
			InitializeListViewItems(ListViewRight);	

			CurPathTextBoxLeft = CreateTextBox(hWnd, ID_CP_TEXTBOXLEFT, 0, 0, 630, 20);
			CurPathTextBoxRight = CreateTextBox(hWnd, ID_CP_TEXTBOXRIGHT, 650, 0, 650, 20);

			CopyButton = CreateButton(hWnd, ID_COPYBUTTON, standartButtonStyle, "Copy", 0, 680, 100, 20);
			RenameButton = CreateButton(hWnd, ID_RENAMEBUTTON, standartButtonStyle, "Rename", 100, 680, 100, 20);
			MoveButton = CreateButton(hWnd, ID_MOVEBTN, standartButtonStyle, "Move", 200, 680, 100, 20);
			CreateFolderButton = CreateButton(hWnd, ID_CREATEFOLDERBTN, standartButtonStyle, "Create folder", 300, 680, 100, 20);
			DeleteButton = CreateButton(hWnd, ID_DELETEBTN, standartButtonStyle, "Delete", 400, 680, 100, 20);

			RenameEditControl =  CreateEditControl(hWnd, WS_CHILD | WS_BORDER, ID_RENAMEEDITCTRL, false, 0, 0, 250, 20);
			RenameConfirmButton = CreateButton(hWnd, ID_RENAMECONFIRMBTN, WS_CHILD | BS_DEFPUSHBUTTON, "OK", 0, 0, 20, 20);
		break;

		case WM_CP_TEXTBOXLEFT_DBLCLICK:
			CheckPathTexBoxStyle(hWnd, ID_CP_TEXTBOXLEFT);
		break;

		case WM_CP_TEXTBOXRIGHT_DBLCLICK:
			CheckPathTexBoxStyle(hWnd, ID_CP_TEXTBOXRIGHT);
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case ID_ABOUTUS: AboutUs(hWnd);
				break;

				case ID_COPYBUTTON: 
					if (listViewLeftFocus) CopyItems(hWnd, ListViewLeft, ListViewRight, ListViewLeftCurrentDirectory, ListViewRightCurrentDirectory, false);
					else if (listViewRightFocus) CopyItems(hWnd, ListViewRight, ListViewLeft, ListViewRightCurrentDirectory, ListViewLeftCurrentDirectory, false);
				break;

				case ID_RENAMEBUTTON:
					if (listViewLeftFocus) RenameByPath(hWnd, &rt, ListViewLeft, ListViewLeftCurrentDirectory, false);
					else if (listViewRightFocus) RenameByPath(hWnd, &rt, ListViewRight, ListViewRightCurrentDirectory, false);
				break;

				case ID_RENAMECONFIRMBTN:
					if (listViewLeftFocus) RenameByPath(hWnd, &rt, ListViewLeft, ListViewLeftCurrentDirectory, true);
					else if (listViewRightFocus) RenameByPath(hWnd, &rt, ListViewRight, ListViewRightCurrentDirectory, true);
				break;

				case ID_MOVEBTN:
					if (listViewLeftFocus) CopyItems(hWnd, ListViewLeft, ListViewRight, ListViewLeftCurrentDirectory, ListViewRightCurrentDirectory, true);
					else if (listViewRightFocus) CopyItems(hWnd, ListViewRight, ListViewLeft, ListViewRightCurrentDirectory, ListViewLeftCurrentDirectory, true);
				break;
				
				case ID_CREATEFOLDERBTN:
					if (listViewLeftFocus) CreateFolder(hWnd, ListViewLeft, ListViewLeftCurrentDirectory);
					else if (listViewRightFocus) CreateFolder(hWnd, ListViewRight, ListViewRightCurrentDirectory);
				break;

				case ID_DELETEBTN:
					if (listViewLeftFocus) Delete(hWnd, ListViewLeft, ListViewLeftCurrentDirectory);
					else if (listViewRightFocus) Delete(hWnd, ListViewRight, ListViewRightCurrentDirectory);
				break;

				default:
				break;
			}
		
			switch (HIWORD(wParam))
			{
				case EN_KILLFOCUS:
					if ((HWND)lParam == CurPathTextBoxLeft) PathTextBoxOnKillFocus(hWnd, ListViewLeft, ID_CP_TEXTBOXLEFT, ListViewLeftCurrentDirectory);
					else if ((HWND)lParam == CurPathTextBoxRight) PathTextBoxOnKillFocus(hWnd, ListViewRight, ID_CP_TEXTBOXRIGHT, ListViewRightCurrentDirectory);
				break;
				default:
				break;
			}
		break;
		case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR)lParam;
			LPNMLISTVIEW pnmLV = (LPNMLISTVIEW)lParam;

			if (pnmh->hwndFrom == ListViewLeft || pnmh->hwndFrom == ListViewRight)
			{
				if ((pnmh->idFrom == ID_LISTVIEWLEFT) || (pnmh->idFrom == ID_LISTVIEWRIGHT))
				{
					switch (pnmh->code)
					{
						case NM_DBLCLK:
						{
							if (pnmh->idFrom == ID_LISTVIEWLEFT)
							{
								ShareProcessing(ListViewLeft, pnmh, pnmLV, ListViewLeftCurrentDirectory);
								SetWindowText(CurPathTextBoxLeft, ListViewLeftCurrentDirectory.c_str());
							}
							else if (pnmh->idFrom == ID_LISTVIEWRIGHT)
							{
								ShareProcessing(ListViewRight, pnmh, pnmLV, ListViewRightCurrentDirectory);
								SetWindowText(CurPathTextBoxRight, ListViewRightCurrentDirectory.c_str());
							}
							break;
						}
						case NM_CLICK:
						{
							if (pnmh->idFrom == ID_LISTVIEWLEFT)
							{
								listViewLeftFocus = true;
								listViewRightFocus = false;
							}
							else if (pnmh->idFrom == ID_LISTVIEWRIGHT)
							{
								listViewLeftFocus = false;
								listViewRightFocus = true;
							}
							break;
						}
					
					}
				}
			}
			break;
		}

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);	// розпочати графічне відображення
			GetClientRect(hWnd, &rt); 	// область вікна для малювання
			EndPaint(hWnd, &ps);
		break;

		case WM_DESTROY: PostQuitMessage(0);
		break;

		default:
			// Обработка сообщений, которые не обработаны пользователем
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
