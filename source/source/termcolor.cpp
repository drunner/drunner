
#include "termcolor.h"


#if defined(_WIN32) || defined(_WIN64)
#   define OS_WINDOWS
#elif defined(__APPLE__)
#   define OS_MACOS
#elif defined(linux) || defined(__linux)
#   define OS_LINUX
#else
#   error unsupported platform
#endif

namespace termcolor
{
   //! Since C++ hasn't a way to hide something in the header from
   //! the outer access, I have to introduce this namespace which
   //! is used for internal purpose and should't be access from
   //! the user code.
   namespace __internal
   {
      //! Since C++ hasn't a true way to extract stream handler
      //! from the a given `std::ostream` object, I have to write
      //! this kind of hack.

      FILE* get_standard_stream(const std::ostream& stream)
      {
         if (&stream == &std::cout)
            return stdout;
         else if ((&stream == &std::cerr) || (&stream == &std::clog))
            return stderr;

         return nullptr;
      }


      //! Test whether a given `std::ostream` object refers to
      //! a terminal.

      bool is_atty(const std::ostream& stream)
      {
         FILE* std_stream = get_standard_stream(stream);

#if defined(OS_MACOS) || defined(OS_LINUX)
         return ::isatty(fileno(std_stream));
#else
         return ::_isatty(_fileno(std_stream));
#endif
      }


#if defined(_WIN32)
      //! Change Windows Terminal colors attribute. If some
      //! parameter is `-1` then attribute won't changed.
      void win_change_attributes(std::ostream& stream, int foreground, int background)
      {
         // yeah, i know.. it's ugly, it's windows.
         static WORD defaultAttributes = 0;

         // get terminal handle
         HANDLE hTerminal = INVALID_HANDLE_VALUE;
         if (&stream == &std::cout)
            hTerminal = GetStdHandle(STD_OUTPUT_HANDLE);
         else if (&stream == &std::cerr)
            hTerminal = GetStdHandle(STD_ERROR_HANDLE);

         // save default terminal attributes if it unsaved
         if (!defaultAttributes)
         {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (!GetConsoleScreenBufferInfo(hTerminal, &info))
               return;
            defaultAttributes = info.wAttributes;
         }

         // restore all default settings
         if (foreground == -1 && background == -1)
         {
            SetConsoleTextAttribute(hTerminal, defaultAttributes);
            return;
         }

         // get current settings
         CONSOLE_SCREEN_BUFFER_INFO info;
         if (!GetConsoleScreenBufferInfo(hTerminal, &info))
            return;

         if (foreground != -1)
         {
            info.wAttributes &= ~(info.wAttributes & 0x0F);
            info.wAttributes |= static_cast<WORD>(foreground);
         }

         if (background != -1)
         {
            info.wAttributes &= ~(info.wAttributes & 0xF0);
            info.wAttributes |= static_cast<WORD>(background);
         }

         SetConsoleTextAttribute(hTerminal, info.wAttributes);
      }
#endif // OS_WINDOWS

   } // namespace __internal

}