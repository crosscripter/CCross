/*_____________________________________________________________________________
*| 		
*|				██████    ██████
*|				██████    ██████
*|				██            ██
*|				██████    ██████
*|				██████    ██████
*|	    
*|           		     Cross Programming Language
*|         		   Version 0.1.0, Build #20230929
*|_____________________________________________________________________________
*| FILE:   cross.c
*| DATE:   2023-09-29 9:09 AM CDT
*| AUTHOR: Michael Schutt (@crosscripter)
*| USAGE:  The Cross compiler and runtime.
*|
*| $ cross [<modules..>] [<-flags..>]
*|_____________________________________________________________________________
*| 	~!~ THIS FILE IS PART OF THE CROSS PROGRAMMING LANGUAGE PROJECT ~!~
*| Copyright © Michael Schutt, 2023-2024. All Rights Reserved.
*| Glory to God our Father and the Lord Jesus Christ and the power of His Cross
******************************************************************************/

#include <assert.h>
#include <locale.h>

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utils */
#define _STR(x) # x
#define STR(x) _STR(x)

/* Namespacing */
#define NS_PREFIX CROSS_
#define NSCAT(A,B) _NSCAT(A,B) 
#define _NSCAT(A,B) A ## B 
#define CROSS(N) NSCAT(NS_PREFIX,N)

/* Build Info */
#define CROSS_NAME "Cross"

// Version
#define CROSS_VERSION_MAJOR 0
#define CROSS_VERSION_MINOR 1
#define CROSS_VERSION_REVISION 0
#define CROSS_VERSION STR(CROSS_VERSION_MAJOR) \
                      "." STR(CROSS_VERSION_MINOR) \
                      "." STR(CROSS_VERSION_REVISION)
// Build
#define CROSS_BUILD_YEAR "2023"
#define CROSS_BUILD_MONTH "09"
#define CROSS_BUILD_DAY "29"
#define CROSS_BUILD CROSS_BUILD_YEAR \
                    CROSS_BUILD_MONTH \
                    CROSS_BUILD_DAY
/* Platform */
// Detect OS and architecture
#ifdef _WIN32
  #include <Windows.h>
  #include <process.h>


  #define CROSS_WIN
  #define CROSS_OS "Windows"
  #ifdef _WIN64
    #define CROSS_BITS "64"
  #else
    #define CROSS_BITS "32"
  #endif

#elif defined(__APPLE__) | defined(__MACH__)
  #include <unistd.h>
  #define CROSS_OS "MacOS"
  #define CROSS_BITS "64"

#elif defined(__linux__)
  #include <unistd.h>
  #define CROSS_OS "Linux"
  #define CROSS_BITS "64"
#endif

/* Locale */
#define CROSS_NEWLINE "\n"
#define CROSS_LOCALE ".UTF-8"

/* Colors */
// Regular
#define CLR "\033[0m"
#define BLK "\033[0;30m"
#define RED "\033[0;31m"
#define GRN "\033[0;32m"
#define YEL "\033[0;33m"
#define BLU "\033[0;34m"
#define MAG "\033[0;35m"
#define CYN "\033[0;36m"
#define WHT "\033[0;37m"
// High intensity
#define HBLK "\e[0;90m"
#define HRED "\e[0;91m"
#define HGRN "\e[0;92m"
#define HYEL "\e[0;93m"
#define HBLU "\e[0;94m"
#define HMAG "\e[0;95m"
#define HCYN "\e[0;96m"
#define HWHT "\e[0;97m"
// Bold
#define BBLK "\033[1;30m"
#define BRED "\033[1;31m"
#define BGRN "\033[1;32m"
#define BYEL "\033[1;33m"
#define BBLU "\033[1;34m"
#define BMAG "\033[1;35m"
#define BCYN "\033[1;36m"
#define BWHT "\033[1;37m"
// Bold High-Intensity
#define BHBLK "\033[1;90m"
#define BHRED "\033[1;91m"
#define BHGRN "\033[1;92m"
#define BHYEL "\033[1;93m"
#define BHBLU "\033[1;94m"
#define BHMAG "\033[1;95m"
#define BHCYN "\033[1;96m"
#define BHWHT "\033[1;97m"

/* Header */
#define CROSS_LOGO BHGRN "\
	       █████  █████\n\
	       ██        ██  \n\
	       █████  █████\n"

// Build
#define CROSS_PLATFORM CROSS_OS "(" CROSS_BITS "-bit)"
#define CROSS_BUILD_INFO WHT "Version " BWHT CROSS_VERSION \
                         WHT " Build " BWHT CROSS_BUILD \
                         WHT " on " BWHT CROSS_PLATFORM
// Copyright
#define CROSS_YEAR "2023"
#define CROSS_NEXT_YEAR "2024"
#define CROSS_AUTHOR "Michael Schutt"
#define CROSS_COPYRIGHT HBLK "© " BHBLK CROSS_YEAR "-" CROSS_NEXT_YEAR \
                        " " CROSS_AUTHOR HBLK ". All Rights Reserved."
// Header
#define CROSS_TITLE "Cross Interactive Command-Line Compiler Shell"
#define CROSS_HEADER CROSS_LOGO "\n" BHWHT CROSS_TITLE "\n" \
                     CROSS_BUILD_INFO "\n" CROSS_COPYRIGHT "\n"

#define CROSS_USAGE BHCYN "Usage" HCYN ": " WHT "$ " BWHT "cross " \
                    BBLK "[" WHT "<modules..>" \
                    BBLK "] [" WHT "<-flags..>" BBLK "]"

#define CROSS_ABOUT "\n" CROSS_HEADER "\n" CROSS_USAGE "\n" CLR

/* Logging */
#define CROSS_LOG_FMT_TIMESTAMP HBLK "[%s %s] "

#ifdef DEBUG
  /**
   * prints a message to stdout formatted
  */
  #define CROSS_LOG(msg, ...) \
    (printf( \
    CROSS_SHELL_PROMPT_LOG \
    CROSS_LOG_FMT_TIMESTAMP \
    HBLK msg CLR "\n", \
    __DATE__, __TIME__, ##__VA_ARGS__))
#else
  // Remove LOGs for release
  #define CROSS_LOG(msg, ...)
#endif

// Prompts
#define CROSS_SHELL_PROMPT_LOG BHBLK "$> " CLR
#define CROSS_SHELL_PROMPT_ERROR BHRED "!> " CLR
#define CROSS_SHELL_PROMPT_INPUT BHYEL ">> " HWHT
#define CROSS_SHELL_PROMPT_OUTPUT BHGRN "=> " GRN 

/* Errors */
// Types
#define CROSS_ERROR_TYPE_IO "IO"
#define CROSS_ERROR_TYPE_SYNTAX "Syntax"
#define CROSS_ERROR_TYPE_SYSTEM "System"
// Errors
#define CROSS_ERROR_CODE_UNKNOWN_ERROR 0x1
#define CROSS_ERROR_NAME_UNKNOWN_ERROR "Unknown Error"
#define CROSS_ERROR_MSG_UNKNOWN_ERROR "An unknown error has occurred."

#define CROSS_ERROR_CODE_MODULE_NOT_FOUND 0x2
#define CROSS_ERROR_NAME_MODULE_NOT_FOUND "Module Not Found"
#define CROSS_ERROR_MSG_MODULE_NOT_FOUND \
        "The module " BHRED "%s" RED " cannot be found."
// Format
#define CROSS_ERROR_FMT_POS HRED "%s(" RED "%d,%d)"
#define CROSS_ERROR_FMT_TYPE BRED "%s Error"
#define CROSS_ERROR_FMT_NAME BRED "[" RED "%s" BRED "]"
#define CROSS_ERROR_FMT_LINE BBLK "\n%*d|" BHWHT " %s\n" 
#define CROSS_ERROR_FMT_ARROW BHRED "      ^"

/**
 * Prints an error on the screen formatted
 */
#define CROSS_ERROR(file, line, col, type, name, src, fatal, ...) \
        do { \
          fprintf(stderr, CROSS_SHELL_PROMPT_ERROR \
            CROSS_ERROR_FMT_POS " " \
            CROSS_ERROR_FMT_TYPE " " CROSS_ERROR_FMT_NAME "\n" \
            RED CROSS_ERROR_MSG_##name \
            CROSS_ERROR_FMT_LINE CROSS_ERROR_FMT_ARROW "\n" CLR, \
            file, line, col, CROSS_ERROR_TYPE_##type, \
            CROSS_ERROR_NAME_##name, 4, line, src, ##__VA_ARGS__); \
            if (fatal) { exit(0); } \
        } while (0)

/* Forward Declarations (for Shell) */
void CROSS_onExit(void);
void Cross_handleSignal(const int signal);

/* Loader */
typedef FILE* Module;

static Module Loader_loadModule(const char *path) {
  Module module = NULL;

  CROSS_LOG("Loader(loadModule): Loading module \"%s\"...", path);
  if ((module = fopen(path, "r")) == NULL) 
    CROSS_ERROR(path, 1, 1, IO, MODULE_NOT_FOUND, path, true, path);

  CROSS_LOG("Loader(loadModule): Module \"%s\" loaded at 0x%p\n", path, module);
  return module;
}

static void Loader_freeModule(Module module) {
  CROSS_LOG("Loader(freeModule): Freeing module at 0x%p...", module);
  if (module != NULL) fclose(module);
  CROSS_LOG("Load(freeModule): Module freed");
}

/* Shell */
// Modes
#define CROSS_SHELL_MODE_COMPILE 0x0
#define CROSS_SHELL_MODE_INTERACTIVE 0x1
// Commands
#define CROSS_SHELL_SLEEP_SECS 2
#define CROSS_SHELL_CMD_CLSCR "\e[1;1H\e[2J"
#define CROSS_SHELL_CMD_SET_TITLE "\033]0;%s\007"
#define CROSS_SHELL_MSG_PAUSE CYN "Press ENTER to continue..." CLR

/* REPL */
#define CROSS_REPL_BUF_SIZE 4096 
#define CROSS_REPL_MSG_EXIT_HELP BHCYN "Press CTRL+C to exit\n"

/**
 * Starts main REPL loop 
 * NOTE: Must use SIGINT to exit
*/
static void Repl_loop(void) {
  // our input line buffer
  char input[CROSS_REPL_BUF_SIZE];
  puts(CROSS_REPL_MSG_EXIT_HELP);

  // Start our REPL loop (an INFINITE loop! o_O)
  while (true) {
    printf(CROSS_SHELL_PROMPT_INPUT);
    fgets(input, CROSS_REPL_BUF_SIZE, stdin);
    input[strcspn(input, "\n")] = '\0';
    if (!strlen(input)) continue; 
    printf("%s%s\n", CROSS_SHELL_PROMPT_OUTPUT, input);
  }
}

/**
 * Puts the Shell to sleep for a given amount of seconds
*/
static void Shell_sleep(unsigned int secs) {
  // Don't sleep on release builds
  #ifndef DEBUG
    return;
  #endif
  
  #ifdef CROSS_WIN
    // WinAPI uses Sleep(ms)
    Sleep(secs * 1000);
  #else
    sleep(secs);
  #endif
}

/**
 * Clears the Shell's screen
*/
static void Shell_clear(void) {
  CROSS_LOG("Shell(clear): Clearing screen...");
  Shell_sleep(CROSS_SHELL_SLEEP_SECS / 2);
  fprintf(stdout, "%s%s", CROSS_SHELL_CMD_CLSCR, CLR);
  fflush(stdout);
}

/**
 * Pauses the Shell and waits for ENTER
*/
static void Shell_pause(void) {
  CROSS_LOG("Shell(pause): Cross Shell paused");
  printf(CROSS_SHELL_MSG_PAUSE);
  fgetc(stdin);
  fprintf(stdout, "\n");
}

/**
 * Sets the console window's title 
 * const char *title  - The title text to set
*/
static void Shell_setTitle(const char *title) {
  CROSS_LOG("Shell(setTitle): Setting terminal title...");
  #ifdef CROSS_WIN
    // on Windows we use SetConsoleTitle
    // HANDLE hWnd = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitleA(title);
  #else // Nix and MacOS
    printf(CROSS_SHELL_CMD_SET_TITLE, title); 
  #endif
}

/**
 * Initializes Cross runtime by setting up environment
*/
static void Shell_init(void) {
  // Setup terminal environment
  CROSS_LOG("Shell(init): Setting Cross locale to Utf8...");
  setlocale(LC_ALL, CROSS_LOCALE);

  // Windows uses some weird codepage by default...
  #ifdef CROSS_WIN
    CROSS_LOG("Shell(init): Setting Windows CodePage to Utf8...");
    SetConsoleOutputCP(CP_UTF8);
  #endif
  
  // Register global handlers
  CROSS_LOG("Shell(init): Registering exit handlers...");
  atexit(CROSS_onExit);
  
  CROSS_LOG("Shell(init): Registering signal handlers...");
  signal(SIGINT, Cross_handleSignal);
}

/* Starts the Cross Shell and sets Shell mode if no args
 * starts in interactive mode and launches REPL loop. 
 * otherwise, passes CLI args and main module to compiler 
 * int argc - argc from main (count of cli args)
 * char **argv - argv from main (cli args as cstring array)
*/
static void Shell_start(int argc, char **argv) {
  CROSS_LOG("Shell(start): Cross Shell started with args:");
  for (int i = 0; i < argc; i++) {
    CROSS_LOG("argv[%d] = \"%s\"", i, argv[i]);
  }

  // If there are no arguments, start in interactive mode
  switch (argc) {
    case CROSS_SHELL_MODE_INTERACTIVE:
      CROSS_LOG("Shell(start): Cross Shell mode set to INTERACTIVE");
      CROSS_LOG("Shell(start): Starting REPL...");
      return Repl_loop(); 

    default:
      CROSS_LOG("Shell(start): Cross Shell mode set to COMPILE");
      CROSS_LOG("Shell(start): Invoking compiler with main module \"%s\"...", argv[1]); 

      // Load module and free as a test
      Module module = Loader_loadModule("test.cross");
      Loader_freeModule(module);
      Shell_pause();
      return;
  }
}

/**
 * Attaches the Cross Shell to the host process and terminal
 * int argc - argc from main (count of cli args)
 * char **argv - argv from main (cli args as cstring array)
*/
static void Shell_attach() {
  Shell_clear();
  CROSS_LOG("Shell(attach): Attaching Cross Shell to host terminal process...");
  Shell_setTitle(CROSS_TITLE);
  Shell_init();
  puts(CROSS_HEADER);
  CROSS_LOG("Shell(attach): Cross Shell attached, starting Shell...\n");
}

/**
 * Detaches the Cross Shell from the host process and terminal
*/
static void Shell_detach(void) {
  Shell_clear();
  CROSS_LOG("Shell(detach): Detaching Cross Shell from host terminal process...");
  Shell_sleep(CROSS_SHELL_SLEEP_SECS);
  Shell_clear();
}

/* Global Handlers */
// Signals 
/**
 * Handles OS signals such as SIGINT and calls their handler
*/
void Cross_handleSignal(const int signal) {
  CROSS_LOG("Cross(handleSignal): Received signal %d", signal);
  switch (signal) {
    case SIGINT: return exit(EXIT_SUCCESS);
  }
}

// Exit 
/**
 * Handles cleanup and detachment of shell on program exit
 * before Cross returns control back to the host OS
*/
void CROSS(onExit)(void) {
  CROSS_LOG("Cross(onExit): Running exit handlers...");
  Shell_detach();
}

/**
* Main entry point into the Cross process. Attaches the Cross Shell
* to the terminal process of the host OS and sets up locale and other
* platform specific settings. Main interface for end users into Cross.
* int argc - The ARGument Count of CLI args passed to the process
* char **argv - The ARGument Vector of cstrings that contains the CLI arguments
* returns EXIT_SUCCESS on succesful exit, otherwise an CROSS_ERROR_* code
*/
int main(int argc, char **argv) {
  CROSS_LOG("Cross(main): Initializing Cross...");
  Shell_attach();
  Shell_start(argc, argv);
  CROSS_LOG("Cross(atExit): Exiting program...");
  return EXIT_SUCCESS;
}
