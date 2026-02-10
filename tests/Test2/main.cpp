#include <Windows.h>
#include <iostream>

int main()
{
	HMODULE bypassDLL = LoadLibraryW(L"UAC_Bypass.dll");


	if (!bypassDLL)
	{
		std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
		return 1;
	}


	auto func = (void(*)())GetProcAddress(bypassDLL, "Example");

	if(!func)
	{
		std::cerr << "Failed to find function: " << GetLastError() << std::endl;
		return 1;
	}
	



	std::cout << "This Uac Bypass is using well known method CMSTP" << "\n";
	std::cout << "This example was created by : pieczywosoleckie " << std::endl;

	func();

	std::cin.get();

	FreeLibrary(bypassDLL);
	return 0;
}
