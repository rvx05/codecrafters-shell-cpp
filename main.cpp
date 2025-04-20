#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <sys/stat.h>

enum CommandType
{ 
  Builtin, 
  Executable, 
  Invalid
};

struct FullCommandType 
{ 
  CommandType type; 
  std::string executable_path;
};

int findLastIndex (std::string&, char);
std::vector<std::string> parse_command_to_string_vector (std::string);
FullCommandType command_to_full_command_type (std::string);
std::string find_command_executable_path (std::string);
std::string find_command_in_path (std::string, std::string);

int main() 
{
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) 
  {
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);

    std::vector<std::string> command_vector = parse_command_to_string_vector(input);
    if (command_vector.size() == 0) continue;

    FullCommandType fct = command_to_full_command_type(command_vector[0]);

    if (fct.type == Builtin) 
    {
      if (command_vector[0] == "exit")
      {
        int exit_code = std::stoi(command_vector[1]);
        return exit_code;
      }

      else if (command_vector[0] == "echo")
      {
        for (int i = 1; i < command_vector.size(); i++) 
        {
          if (command_vector[i][0] == '\'')
          {
            std::cout << command_vector[i].substr(1, command_vector[i].size() - 2);
          }
          else std::cout << command_vector[i];
          if (i != command_vector.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
      }

      else if (command_vector[0] == "type")
      {
        if (command_vector.size() < 2) continue;

        std::string command_name = command_vector[1];
        FullCommandType command_type = command_to_full_command_type(command_name);

        switch (command_type.type) 
        {
          case Builtin:
            std::cout << command_name << " is a shell builtin\n";
            break;

          case Executable:
            std::cout << command_name << " is " << command_type.executable_path << "\n";
            break;

          case Invalid:
            std::cout << command_name << " not found\n";
            break;
            
          default:
            break;
        }
      }

      else if (command_vector[0] == "pwd")
      {
        std::string curr_dir = std::filesystem::current_path();
        std::cout << curr_dir << std::endl;
      }
      
      else if (command_vector[0] == "cd")
      {
        std::string new_dir = command_vector[1];

        if (new_dir == "~") new_dir = getenv("HOME");

        if (std::filesystem::exists(new_dir)) std::filesystem::current_path(new_dir);

        else std::cout << command_vector[0] << ": " << new_dir << ": " << "No such file or directory" << std::endl;        
      }
    }

    else if (fct.type == Executable) 
    {
      std::string command_with_full_path = fct.executable_path.substr(findLastIndex(fct.executable_path, '/') + 1);
      for (int argn = 1; argn < command_vector.size(); argn++) 
      {
        command_with_full_path += " ";
        command_with_full_path += command_vector[argn];
      }
      const char *command_ptr = command_with_full_path.c_str();
      system(command_ptr);
    }
    
    else if (command_vector[0] == "cat")
    {
      system(input.c_str());
    }

    else 
    {
      std::cout << input << ": command not found\n";
    }
  }
}

std::vector<std::string> parse_command_to_string_vector(std::string command) 
{
  std::vector<std::string> args;
  std::string arg_acc = "";
  
  bool quote = false;

  for (char c : command) 
  {
    if (c == '\'')
    {
      quote = !quote;
      arg_acc += c;
      if (!quote)
      {
        args.push_back(arg_acc);
        arg_acc = "";
      }
    }
    else if (c == ' ' && !quote) 
    {
      if (arg_acc == "") continue;
      args.push_back(arg_acc);
      arg_acc = "";
    } 
    else 
    {
      arg_acc += c;
    }
  }

  if (arg_acc != "") args.push_back(arg_acc);

  return args;
}

FullCommandType command_to_full_command_type(std::string command) 
{
  FullCommandType fct;
  fct.type = CommandType::Invalid;

  std::vector<std::string> builtin_commands = {"exit", "echo", "type", "pwd", "cd"};

  if (std::find(builtin_commands.begin(), builtin_commands.end(), command) != builtin_commands.end()) 
  {
    fct.type = CommandType::Builtin;
    return fct;
  }

  std::string exec_path = find_command_executable_path(command);
  if (exec_path != "") 
  {
    fct.type = Executable;
    fct.executable_path = exec_path;
    return fct;
  }

  return fct;
}

std::string find_command_executable_path(std::string command) 
{
  char *path = getenv("PATH");

  if (path == NULL) return "";

  std::string path_acc = "";
  char *p = path;

  while (*p != '\0') 
  {
    if (*p == ':') 
    {
      std::string exec_path = find_command_in_path(command, path_acc);
      if (exec_path != "") return exec_path;
      path_acc = "";
    } 
    else 
    {
      path_acc += *p;
    }
    p++;
  }

  std::string exec_path = find_command_in_path(command, path_acc);
  
  return exec_path;
}

std::string find_command_in_path(std::string command, std::string path) 
{
  for (const auto &entry : std::filesystem::directory_iterator(path)) 
  {
    if (entry.path() == (path + "/" + command)) return entry.path();
  }
  return "";
}

int findLastIndex(std::string& str, char x) 
{
  int index = -1;
  for (int i = str.length() - 1; i >= 0; --i)
  {
    if (str[i] == x)
    {
      index = i;
      break;
    }
  }
  return index;
}
