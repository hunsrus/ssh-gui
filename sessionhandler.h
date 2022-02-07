#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <libssh/libssh.h>
#include <string>
#include <string.h>
#include <iostream>
#include <list>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <QStandardPaths>
#include <QProgressBar>
#include <QApplication>

typedef struct
{
    std::string name;
    bool dir;
}Item;

class SessionHandler
{
private:
    ssh_session session;
    std::list<Item*> file_list;
    std::string userName;
    std::string currentPath = "/home";
public:
    SessionHandler();
    int verify_knownhost();
    int show_remote_processes(const char* cmd);
    int start_session(std::string user, std::string host, std::string password);
    int list_files();
    void back_path();
    int move_path(const char* folder);
    std::list<Item*> get_file_list();
    int scp_upload(std::string pathToFile, QProgressBar *pbar);
    int scp_download(std::string fileName, QProgressBar *pbar);
    std::string getUserName();
    std::string getCurrentPath();
    ~SessionHandler();
};

#endif // SESSIONHANDLER_H
