#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

enum CommandType
{ 
  Builtin, 
  Executable, 
  Nonexistent
};

struct FullCommandType 
{ 
  CommandType type; 
  std::string executable_path;
};

int findLastIndex(string&, char);
std::vector<std::string> parse_command_to_string_vector(std::string command);
FullCommandType command_to_full_command_type(std::string command);
std::string find_command_executable_path(std::string command);
std::string find_command_in_path(std::string command, std::string path);

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
      if (command_vector[0] == "echo") 
      {
        for (int i = 1; i < command_vector.size(); i++) 
        {
          if (i != 1) std::cout << " ";
          std::cout << command_vector[i];
        }
        std::cout << "\n";
        continue;
      }
      if (command_vector[0] == "type") 
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
          case Nonexistent:
            std::cout << command_name << " not found\n";
            break;
          default:
            break;
        }
        continue;
      }
      continue;
    }
    if (fct.type == Executable) 
    {
      std::string command_with_full_path = fct.executable_path.substr(findLastIndex(fct.executable_path, '/') + 1);
      for (int argn = 1; argn < command_vector.size(); argn++) 
      {
        command_with_full_path += " ";
        command_with_full_path += command_vector[argn];
      }
      const char *command_ptr = command_with_full_path.c_str();
      system(command_ptr);
      continue;
    }
    std::cout << input << ": command not found\n";
  }
}

std::vector<std::string> parse_command_to_string_vector(std::string command) 
{
  std::vector<std::string> args;
  std::string arg_acc = "";
  for (char c : command) 
  {
    if (c == ' ') 
    {
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
  std::vector<std::string> builtin_commands = {"exit", "echo", "type"};
  if (std::find(builtin_commands.begin(), builtin_commands.end(), command) != builtin_commands.end()) 
  {
    FullCommandType fct;
    fct.type = CommandType::Builtin;
    return fct;
  }
  std::string exec_path = find_command_executable_path(command);
  if (exec_path != "") 
  {
    FullCommandType fct;
    fct.type = Executable;
    fct.executable_path = exec_path;
    return fct;
  }
  FullCommandType fct;
  fct.type = CommandType::Nonexistent;
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
  if (exec_path != "") return exec_path;
  return "";
}

std::string find_command_in_path(std::string command, std::string path) 
{
  for (const auto &entry : std::filesystem::directory_iterator(path)) 
  {
    if (entry.path() == (path + "/" + command)) return entry.path();
  }
  return "";
}

int findLastIndex(string& str, char x) 
{
  int index = -1;
  for (int i = 0; i < str.length(); i++)
    if (str[i] == x) index = i;
  return index;
}
