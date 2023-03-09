#include<Communication-Learning/inter-process.h>

int main()
{
	TCHAR commandLine[] = _T("testProcess");
	AnonymousPipe(commandLine);
	return 0;
}