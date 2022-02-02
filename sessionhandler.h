#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <libssh/libssh.h>
#include <string>
#include <string.h>

class SessionHandler
{
private:
    ssh_session session;
public:
    SessionHandler();
    int verify_knownhost();
    int show_remote_processes();
    int start_session(std::string user, std::string host, std::string password);
    ~SessionHandler();
};

#endif // SESSIONHANDLER_H
