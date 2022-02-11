#include "sessionhandler.h"

SessionHandler::SessionHandler()
{

}

SessionHandler::~SessionHandler()
{
    ssh_disconnect(session);
    ssh_free(session);
}


int SessionHandler::verify_knownhost()
{
    enum ssh_known_hosts_e state;
    unsigned char *hash = NULL;
    ssh_key srv_pubkey = NULL;
    size_t hlen;
    char buf[10];
    char *hexa;
    char *p;
    int cmp;
    int rc;

    rc = ssh_get_server_publickey(session, &srv_pubkey);
    if (rc < 0) {
        return -1;
    }

    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA1,
                                &hash,
                                &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return -1;
    }

    state = ssh_session_is_known_server(session);
    switch (state) {
        case SSH_KNOWN_HOSTS_OK:
            /* OK */

            break;
        case SSH_KNOWN_HOSTS_CHANGED:
            fprintf(stderr, "Host key for server changed: it is now:\n");
            ssh_print_hexa("Public key hash", hash, hlen);
            fprintf(stderr, "For security reasons, connection will be stopped\n");
            ssh_clean_pubkey_hash(&hash);

            return -1;
        case SSH_KNOWN_HOSTS_OTHER:
            fprintf(stderr, "The host key for this server was not found but an other"
                    "type of key exists.\n");
            fprintf(stderr, "An attacker might change the default server key to"
                    "confuse your client into thinking the key does not exist\n");
            ssh_clean_pubkey_hash(&hash);

            return -1;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
            fprintf(stderr, "Could not find known host file.\n");
            fprintf(stderr, "If you accept the host key here, the file will be"
                    "automatically created.\n");

            /* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */

        case SSH_KNOWN_HOSTS_UNKNOWN:
            hexa = ssh_get_hexa(hash, hlen);
            fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
            fprintf(stderr, "Public key hash: %s\n", hexa);
            ssh_string_free_char(hexa);
            ssh_clean_pubkey_hash(&hash);
            p = fgets(buf, sizeof(buf), stdin);
            if (p == NULL) {
                return -1;
            }

            cmp = strncasecmp(buf, "yes", 3);
            if (cmp != 0) {
                return -1;
            }

            rc = ssh_session_update_known_hosts(session);
            if (rc < 0) {
                fprintf(stderr, "Error %s\n", strerror(errno));
                return -1;
            }

            break;
        case SSH_KNOWN_HOSTS_ERROR:
            fprintf(stderr, "Error %s", ssh_get_error(session));
            ssh_clean_pubkey_hash(&hash);
            return -1;
    }

    ssh_clean_pubkey_hash(&hash);
    return 0;
}

int SessionHandler::list_files()
{
    ssh_channel channel;
    int rc, i, j = 0, k, space_counter;
    char buffer[256];
    char item_buffer[256];
    bool dir, ignore_first = true;
    char* item_name;
    int nbytes;
    std::string quotedPath;

    strcpy(buffer,"ls -l --group-directories-first ");
    //Le agrego comillas a la dirección para evitar errores en archivos con espacios en el nombre
    strcat(buffer,quoteString(currentPath).c_str());

    file_list.clear(); //LIMPIA LA LISTA ANTES DE AGREGARLE

    channel = ssh_channel_new(session);
    if (channel == NULL)
      return SSH_ERROR;

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK)
    {
      ssh_channel_free(channel);
      return rc;
    }

    rc = ssh_channel_request_exec(channel, buffer);
    if (rc != SSH_OK)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return rc;
    }

    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0)
    {
        for(i  = 0; i < nbytes; i++)
        {
            item_buffer[j] = buffer[i];
            j++;
            if(buffer[i] == '\n')
            {
                if(ignore_first) ignore_first = false;
                else
                {
                    item_buffer[j-1] = '\0';                //j-1 PARA BORRAR EL ÚLTIMO SALTO DE LÍNEA
                    //item_name = strrchr(item_buffer,' ')+1; //+1 PARA QUE NO GUARDE EL ESPACIO QUE DEJA ADELANTE DEL NOMBRE

                    space_counter = 0;
                    k = 0;
                    while((space_counter <= 7) && (k < j-1))
                    {
                        if((item_buffer[k] == ' ') && (item_buffer[k+1] != ' ')) space_counter++;
                        k++;
                    }
                    item_name = &item_buffer[k];

                    if (item_buffer[0] == 'd') dir = true;
                    else dir = false;

                    file_list.push_back(new Item{item_name,dir});
                }
                j = 0;
            }
        }
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return SSH_OK;
}

std::list<Item*> SessionHandler::get_file_list()
{
    return this->file_list;
}

int SessionHandler::show_remote_processes(const char* cmd)
{
      ssh_channel channel;
      int rc;
      char buffer[256];
      int nbytes;

      channel = ssh_channel_new(session);
      if (channel == NULL)
        return SSH_ERROR;

      rc = ssh_channel_open_session(channel);
      if (rc != SSH_OK)
      {
        ssh_channel_free(channel);
        return rc;
      }

      rc = ssh_channel_request_exec(channel, cmd);
      if (rc != SSH_OK)
      {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return rc;
      }

      nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
      while (nbytes > 0)
      {
        if (write(1, buffer, nbytes) != (unsigned int) nbytes)
        {
          ssh_channel_close(channel);
          ssh_channel_free(channel);
          return SSH_ERROR;
        }
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
      }

      if (nbytes < 0)
      {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
      }

      ssh_channel_send_eof(channel);
      ssh_channel_close(channel);
      ssh_channel_free(channel);

      return SSH_OK;
}

int SessionHandler::start_session(std::string user, std::string host, std::string password, std::string port)
{
    int rc;

    // Open session and set options
    session = ssh_new();
    if (session == NULL)
    exit(-1);
    ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());
    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session, SSH_OPTIONS_PORT_STR, port.c_str());

    // Connect to server
    rc = ssh_connect(session);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error connecting to localhost: %s\n",
            ssh_get_error(session));
        ssh_free(session);
        return -1;
    }

    // Verify the server's identity
    // For the source code of verify_knownhost(), check previous example
    if (verify_knownhost() < 0)
    {
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    // Authenticate ourselves
    rc = ssh_userauth_password(session, NULL, password.c_str());
    if (rc != SSH_AUTH_SUCCESS)
    {
        fprintf(stderr, "Error authenticating with password: %s\n",
            ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        return -1;
    }

    this->userName = user;

    return 0;
}

int SessionHandler::scp_upload(std::string pathToFile, QProgressBar *pbar)
{
    ssh_scp scp;
    int rc;

    scp = ssh_scp_new
    (session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, ".");
    if (scp == NULL)
    {
        fprintf(stderr, "Error allocating scp session: %s\n",
            ssh_get_error(session));
        return SSH_ERROR;
    }

    rc = ssh_scp_init(scp);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error initializing scp session: %s\n",
            ssh_get_error(session));
        ssh_scp_free(scp);
        return rc;
    }


    std::string fileName = pathToFile;
    fileName.erase(0,fileName.find_last_of('/')+1);

    //Para subir no hace falta agregar comillas para evitar errores de espacios

    int fileSize = std::filesystem::file_size(pathToFile);
    std::ifstream inputFile(pathToFile,std::ios::binary);
    char buffer[2048];

    rc = ssh_scp_push_file(scp, fileName.c_str(), fileSize, S_IRUSR |  S_IWUSR);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Can't open remote file: %s\n",
            ssh_get_error(session));
        return rc;
    }

    unsigned int count = 0;
    while (inputFile)
    {
        inputFile.read(buffer, sizeof(buffer));
        rc = ssh_scp_write(scp, buffer, sizeof(buffer));
        if (rc != SSH_OK)
        {
            fprintf(stderr, "Cant write to remote file: %s\n", ssh_get_error(session));
            return rc;
        }
        count++;
        pbar->setValue(count*sizeof(buffer)*100/fileSize);
        QApplication::processEvents();
    }

    std::cout << "File uploaded" << std::endl;

    ssh_scp_close(scp);
    ssh_scp_free(scp);
    return SSH_OK;
}

int SessionHandler::scp_download(std::string fileName, QProgressBar *pbar)
{
    //Para descargar tampoco hay problema con los espacios
    ssh_scp scp;
    int rc;
    int fileSize, mode;
    unsigned int count = 0;
    char buffer[256];
    std::string filePath = fileName;
    std::string downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toStdString();

    downloadPath.append("/");
    downloadPath.append(fileName);
    std::ofstream outputFile(downloadPath,std::ios::binary);

    filePath.insert(0,1,'/');
    filePath.insert(0,this->getCurrentPath());

    scp = ssh_scp_new(session, SSH_SCP_READ, filePath.c_str());
    if (scp == NULL)
    {
        fprintf(stderr, "Error allocating scp session: %s\n",ssh_get_error(session));
        return SSH_ERROR;
    }

    rc = ssh_scp_init(scp);
    if (rc != SSH_OK)
    {
        fprintf(stderr, "Error initializing scp session: %s\n",ssh_get_error(session));
        ssh_scp_free(scp);
        return rc;
    }

    rc = ssh_scp_pull_request(scp);
    if (rc != SSH_SCP_REQUEST_NEWFILE)
    {
        fprintf(stderr, "Error receiving information about file: %s\n",ssh_get_error(session));
        return SSH_ERROR;
    }

    fileSize = ssh_scp_request_get_size(scp);
    mode = ssh_scp_request_get_permissions(scp);
    printf("Receiving file %s, size %d, permissions 0%o\n",fileName.c_str(), fileSize, mode);

    ssh_scp_accept_request(scp);
    rc = ssh_scp_read(scp, buffer, sizeof(buffer));
    outputFile.write(buffer, rc);
    count++;
    pbar->setValue(count*sizeof(buffer)*100/fileSize);
    QApplication::processEvents();
    while(rc >= (int)sizeof(buffer))
    {
        rc = ssh_scp_read(scp, buffer, sizeof(buffer));
        if (rc != SSH_ERROR)
        {
            outputFile.write(buffer, rc);
            count++;
            pbar->setValue(count*sizeof(buffer)*100/fileSize);
            QApplication::processEvents();
        }
        else rc++;
    }
    outputFile.close();
    if (rc < 0)
    {
        fprintf(stderr, "Error receiving file data: %s\n",
            ssh_get_error(session));
        return rc;
    }
    std::cout << "Done" << std::endl;

    rc = ssh_scp_pull_request(scp);
    if (rc != SSH_SCP_REQUEST_EOF)
    {
        fprintf(stderr, "Unexpected request: %s\n",
            ssh_get_error(session));
        return SSH_ERROR;
    }


    ssh_scp_close(scp);
    ssh_scp_free(scp);
    return SSH_OK;
}

std::string SessionHandler::getUserName()
{
    return this->userName;
}

std::string SessionHandler::getCurrentPath()
{
    return this->currentPath;
}

void SessionHandler::back_path()
{
    std::size_t pos = currentPath.find_last_of('/');
    currentPath.erase(pos,currentPath.length());
    if(strcmp(currentPath.c_str(),"/home") == 0)
    {
        currentPath.append("/");
        currentPath.append(this->userName);
    }
}

int SessionHandler::move_path(const char* folder)
{
    bool exists = false;
    for(std::list<Item*>::iterator it = file_list.begin(); it != file_list.end(); it++)
    {
        if(strcmp((*it)->name.c_str(),folder) == 0) exists = true;
    }
    if(exists)
    {
        currentPath.append("/");
        currentPath.append(folder);
    }else return -1;

    return 0;
}

std::string SessionHandler::quoteString(std::string string)
{
    std::string ret = string;
    ret.insert(0,1,'"');
    ret.append("\"");
    return ret;
}
