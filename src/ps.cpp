#ifdef QT_GUI 

#include "ps.hpp"

#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

/**
 * @brief Construct a new Ps:: Ps object
 *
 */
Ps::Ps()
{
}

/**
 * @brief Destroy the Ps:: Ps object
 *
 */
Ps::~Ps()
{
}


/**
 * @brief this method will read the entire past file
 *
 * @param p_name name for read file
 * @param p_buffer in which variable will you store
 * @param p_nblock block size for reading
 * @param p_blockm if this option is turned on the block size will multiply by 2 per p_nblock
 *
 * @return long, if reading file success return READ_SUCCESS else if not len READ_FAIL not open file return OPEN_FAIL
 */
void Ps::read_file_system_linux ( std::string& p_name, std::string &p_buffer, long p_nblock = 256, bool p_blockm )
{

  p_buffer.clear();

  off_t nblock = p_nblock;
  const char *name = p_name.data();

  p_buffer.reserve ( p_nblock );
  int FS = open ( name, O_RDONLY );

  if ( FS > 0 )
  {
    do
    {
      char buffer[nblock];
      memset ( buffer, 0, nblock ); // clean buffer array

      if ( read ( FS, buffer, nblock ) == 0 )
        break;

      p_buffer += buffer; // save block in variable p_buffer

      // increase the bytes of the block thus decreasing the read cycles
      // it could possibly end up exceeding the file buffer limit by allocating more than necessary
      ( p_blockm ) ? nblock += p_nblock : nblock;

    } while ( FS != EOF );

    if ( p_buffer.size() == 0 )
      throw std::runtime_error ( "Not possible read file, check permission " + p_name );


    close ( FS );
  }
  else
    throw std::runtime_error ( "Error open file " + p_name );

}

/**
 * @brief will get all processes from the magic folder / proc
 * will store in the parameters the name and pid equivalent to the process
 *
 * @param NameProcess pass an array as a parameter to store process names
 * @param PidProcess pass an array as a parameter to store process pids
 *
 */
void Ps::ps_proc(std::unordered_map<std::string, infos> &p_umap)
{
    infos infos_pid;
    p_umap.clear();

    DIR *dir = opendir("/proc/");
    struct dirent *dir_read;

    if (dir == nullptr)
    {
        throw std::runtime_error("Error not open dir /proc");
    }

    while ((dir_read = readdir(dir)) != nullptr)
    {
        std::string file;
        if (strcmp(dir_read->d_name, "self") && strcmp(dir_read->d_name, "thread-self"))
        {
            try
            {
                file = "/proc/" + std::string(dir_read->d_name) + "/comm";
                read_file_system_linux(file, infos_pid.name, 20);
                infos_pid.pid =  dir_read->d_name;
                
                p_umap.insert(std::make_pair(infos_pid.name, infos_pid));
            }
            catch (std::exception &error)
            {
                file.clear();
            }
        }
    }

    closedir(dir);
}

#endif