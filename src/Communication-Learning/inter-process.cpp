#include<Communication-Learning/inter-process.h>
#include<stdio.h>
#include<iostream>

int AnonymousPipe(LPWSTR szCommandLine)
{
	//create anonymous pipe
	HANDLE hRead, hWrite;
	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle = TRUE;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = nullptr;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		printf("Create pipe fail. error code: %d\n", GetLastError());
		return -1;
	}
	printf("Create pipe success.\n");

	//create sub process
	STARTUPINFO si{ 0 };
	si.cb = sizeof(si);
	si.hStdOutput = hWrite;
	si.hStdError = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	PROCESS_INFORMATION pi{ 0 };

	auto ret = CreateProcess(nullptr,
		szCommandLine,
		nullptr,
		nullptr,
		TRUE,
		CREATE_NO_WINDOW,
		nullptr,
		nullptr,
		&si,
		&pi);

	if (FALSE == ret) {
		printf("Create process fail. error code: %d\n", GetLastError());
		if (hWrite) {
			CloseHandle(hWrite);
			hWrite = nullptr;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return -1;
	}

	//read data from pipe
	if (nullptr != hWrite) {
		CloseHandle(hWrite);
		hWrite = nullptr;
	}

	char szRecvData[512]{ 0 };
	DWORD dwRecvSize{ 0 };
	
	std::string szOutputLine;
	while (FALSE != ReadFile(hRead, szRecvData, _countof(szRecvData) - 1, &dwRecvSize, nullptr)) {
		szRecvData[dwRecvSize] = '\0';
		auto szLineStart = szRecvData;
		char* szLineEnd = nullptr;
		while (TRUE) {
			auto szFound = szLineStart + strcspn(szLineStart, "\r\n");
			szLineEnd = '\0' != *szFound ? szFound : nullptr;
			if (nullptr == szLineEnd) {
				szOutputLine += szLineStart;
				break;
			}
			*szLineEnd = '\0';
			if (!szOutputLine.empty()) {
				szOutputLine += szLineStart;
				printf("%s\n", szOutputLine.c_str());
				szOutputLine.clear();
			}
			else {
				if ('\0' != *szLineStart)
					printf("%s\n", szLineStart);
			}
			szLineStart = szLineEnd + 1;
		}
	}
	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	system("pause");
	return 0;

}

