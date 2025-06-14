#include "core.h"
#include "blunder.h"

//////////////////////////////////////////////////////////////////////////

int32_t main(const int32_t argc, char **pArgv)
{
  // Ignore Args for now.
  (void)argc;
  (void)pArgv;

  sformatState_ResetCulture();
  cpu_info::DetectCpuFeatures();

  // Set Working Directory.
  do
  {
    wchar_t filePath[MAX_PATH];
    GetModuleFileNameW(nullptr, filePath, sizeof(filePath) / sizeof(wchar_t));

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      print_error_line("Insufficient Buffer for Module File Name Retrieval.");
      break;
    }

    wchar_t *lastSlash = nullptr;

    for (size_t i = 0; i < sizeof(filePath) / sizeof(wchar_t) && filePath[i] != L'\0'; i++)
      if (filePath[i] == L'\\')
        lastSlash = filePath + i;

    if (lastSlash != nullptr)
      *lastSlash = L'\0';

    if (0 == SetCurrentDirectoryW(filePath))
      print_error_line("Failed to set working directory.");

  } while (false);

  print("Blunder ConIO (built " __DATE__ " " __TIME__ ") running on ", cpu_info::GetCpuName(), ".\n");

  return EXIT_SUCCESS;
}
