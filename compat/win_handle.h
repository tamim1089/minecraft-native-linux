// (threads, events, files). A single CloseHandle() deletes through the virtual destructor,
// so each handle kind cleans itself up correctly regardless of which subsystem made it.
#pragma once
#ifndef PORT_WIN_HANDLE_H
#define PORT_WIN_HANDLE_H

struct WinHandleBase {
    enum { K_THREAD = 1, K_EVENT = 2, K_FILE = 3 };
    int kind = 0;
    virtual ~WinHandleBase() {}
};

#endif // PORT_WIN_HANDLE_H
