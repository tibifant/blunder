#include <stdio.h>
#include <exception>
#include <stdlib.h>
#include <malloc.h>

#define ASIO_STANDALONE 1
#define ASIO_NO_EXCEPTIONS 1

#define BLUNDER_WEBIO_LOCALHOST
#define BLUNDER_WEBIO_HOSTNAME "https://hostname_not_configured"

namespace asio
{
  namespace detail
  {
    template <typename Exception>
    void throw_exception(const Exception &e)
    {
#ifdef _MSC_VER
      __debugbreak(); // windows only, sorry!
#endif
      printf("Exception thrown: %s.\n", e.what());
    }
  }
}

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702) // unreachable (somewhere in json.h)
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (push, 0)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
#include "crow.h"
#include "crow/middlewares/cors.h"
#ifdef _MSC_VER
#pragma warning (pop)
#else
#pragma GCC diagnostic pop
#endif

//////////////////////////////////////////////////////////////////////////

#include "core.h"
#include "blunder.h"

//////////////////////////////////////////////////////////////////////////

static std::atomic<bool> _IsRunning = true;

//////////////////////////////////////////////////////////////////////////

int32_t main(const int32_t argc, const char **pArgv)
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

  print("Blunder WebIO (built " __DATE__ " " __TIME__ ") running on ", cpu_info::GetCpuName(), ".\n");

  {
    crow::App<crow::CORSHandler> app;

    auto &cors = app.get_middleware<crow::CORSHandler>();
#ifndef BLUNDER_WEBIO_LOCALHOST
    cors.global().origin(BLUNDER_WEBIO_HOSTNAME);
#else
    cors.global().origin("*");
#endif

    app.port(21110).multithreaded().run();

    _IsRunning = false;
  }

  return EXIT_SUCCESS;
}
