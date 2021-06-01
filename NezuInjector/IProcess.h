#pragma once
class IProcess {
public:
	virtual DWORD FindProcess(LPCTSTR name) = 0;
	virtual BOOL IsProcessOpen(LPCTSTR name) = 0;
	virtual DWORD KillAll(LPCTSTR name) = 0;
	virtual BOOL SpawnProcess(LPTSTR commandLine, LPSTARTUPINFOW startupInfo, LPPROCESS_INFORMATION processInformation) = 0;
	virtual BOOL Execute(LPCTSTR command) = 0;
	virtual BOOL IsProcessInitialized(HANDLE hProc) = 0;
};

//standard
class StProcess : public IProcess {
public:
	explicit StProcess();

	virtual DWORD FindProcess(LPCTSTR name);
	virtual BOOL IsProcessOpen(LPCTSTR name);
	virtual DWORD KillAll(LPCTSTR name);
	virtual BOOL SpawnProcess(LPTSTR commandLine, LPSTARTUPINFOW startupInfo, LPPROCESS_INFORMATION processInformation);
	virtual BOOL Execute(LPCTSTR command);
	virtual BOOL IsProcessInitialized(HANDLE hProc);

};

//Sandboxie
class SbProcess : public IProcess {
private:
	std::string sandbox_name;
public:
	explicit SbProcess(std::string sandbox_name) {
		this->sandbox_name = sandbox_name;
	}

	virtual DWORD FindProcess(LPCTSTR name);
	virtual BOOL IsProcessOpen(LPCTSTR name);
	virtual DWORD KillAll(LPCTSTR name);
	virtual BOOL SpawnProcess(LPTSTR commandLine, LPSTARTUPINFOW startupInfo, LPPROCESS_INFORMATION processInformation);
	virtual BOOL Execute(LPCTSTR command);
	virtual BOOL IsProcessInitialized(HANDLE hProc);

};