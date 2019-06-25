#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <sys/stat.h>
#include <ctime>
#include <unistd.h>

//set to 1 to enable some debug std::cout statements
#define DEBUG 0

#define TCOIN_PATH "/home/login/tcoin"
#define TCOIN_MSG_PATH "/home/login/tcoin/messages/"
#define TCOIN_SALT_PATH "/home/login/tcoin/salts/"
#define TCOIN_PASS_PATH "/home/login/tcoin/passwords/"
#define TCOIN_PROG_ACT_PATH "/home/login/tcoin/program_accounting/"
#define PROG_ACT_W_SLASH "program_accounting/"
#define PCOIN_KEY_PATH "/home/login/bin/pcoin_keys"
#define TCOIN_CODEZ_PATH "/home/login/bin/tcoin_codez"
#define TCOIN_BIN_PATH_W_SPACE "/home/login/bin/tcoin "
#define PCOIN_BIN_PATH "/home/login/bin/pcoin"
#define PCOIN_BIN_PATH_W_SPACE "/home/login/bin/pcoin "
#define TCOIN_PATH_W_SLASH "/home/login/tcoin/"
#define TCOIN_SCRYPT_PATH "/home/login/bin/scrypt"
#define LS_HOME_CMD "/bin/ls /home"
#define BIN_ECHO_CMD "/bin/echo $$"
#define KROWBAR_SCORE_PATH "/home/krowbar/Code/irc/data/tildescores.txt"
#define WHOAMI_PATH "/usr/bin/whoami"
#define TROIDO_DACOINS_CMD "cd /home/troido/daily_adventure/client/ && /home/troido/daily_adventure/client/daclient printinfo 2>&1 | /bin/grep -oP '(?<=\"Coins\", )\[[:digit:]]+'"
#define MINERCOIN_CMD_PRE_USERNAME "/bin/grep -oP '(?<=\"~"
#define MINERCOIN_CMD_POST_USERNAME "\": )[[:digit:]]+' /home/minerobber/Code/minerbot/minercoin.json"
#define USERNAME_LENGTH_LIMIT 25

void exit_program(const int error_number)
{
  // Cleanup to do before exiting the program

  // Finally, we can exit
  std::exit(error_number);
}

//custom function to convert ("abcd.de") to ("abcde")
long long int strtol100(const char* amount_str)
{
  long long int result = 0;
  int multiplier = 1;
  int i=0;
  if(amount_str[i]=='-')
  {
    multiplier = -1;
    ++i;
  }
  else if(amount_str[i]=='+')
  {
    ++i;
  }
  else if(amount_str[i]=='\0') //empty string
  {
    return (long long int)(0);
  }
  //before the decimal point
  while(amount_str[i]!='.')
  {
    if(amount_str[i]>='0' && amount_str[i]<='9')
      result = result*10 + ((long long int)(amount_str[i]) - (long long int)('0'));
    else if(amount_str[i]=='\0') //e.g. "500"
    {
      result *= (multiplier*100); //multiplied by 100 to get centitildecoins
      return result;
    }
    else //error
    {
      return (long long int)(0);
    }
    ++i;
  }
  //at decimal point
  ++i;
  //after decimal point (i.e., tenth's place)
  if(amount_str[i]>='0' && amount_str[i]<='9')
    result = result*100 + ((long long int)(amount_str[i]) - (long long int)('0'))*10;
  else if(amount_str[i]=='\0') //e.g. "500."
  {
    result *= (multiplier*100); //multiplied by 100 to get centitildecoins
    return result;
  }
  else //error
  {
    return (long long int)(0);
  }
  //before hundredth's place
  ++i;
  //at hundredth's place
  if(amount_str[i]>='0' && amount_str[i]<='9')
  {
    result += ((long long int)(amount_str[i]) - (long long int)('0'));
  }
  else if(amount_str[i]=='\0') //e.g. "500.3"
  {
    result *= multiplier;
    return result;
  }
  else //error
  {
    return (long long int)(0);
  }
  result *= multiplier;
  return result;
}

//custom function to convert integer string to long long int fast
long long int strtol_fast(const char* amount_str)
{
  long long int result = 0;
  int multiplier = 1;
  int i=0;
  if(amount_str[i]=='-')
  {
    multiplier = -1;
    ++i;
  }
  else if(amount_str[i]=='+')
  {
    ++i;
  }
  else if(amount_str[i]=='\0') //empty string
  {
    return (long long int)(0);
  }
  //before the end of the string
  while(amount_str[i]>='0' && amount_str[i]<='9')
  {
    result = result*10 + ((long long int)(amount_str[i]) - (long long int)('0'));
    ++i;
  }
  result *= multiplier;
  return result;
}

//string constant time compare (only checks for equality (return 0 if equal))
int strctcmp(const char*a, const char*b)
{
  if(!(*a) || !(*b)) //a or b are empty (start with a NULL character)
    return 1;

  int r = 0;
  for (; *a && *b; ++a, ++b)
  {
    r |= *a != *b;
  }
  return r;
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

long long int get_file_value(const char* file_name)
{
  char* file_path = new char[strlen(file_name)+strlen(TCOIN_PATH_W_SLASH)+5];
  std::strcpy(file_path, TCOIN_PATH_W_SLASH);
  std::strcat(file_path, file_name);
  std::strcat(file_path, ".txt");

  std::ifstream file(file_path);

  if(!file)
  {
    if(!strcmp(file_name, "base/base"))
    {
      std::cerr << "\nError! Could not open file at " << file_path << "!\n\n";
      exit_program(1);
    }
    else {
      std::cerr << "\nError! Could not open file at " << file_path << "! Assuming its internal content is \"0\\n\".\n\n";
      return (long long int)(0);
    }
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  delete[] file_path;
  return strtol_fast(ss.str().c_str());
}

int add_file_value(const char* file_name, const long long int &value_to_add, const long long int &base_amount)
{
  char* file_path = new char[strlen(file_name)+strlen(TCOIN_PATH_W_SLASH)+5];
  char* temp_file_path = new char[strlen(file_name)+strlen(TCOIN_PATH_W_SLASH)+9];
  std::strcpy(file_path, TCOIN_PATH_W_SLASH);
  std::strcat(file_path, file_name);
  std::strcpy(temp_file_path, file_path);
  std::strcat(file_path, ".txt");
  std::strcat(temp_file_path, "_tmp");
  std::strcat(temp_file_path, ".txt");

  std::ifstream file(file_path);

  if(!file)
  {
    if(!strcmp(file_name, "base/base"))
    {
      std::cerr << "\nError! Could not open file at " << file_path << "!\n\n";
      file.close();
      delete[] file_path;
      delete[] temp_file_path;
      exit_program(1);
    }
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  long long int old_value = strtol_fast(ss.str().c_str());

  //sufficient funds check
  if(value_to_add < 0 && (old_value + base_amount + value_to_add < 0))
  {
    file.close();
    delete[] file_path;
    delete[] temp_file_path;
    return 1;
  }

  long long int new_value = old_value + value_to_add;

  // Writing new value to file
  file.close();


  std::ofstream file2(temp_file_path);
  file2 << new_value << "\n";
  file2.close();

  chmod(temp_file_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

  if(!file2) //error
  {
    std::cerr << "Fatal error 999: the file \"" << file_name << "\" was unable to be updated. Please contact login@tilde.town (town-only) or login@tilde.team (internet-wide to report this error (because it requires manual recovery).";
    exit_program(999);
  }
  else
  {
    std::remove(file_path);
    while(1)
    {
      if(!std::rename(temp_file_path, file_path))
      {
        chmod(file_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
        break;
      }
    }
  }

  delete[] file_path;
  delete[] temp_file_path;
  return 0;
}

std::string global_username;
std::string get_username()
{
  return global_username;
}
int set_username(std::string &username)
{
  global_username.assign(username);
  return 0;
}

std::string formatted_amount(long long int const& amount, char const* appended_chars_default = "", char const* appended_chars_singular = "")
{
  std::ostringstream ss;

  bool is_non_negative = amount >= 0 ? true : false;
  if(!is_non_negative) //i.e., is negative
    ss << "-";
  if(((is_non_negative*2-1)*amount) % 100 == 0)
    ss << (is_non_negative*2-1)*amount/100;
  else if((is_non_negative*2-1)*amount % 100 < 10)
    ss << (is_non_negative*2-1)*amount/100 << ".0" << (is_non_negative*2-1)*amount % 100;
  else
    ss << (is_non_negative*2-1)*amount/100 << "." << (is_non_negative*2-1)*amount % 100;
  if(((is_non_negative*2-1)*amount == 100) && strcmp(appended_chars_singular, ""))
    ss << appended_chars_singular;
  else
    ss << appended_chars_default;

  std::string formatted_string(ss.str());
  return formatted_string;
}

void cout_formatted_amount(long long int const& amount, char const* appended_chars_default = "", char const* appended_chars_singular = "", bool negative_with_parentheses = false)
{
  bool amount_is_negative = (amount < 0);
  if(negative_with_parentheses && amount_is_negative) std::cout << "(";
  std::cout << formatted_amount(amount, appended_chars_default, appended_chars_singular);
  if(negative_with_parentheses && amount_is_negative) std::cout << ")";
}

long long int base_amount;
long long int user_amount;
long long int krowbar_amount; //krowbar's tilde game amount
long long int minercoin_amount; //minerobber's !minercoin game amount

void show_breakdown(const long long int &amount0 = 0, char const* amount0_source = "", const long long int &amount1 = 0, char const* amount1_source = "", const long long int &amount2 = 0, char const* amount2_source = "", const long long int &amount3 = 0, char const* amount3_source = "", const long long int &amount4 = 0, char const* amount4_source = "")
{
  bool a0 = (amount0 != 0 && strcmp(amount0_source, ""));
  bool a1 = (amount1 != 0 && strcmp(amount1_source, ""));
  bool a2 = (amount2 != 0 && strcmp(amount2_source, ""));
  bool a3 = (amount3 != 0 && strcmp(amount3_source, ""));
  bool a4 = (amount4 != 0 && strcmp(amount4_source, ""));
  if(a0 || a1 || a2 || a3 || a4)
  {
    if(a0)
    {
      std::cout << amount0_source << ",";
      cout_formatted_amount(amount0);
      if(a1 || a2 || a3 || a4)
      {
        std::cout << ";";
      }
    }
    if(a1)
    {
      std::cout << amount1_source << ",";
      cout_formatted_amount(amount1);
      if(a2 || a3 || a4)
      {
        std::cout << ";";
      }
    }
    if(a2)
    {
      std::cout << amount2_source << ",";
      cout_formatted_amount(amount2);
      if(a3 || a4)
      {
        std::cout << ";";
      }
    }
    if(a3)
    {
      std::cout << amount3_source << ",";
      cout_formatted_amount(amount3);
      if(a4)
      {
        std::cout << ";";
      }
    }
    if(a4)
    {
      std::cout << amount4_source << ",";
      cout_formatted_amount(amount4);
    }
    std::cout << "\n";
  }
}

void show_messages(const char* username)
{
  std::string messages_path = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages.txt");
  std::ifstream fin(messages_path.c_str());
  std::cout << fin.rdbuf();
  std::cout << "\n";
  fin.close();
}

void show_tsv_messages(const char* username) //tab-separated-value messages (tsv)
{
  std::string messages_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(username) + std::string("/_MESSAGES.txt");
  std::ifstream fin(messages_path.c_str());
  std::cout << fin.rdbuf();
  std::cout << "\n";
  fin.close();
}

void show_messages_tail(const char* username, int lineCount)
{
  size_t const granularity = 100 * lineCount;
  std::string messages_path = std::string(TCOIN_MSG_PATH) + std::string(username) + std::string("_messages.txt");
  std::ifstream source(messages_path.c_str(), std::ios_base::binary);
  source.seekg(0, std::ios_base::end);
  size_t size = static_cast<size_t>(source.tellg());
  std::vector<char> buffer;
  int newlineCount = 0; //pseudo newline count
  while(source && buffer.size() != size && newlineCount <= lineCount)
  {
    buffer.resize(std::min(buffer.size() + granularity, size));
    source.seekg(-static_cast<std::streamoff>(buffer.size()), std::ios_base::end);
    source.read(buffer.data(), buffer.size());
    newlineCount = std::count(buffer.begin(), buffer.end(), '\n');
    for(std::vector<char>::size_type i = 0; i < (buffer.size()-1); ++i)
      if(buffer[i] == '\n' && (buffer[i+1] == '\n' || buffer[i+1] == ' '))
      {
        newlineCount--; // An entry as follows: "<stuff>\n \_message>\n\n" must be treated as a single newline-ended line (and thus count 1 newline (not 3))
        ++i; //three consecutive newlines should not be "two pairs" of newlines
      }
  }
  std::vector<char>::iterator start = buffer.begin();
  while(newlineCount > lineCount)
  {
    start = std::find(start, buffer.end(), '\n') + 1;
    if(*start == ' ' || *start == '\n')
      continue; //we're counting cutting off a '\n ' (and '\n\n') as zero (and one) newline cut off because "<stuff>\n \_message>\n\n" is one message
    --newlineCount;
  }
  std::cout << "Last " << lineCount << " Messages:\n\n";
  std::vector<char>::iterator end = remove(start, buffer.end(), '\r');
  std::cout << std::string(start, end);
  if(*(end-2) != '\n') //if it ends with two newlines, don't put another one
    std::cout << "\n";
}

void show_tsv_messages_tail(const char* username, int lineCount)
{
  size_t const granularity = 100 * lineCount;
  std::string messages_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(username) + std::string("/_MESSAGES.txt");
  std::ifstream source(messages_path.c_str(), std::ios_base::binary);
  source.seekg(0, std::ios_base::end);
  size_t size = static_cast<size_t>(source.tellg());
  std::vector<char> buffer;
  int newlineCount = 0; //pseudo newline count
  while(source && buffer.size() != size && newlineCount <= lineCount)
  {
    buffer.resize(std::min(buffer.size() + granularity, size));
    source.seekg(-static_cast<std::streamoff>(buffer.size()), std::ios_base::end);
    source.read(buffer.data(), buffer.size());
    newlineCount = std::count(buffer.begin(), buffer.end(), '\n');
  }
  std::vector<char>::iterator start = buffer.begin();
  while(newlineCount > lineCount)
  {
    start = std::find(start, buffer.end(), '\n') + 1;
    --newlineCount;
  }
  std::vector<char>::iterator end = remove(start, buffer.end(), '\r');
  std::cout << std::string(start, end);
}

bool program_exists(const char* username)
{
  std::ifstream fin(PCOIN_KEY_PATH);
  //first word is program username, second word is key
  std::string word1;
  std::string word2;
  while(fin >> word1)
  {
    if(!word1.compare(username))
    {
      fin.close();
      return true;
    }
    fin >> word2; //to get rid of the second word (which is the key)
  }
  fin.close();
  return false; //if program not found
}

bool username_exists(const char* username)
{
  const static std::string all_usernames = exec(LS_HOME_CMD);
  std::istringstream iss(all_usernames);
  static std::vector<std::string> usernames{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
  if(std::find(usernames.begin(), usernames.end(), username) != usernames.end())
  {
    return true;
  }
  return program_exists(username);
}

bool file_is_empty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

bool files_are_same(const char* file_path1, const char* file_path2)
{
  std::ifstream fin1(file_path1);
  if(!fin1)
    return false;
  std::ifstream fin2(file_path2);
  if(!fin2)
    return false;

  char c1;
  char c2;
  while(fin1.get(c1) && fin2.get(c2)) //we need to go inside the loop body even if one of the reads fails, so we use || instead of &&
  {
    if(c2 != c1)
      return false;
  }
  fin2.get(c2); //when fin1.get(c1) fails, fin2.get(c2) is not executed because of short-circuited boolean operators. This line compensates for that.
  if(fin1 || fin2) //one of the files must still be valid while both files are not simulatenously valid (after the while loop), which means the files are of different sizes
    return false;

  return true;
  //Because of (!fin1 && fin2) || (fin1 && !fin2), if any one
  //file is larger than the other, the files are deemed not the same.
  //If the last characters of the two files have not been read, then
  //both "fin1" and "fin2" will return true in the next iteration, and
  //in the current iteration, c1 and c2 have valid values that can be compared.
  //If the last characters of the two files were read, then "fin1" and "fin2"
  //will still return true until the next time fin1.get() or fin2.get() is called.
  //c1 and c2 still carry valid values (namely, the last characters of fin1 and fin2)
  //which are compared. At the next iteration, both fin1 and fin2 fail and the loop exits.
  //This means all c1's and c2's were equal in the iterations before. Thus, the two files are
  //deemed the same.
}

int send_message(const char* sender_username, const char* receiver_username, const char* message, const long long int &amount_sent)
{
  std::string random_string = std::to_string(rand());

  char *receiver_path = new char[strlen(receiver_username) + 41];
  char *temp_receiver_path = new char[strlen(receiver_username) + strlen(random_string.c_str()) + 41];

  std::strcpy(receiver_path, TCOIN_MSG_PATH);
  std::strcat(receiver_path, receiver_username);
  std::strcat(receiver_path, "_messages.txt");

  std::strcpy(temp_receiver_path, TCOIN_MSG_PATH);
  std::strcat(temp_receiver_path, receiver_username);
  std::strcat(temp_receiver_path, random_string.c_str());
  std::strcat(temp_receiver_path, "_messages.txt");

  //create receiver's message file if none exists
  //the message will be included in the receiver's
  //account when she/he initialises her/his account
  //at a later time

  char *receiver_salt_path = new char[strlen(receiver_username) + 34];
  char *receiver_salt_logged_in_path = new char[strlen(receiver_username) + 44];
  std::strcpy(receiver_salt_path, TCOIN_SALT_PATH);
  std::strcat(receiver_salt_path, receiver_username);
  std::strcpy(receiver_salt_logged_in_path, receiver_salt_path);
  std::strcat(receiver_salt_path, "_salt.txt");
  std::strcat(receiver_salt_logged_in_path, "_salt_logged_in.txt");

  std::ifstream fin(receiver_path);
  std::ifstream fin2(receiver_salt_path);
  std::ifstream fin3(receiver_salt_logged_in_path);

  if((!fin || file_is_empty(fin)) && ((!program_exists(receiver_username)) || ((!fin2 || file_is_empty(fin2)) && (!fin3 || file_is_empty(fin3)))))
  {
    fin.close();
    std::ofstream fout(receiver_path, std::fstream::trunc);
    fout << "\n";
    fout.close();
  }
  else
    fin.close();
  fin2.close();
  fin3.close();

  chmod(receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

  delete[] receiver_salt_path;
  delete[] receiver_salt_logged_in_path;

  while(1)
  {
    if(!std::rename(receiver_path, temp_receiver_path))
    {
      char *really_temp_receiver_path = new char[strlen(temp_receiver_path) + 5];
      std::strcpy(really_temp_receiver_path, temp_receiver_path);
      std::strcat(really_temp_receiver_path, "_tmp");

      std::ifstream fin(temp_receiver_path);
      std::ofstream fout(really_temp_receiver_path);

      if(!file_is_empty(fin))
        fout << fin.rdbuf();
      fin.close();

      time_t now = time(0);
      char* dt = std::ctime(&now);
      dt[strlen(dt)-1]='\0';
      char sender_formatted_string[26];
      char sender_arrow_formatted_string[47];
      char sender_arrow_string[47];
      char receiver_formatted_string[26];
      std::snprintf(sender_formatted_string, 26, "%25s", sender_username);
      std::snprintf(receiver_formatted_string, 26, "%-25s", receiver_username);
      int sender_username_length = std::strlen(sender_username);
      int number_of_chars = 26 >= sender_username_length ? 26 : sender_username_length;
      std::strncpy(sender_arrow_string, sender_username, number_of_chars);
      sender_arrow_string[number_of_chars] = '\0';
      std::strcat(sender_arrow_string, " ----");
      std::string amount_sent_str = formatted_amount(amount_sent);
      std::strncat(sender_arrow_string, amount_sent_str.c_str(), 10);
      std::strcat(sender_arrow_string, "----> ");
      std::snprintf(sender_arrow_formatted_string, 47, "%46s", sender_arrow_string);
      fout << dt << ": " << sender_arrow_formatted_string << receiver_username;
      if(!strcmp(message, "")) //if message is empty
        fout << "\n";
      else
      {
        fout << "\n \\_ " << sender_username << " said: ";
        for(int i=0; message[i]!='\0'; ++i)
        {
          if(message[i] == '\n')
            fout << "\u23CE "; //return-key symbol, or return symbol, or enter symbol; space for spacing
          else
            fout << message[i];
        }
        fout << "\n\n";
      }
      fout.close();
      chmod(really_temp_receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

      if(!fout) //error
      {
        std::cerr << "Fatal error 101: the receiver message file was unable to be updated. Please contact login@tilde.town (town-only) or login@tilde.team (internet-wide to report this error (because it requires manual recovery).";
        exit_program(101);
      }
      else
      {
        std::remove(temp_receiver_path);
      }

      while(1)
      {
        if(!std::rename(really_temp_receiver_path, temp_receiver_path))
          break;
      }

      // unlock_receiver_messages
      while(1)
      {
        if(!std::rename(temp_receiver_path, receiver_path))
          break;
      }

      chmod(receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

      delete[] really_temp_receiver_path;
      delete[] temp_receiver_path;
      delete[] receiver_path;

      //additional place to write if sending to a program:
      if(program_exists(receiver_username))
      {
        random_string = std::string("rand");
        std::string program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_MESSAGES.txt");
        std::string temp_program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_MESSAGES") + random_string + std::string(".txt");
        std::string really_temp_program_receiver_path = temp_program_receiver_path + std::string("_tmp");

        //create program receiver's _MESSAGES file if none exists
        {
          std::ifstream fin(program_receiver_path.c_str());
          std::ifstream fin2(temp_program_receiver_path.c_str());
          if((!fin || file_is_empty(fin)) && (!fin2))
          {
            fin.close();
            std::ofstream fout(program_receiver_path.c_str(), std::fstream::trunc);
            fout << "\n";
            fout.close();
          }
          else
            fin.close();
          fin2.close();
          chmod(program_receiver_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
        }

        while(1)
        {
          if(!std::rename(program_receiver_path.c_str(), temp_program_receiver_path.c_str()))
          {
            std::ifstream fin(temp_program_receiver_path.c_str());
            std::ofstream fout(really_temp_program_receiver_path.c_str());

            if(!file_is_empty(fin))
              fout << fin.rdbuf();
            fin.close();

            //now, sender_username, receiver_username, amount_sent
            char sender_formatted_string[26];
            char receiver_formatted_string[26];
            std::snprintf(sender_formatted_string, 26, "%s", sender_username);
            std::snprintf(receiver_formatted_string, 26, "%s", receiver_username);
            std::string amount_sent_str = formatted_amount(amount_sent);
            fout << now << "\t" << sender_formatted_string << "\t" << receiver_formatted_string << "\t" << amount_sent_str;
            if(!strcmp(message, "")) //if message is empty
              fout << "\n";
            else
            {
              fout << "\t" << sender_formatted_string << "\t";
              for(int i=0; message[i]!='\0'; ++i)
              {
                if(message[i] == '\n')
                  fout << "\u23CE "; //return-key symbol, or return symbol, or enter symbol; space for spacing
                else
                  fout << message[i];
              }
              fout << "\n";
            }
            fout.close();
            chmod(really_temp_program_receiver_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

            if(!fout) //error
            {
              std::cerr << "Fatal error 103: the receiver program_message file was unable to be updated. Please contact login@tilde.town (town-only) or login@tilde.team (internet-wide to report this error (because it requires manual recovery).";
              exit_program(103);
            }
            else
            {
              std::remove(temp_program_receiver_path.c_str());
            }

            while(1)
            {
              if(!std::rename(really_temp_program_receiver_path.c_str(), temp_program_receiver_path.c_str()))
              break;
            }

            // unlock_receiver_program_messages
            while(1)
            {
              if(!std::rename(temp_program_receiver_path.c_str(), program_receiver_path.c_str()))
                break;
            }

            chmod(program_receiver_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
            break;
          }//if statement with !std::rename for receiver's program accounting _messages file
        }//while loop for program accounting receiver's _messages file
      }//receiver is program account

      //locking sender_messages_after_receiver_messages_unlocked

      random_string = std::to_string(rand());

      char *sender_path = new char[strlen(sender_username) + 41];
      char *temp_sender_path = new char[strlen(sender_username) + strlen(random_string.c_str()) + 41];

      std::strcpy(sender_path, TCOIN_MSG_PATH);
      std::strcat(sender_path, sender_username);
      std::strcat(sender_path, "_messages.txt");

      std:strcpy(temp_sender_path, TCOIN_MSG_PATH);
      std::strcat(temp_sender_path, sender_username);
      std::strcat(temp_sender_path, random_string.c_str());
      std::strcat(temp_sender_path, "_messages.txt");

      while(1)
      {
        if(!std::rename(sender_path, temp_sender_path))
        {
          char *really_temp_sender_path = new char[strlen(temp_sender_path) + 5];
          std::strcpy(really_temp_sender_path, temp_sender_path);
          std::strcat(really_temp_sender_path, "_tmp");

          fin.open(temp_sender_path);
          fout.open(really_temp_sender_path);
          chmod(really_temp_sender_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

          fout << fin.rdbuf();

          now = time(0);
          dt = std::ctime(&now);
          dt[strlen(dt)-1]='\0';
          char sender_formatted_string_right_aligned[26];
          char receiver_arrow_formatted_string[47];
          char receiver_arrow_string[47];
          std::snprintf(receiver_formatted_string, 26, "%25s", receiver_username);
          std::snprintf(sender_formatted_string, 26, "%-25s", sender_username);
          std::snprintf(sender_formatted_string_right_aligned, 26, "%25s", sender_username);
          int receiver_username_length = std::strlen(receiver_username);
          int number_of_chars = 26 >= receiver_username_length ? 26 : receiver_username_length;
          std::strncpy(receiver_arrow_string, receiver_username, number_of_chars);
          receiver_arrow_string[number_of_chars] = '\0';
          std::strcat(receiver_arrow_string, " <---");
          std::string amount_sent_str = formatted_amount(amount_sent);
          std::strncat(receiver_arrow_string, amount_sent_str.c_str(), 10);
          std::strcat(receiver_arrow_string, "----- ");
          std::snprintf(receiver_arrow_formatted_string, 47, "%46s", receiver_arrow_string);
          fout << dt << ": " << receiver_arrow_formatted_string << sender_username;
          if(!strcmp(message, "")) //if message is empty
            fout << "\n";
          else
          {
            fout << "\n \\_ " << sender_username << " said: ";
            for(int i=0; message[i]!='\0'; ++i)
            {
              if(message[i] == '\n')
                fout << "\u23CE "; //return-key symbol; space for spacing
              else
                fout << message[i];
            }
            fout << "\n\n";
          }

          fin.close();
          fout.close();

          if(!fout) //error
          {
            std::cerr << "Fatal error 102: the sender message file was unable to be updated. Please contact login@tilde.town (town-only) or login@tilde.team (internet-wide to report this error (because it requires manual recovery).";
            exit_program(102);
          }
          else
          {
            std::remove(temp_sender_path);
          }

          while(1)
          {
            if(!std::rename(really_temp_sender_path, temp_sender_path))
              break;
          }

          while(1)
          {
            if(!std::rename(temp_sender_path, sender_path))
              break;
          }

          delete[] really_temp_sender_path;
          delete[] temp_sender_path;
          delete[] sender_path;

          //additional place to write if sending from a program:
          if(program_exists(sender_username))
          {
            random_string = std::string("rand");
            std::string program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_MESSAGES.txt");
            std::string temp_program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_MESSAGES") + random_string + std::string(".txt");
            std::string really_temp_program_sender_path = temp_program_sender_path + std::string("_tmp");

            //create program sender's _MESSAGES file if none exists
            {
              std::ifstream fin(program_sender_path.c_str());
              std::ifstream fin2(temp_program_sender_path.c_str());
              if((!fin || file_is_empty(fin)) && (!fin2))
              {
                fin.close();
                std::ofstream fout(program_sender_path.c_str(), std::fstream::trunc);
                fout << "\n";
                fout.close();
              }
              else
                fin.close();
              fin2.close();
              chmod(program_sender_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
            }

            while(1)
            {
              if(!std::rename(program_sender_path.c_str(), temp_program_sender_path.c_str()))
              {
                std::ifstream fin(temp_program_sender_path.c_str());
                std::ofstream fout(really_temp_program_sender_path.c_str());

                if(!file_is_empty(fin))
                  fout << fin.rdbuf();
                fin.close();

                //now (updated by code above to reflect "sending time"), sender_username, receiver_username, amount_sent
                char sender_formatted_string[26];
                char receiver_formatted_string[26];
                std::snprintf(sender_formatted_string, 26, "%s", sender_username);
                std::snprintf(receiver_formatted_string, 26, "%s", receiver_username);
                std::string amount_sent_str = formatted_amount(-1*amount_sent);
                fout << now << "\t" << receiver_formatted_string << "\t" << sender_formatted_string << "\t" << amount_sent_str;
                if(!strcmp(message, "")) //if message is empty
                  fout << "\n";
                else
                {
                  fout << "\t" << sender_formatted_string << "\t";
                  for(int i=0; message[i]!='\0'; ++i)
                  {
                    if(message[i] == '\n')
                      fout << "\u23CE "; //return-key symbol, or return symbol, or enter symbol; space for spacing
                    else
                      fout << message[i];
                  }
                  fout << "\n";
                }
                fout.close();
                chmod(really_temp_program_sender_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

                if(!fout) //error
                {
                  std::cerr << "Fatal error 104: the sender program message file was unable to be updated. Please contact login@tilde.town (town-only) or login@tilde.team (internet-wide to report this error (because it requires manual recovery).";
                  exit_program(104);
                }
                else
                {
                  std::remove(temp_program_sender_path.c_str());
                }

                while(1)
                {
                  if(!std::rename(really_temp_program_sender_path.c_str(), temp_program_sender_path.c_str()))
                  break;
                }

                // unlock_sender_program_messages
                while(1)
                {
                  if(!std::rename(temp_program_sender_path.c_str(), program_sender_path.c_str()))
                    break;
                }

                chmod(program_sender_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
                break;
              }//if statement with !std::rename for sender's program accounting _messages file
            }//while loop for program accounting sender's _messages file
          }//sender is program account

          break;
        }
      }

      break;
    }
  }

  return 0;
}

bool user_is_locked(const char* username)
{
  std::ifstream fin((std::string(TCOIN_PATH_W_SLASH) + std::string(username) + std::string("_locked.txt")).c_str());
  if(!fin)
    return false;
  return true;
}

int send(const char* sender_username, const char* receiver_username, const long long int &amount_to_send, const long long int &base_amount, const char* option)
{
  int final_return_value = 0;

  //receiver usrname check
  if(username_exists(receiver_username))
  {
    if(!strcmp(sender_username, receiver_username))
    {
      std::cout << "\nSorry, you cannot send tildecoins to yourself.\n\n";
      return 5;
    }
    if(user_is_locked(receiver_username))
    {
      if(!strcmp(option, "verbose"))
        std::cout << "\nSorry, `" << receiver_username << "` does not wish to receive any tildecoins at this time.\n\n";
      return 4;
    }
    if(amount_to_send <= 0)
    {
      if(!strcmp(option, "verbose"))
        std::cout << "\nSorry, that amount is not valid. The amount should be a positive decimal number when truncated to two decimal places.\n\n";
      return 2;
    }
    else
    {
      std::string random_string = std::to_string(rand());
      int return_value = -1;

      //additional place to deduct from if the sender is a program (which is
      //always the case when `pcoin` is used, but we'll check anyway
      //we do this before the "tcoin send" part so that if program tries to send
      //more than it owes receiver, it's checked before checking "tcoin send's"
      //conditions
      if(program_exists(sender_username))
      {
        random_string = std::string("rand"); //not really random
        std::string program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/") + std::string(receiver_username) + std::string(".txt");
        std::string temp_program_sender_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/") + std::string(receiver_username) + random_string + std::string(".txt");
        std::string temp_program_sender_username = std::string(PROG_ACT_W_SLASH) + std::string(sender_username) + std::string("/") + std::string(receiver_username) + random_string;

        //create program sender's "receiver's balance file" if none exists
        {
          std::ifstream fin(program_sender_path.c_str());
          std::ifstream fin2(temp_program_sender_path.c_str());
          #if DEBUG
            std::cout << program_sender_path << "," << temp_program_sender_path << "," << !fin << "," << file_is_empty(fin) << "," << !fin2 << std::endl;
          #endif
          if((!fin || file_is_empty(fin)) && (!fin2))
          {
            fin.close();
            std::ofstream fout(program_sender_path.c_str(), std::fstream::trunc);
            fout << "0\n";
            fout.close();
            #if DEBUG
              char dummy;
              std::cout << "Press enter to continue"; std::cin >> dummy;
            #endif
          }
          else
            fin.close();
          fin2.close();
          chmod(program_sender_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
        }

        while(1)
        {
          if(!std::rename(program_sender_path.c_str(), temp_program_sender_path.c_str()))
          {
            //Insufficient funds check in add_file_value() itself
            //third argument, base_amount, is 0 because program
            //should not be allowed to send more than what is owed
            //to the receiver
            return_value = add_file_value(temp_program_sender_username.c_str(), -1 * amount_to_send, 0);

            //The same amount must also be deducted from the "_total.txt" file
            //which records the total amount owed to others
            if(return_value == 0)
            {
              random_string = std::string("rand");
              std::string program_sender_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_TOTAL.txt");
              std::string temp_program_sender_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(sender_username) + std::string("/_TOTAL") + random_string + std::string(".txt");
              std::string temp_program_sender_total_username = std::string(PROG_ACT_W_SLASH) + std::string(sender_username) + std::string("/_TOTAL") + random_string;

              //create program sender's "total balance file" if none exists
              {
                std::ifstream fin(program_sender_total_path.c_str());
                std::ifstream fin2(temp_program_sender_total_path.c_str());
                if((!fin || file_is_empty(fin)) && (!fin2))
                {
                  fin.close();
                  std::ofstream fout(program_sender_total_path.c_str(), std::fstream::trunc);
                  fout << "0\n";
                  fout.close();
                }
                else
                  fin.close();
                fin2.close();
                chmod(program_sender_total_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
              }

              while(1)
              {
                if(!std::rename(program_sender_total_path.c_str(), temp_program_sender_total_path.c_str()))
                {
                  //Insufficient funds check in add_file_value() itself
                  //third argument, base_amount, is 0 because program
                  //should not be allowed to send more than what is owed
                  //to the receiver
                  add_file_value(temp_program_sender_total_username.c_str(), -1 * amount_to_send, 0);

                  random_string = std::to_string(rand());

                  char* temp_sender_path = new char[strlen(sender_username) + strlen(random_string.c_str()) + 23];
                  char* sender_path = new char[strlen(sender_username) + 23];
                  char* temp_sender_username = new char[strlen(sender_username) + strlen(random_string.c_str()) + 1];

                  std::strcpy(temp_sender_username, sender_username);
                  std::strcat(temp_sender_username, random_string.c_str());

                  std::strcpy(temp_sender_path, TCOIN_PATH_W_SLASH);
                  std::strcat(temp_sender_path, temp_sender_username);
                  std::strcat(temp_sender_path, ".txt");

                  std::strcpy(sender_path, TCOIN_PATH_W_SLASH);
                  std::strcat(sender_path, sender_username);
                  std::strcat(sender_path, ".txt");

                  while(1)
                  {
                    if(!std::rename(sender_path, temp_sender_path))
                    {
                      //Insufficient funds check is in add_file_value()
                      return_value = add_file_value(temp_sender_username, -1 * amount_to_send, base_amount);

                      if(return_value == 0) // Funds sucessfully deducted from sender_username
                      {
                        random_string = std::string("rand");

                        char *temp_receiver_path = new char[strlen(receiver_username) + strlen(random_string.c_str()) + 23];
                        char *receiver_path = new char[strlen(receiver_username) + 23];
                        char *temp_receiver_username = new char[strlen(receiver_username) + strlen(random_string.c_str()) + 1];

                        std::strcpy(temp_receiver_username, receiver_username);
                        std::strcat(temp_receiver_username, random_string.c_str());

                        std::strcpy(temp_receiver_path, TCOIN_PATH_W_SLASH);
                        std::strcat(temp_receiver_path, temp_receiver_username);
                        std::strcat(temp_receiver_path, ".txt");

                        std::strcpy(receiver_path, TCOIN_PATH_W_SLASH);
                        std::strcat(receiver_path, receiver_username);
                        std::strcat(receiver_path, ".txt");

                        //create receiver's balance file if none exists
                        //the balance will be included in the receiver's
                        //account when she/he initialises her/his account
                        //at a later time
                        char *receiver_salt_path = new char[strlen(receiver_username) + 34];
                        char *receiver_salt_logged_in_path = new char[strlen(receiver_username) + 44];
                        std::strcpy(receiver_salt_path, TCOIN_SALT_PATH);
                        std::strcat(receiver_salt_path, receiver_username);
                        std::strcpy(receiver_salt_logged_in_path, receiver_salt_path);
                        std::strcat(receiver_salt_path, "_salt.txt");
                        std::strcat(receiver_salt_logged_in_path, "_salt_logged_in.txt");

                        std::ifstream fin(receiver_path);
                        std::ifstream fin2(receiver_salt_path);
                        std::ifstream fin3(receiver_salt_logged_in_path);

                        if((!fin || file_is_empty(fin)) && ((!program_exists(receiver_username)) || ((!fin2 || file_is_empty(fin2)) && (!fin3 || file_is_empty(fin3)))))
                        {
                          fin.close();
                          std::ofstream fout(receiver_path, std::fstream::trunc);
                          fout << "0\n";
                          fout.close();
                        }
                        else
                          fin.close();
                        fin2.close();
                        fin3.close();

                        chmod(receiver_path, (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);

                        delete[] receiver_salt_path;
                        delete[] receiver_salt_logged_in_path;

                        while(1)
                        {
                          if(!std::rename(receiver_path, temp_receiver_path))
                          {
                            //Insufficient funds check in add_file_value() itself
                            add_file_value(temp_receiver_username, amount_to_send, base_amount);

                            //additional place to write if sending to a program:
                            if(program_exists(receiver_username))
                            {
                              random_string = std::string("rand");
                              std::string program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/") + std::string(sender_username) + std::string(".txt");
                              std::string temp_program_receiver_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/") + std::string(sender_username) + random_string + std::string(".txt");
                              std::string temp_program_receiver_username = std::string(PROG_ACT_W_SLASH) + std::string(receiver_username) + std::string("/") + std::string(sender_username) + random_string;

                              //create program receiver's balance file if none exists
                              {
                                std::ifstream fin(program_receiver_path.c_str());
                                std::ifstream fin2(temp_program_receiver_path.c_str());
                                if((!fin || file_is_empty(fin)) && (!fin2))
                                {
                                  fin.close();
                                  std::ofstream fout(program_receiver_path.c_str(), std::fstream::trunc);
                                  fout << "0\n";
                                  fout.close();
                                }
                                else
                                  fin.close();
                                fin2.close();
                                chmod(program_receiver_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
                              }

                              while(1)
                              {
                                if(!std::rename(program_receiver_path.c_str(), temp_program_receiver_path.c_str()))
                                {
                                  //Insufficient funds check in add_file_value() itself
                                  add_file_value(temp_program_receiver_username.c_str(), amount_to_send, base_amount);

                                  //Value must also be added to a _total.txt file
                                  {
                                    random_string = std::string("rand");
                                    std::string program_receiver_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_TOTAL.txt");
                                    std::string temp_program_receiver_total_path = std::string(TCOIN_PROG_ACT_PATH) + std::string(receiver_username) + std::string("/_TOTAL")  + random_string + std::string(".txt");
                                    std::string temp_program_receiver_total_username = std::string(PROG_ACT_W_SLASH) + std::string(receiver_username) + std::string("/_TOTAL") + random_string;

                                    //create program receiver's "total balance file" if none exists
                                    {
                                      std::ifstream fin(program_receiver_total_path.c_str());
                                      std::ifstream fin2(temp_program_receiver_total_path.c_str());
                                      if((!fin || file_is_empty(fin)) && (!fin2))
                                      {
                                        fin.close();
                                        std::ofstream fout(program_receiver_total_path.c_str(), std::fstream::trunc);
                                        fout << "0\n";
                                        fout.close();
                                      }
                                      else
                                        fin.close();
                                      fin2.close();
                                      chmod(program_receiver_total_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
                                    }

                                    while(1)
                                    {
                                      if(!std::rename(program_receiver_total_path.c_str(), temp_program_receiver_total_path.c_str()))
                                      {
                                        //Insufficient funds check in add_file_value() itself
                                        add_file_value(temp_program_receiver_total_username.c_str(), amount_to_send, base_amount);
                                        while(1)
                                        {
                                          if(!std::rename(temp_program_receiver_total_path.c_str(), program_receiver_total_path.c_str()))
                                            break;
                                        }
                                        break;
                                      }
                                    }
                                  }

                                  while(1)
                                  {
                                    if(!std::rename(temp_program_receiver_path.c_str(), program_receiver_path.c_str()))
                                      break;
                                  }
                                  break;
                                }
                              }
                            }

                            if(!strcmp(option, "verbose"))
                            {
                              std::cout << "\n";
                              cout_formatted_amount(amount_to_send, " tildecoins were ", " tildecoin was ");
                              std::cout << "sent from `" << sender_username << "` to `" << receiver_username << "`.";
                              std::cout << "\n\n";
                            }

                            while(1)
                            {
                              if(!std::rename(temp_receiver_path, receiver_path))
                                break;
                            }
                            delete[] temp_receiver_path;
                            delete[] receiver_path;
                            delete[] temp_receiver_username;
                            break;
                          }
                        }
                      }
                      else if(return_value == 1)
                      {
                        if(!strcmp(option, "verbose"))
                        {
                          long long int amount_of_funds = base_amount + get_file_value(temp_sender_username);
                          std::cout << "\nSorry, you do not have sufficient funds to execute this transaction. ";
                          std::cout << "Your current balance is ";
                          cout_formatted_amount(amount_of_funds, " tildecoins.\n\n", " tildecoin.\n\n");
                        }
                        final_return_value = 3; //we don't simply "return 3" here because we want temp_sender_path to get renamed again
                      }

                      while(1)
                      {
                        if(!std::rename(temp_sender_path, sender_path))
                          break;
                      }

                      delete[] temp_sender_path;
                      delete[] sender_path;
                      delete[] temp_sender_username;
                      break;
                    }
                  }

                  while(1)
                  {
                    if(!std::rename(temp_program_sender_total_path.c_str(), program_sender_total_path.c_str()))
                      break;
                  }
                  break;
                }
              }
            }
            else if(return_value == 1)
            {
              long long int amount_owed = get_file_value(temp_program_sender_username.c_str());
              long long int amount_to_aib = (long long int)(amount_to_send) - amount_owed;
              std::cout << "\nSorry, you only owe `" << receiver_username << "` ";
              cout_formatted_amount(amount_owed, " tildecoins", " tildecoin");
              std::cout << ", not ";
              cout_formatted_amount(amount_to_send, " tildecoins. ", " tildecoin. ");
              std::cout << "Please run `" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance " << receiver_username << " ";
              cout_formatted_amount(amount_to_aib);
              std::cout << "` to sufficiently increase the amount owed to `" << receiver_username << "`.\n\n";

              final_return_value = 3; //we don't simply "return 3" here because we want temp_program_sender_path to get renamed again
            }

            while(1)
            {
              if(!std::rename(temp_program_sender_path.c_str(), program_sender_path.c_str()))
                break;
            }
            break;
          }
        }
      }
    }
  }
  else
  {
    if(!strcmp(option, "verbose"))
      std::cout << "\nSorry, no user with the username `" << receiver_username << "` was found.\n\n";
    return 1;
  }

  return final_return_value;
}

void help()
{
  std::cout <<"\npcoin is meant for programs. After each of the following commands, you will have to input a valid key to stdin.";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages` or `" << PCOIN_BIN_PATH_W_SPACE << "-m`: check your messages";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages <num>` or `" << PCOIN_BIN_PATH_W_SPACE << "-m <num>`: print the last <num> messages";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages_tsv` or `" << PCOIN_BIN_PATH_W_SPACE << "-mtsv`: check your messages in tab-separated-values format";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "messages_tsv <num>` or `" << PCOIN_BIN_PATH_W_SPACE << "-mtsv <num>`: print the last <num> messages in tab-separated-values format";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "balance` or `" << PCOIN_BIN_PATH_W_SPACE << "-b`: print the number representing your balance";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "total_owed` or `" << PCOIN_BIN_PATH_W_SPACE << "-to`: print the total amount owed to others";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "internal_balance <username>` or `" << PCOIN_BIN_PATH_W_SPACE << "-ib <username>`: print the amount you owe <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance <username>` or `" << PCOIN_BIN_PATH_W_SPACE << "-aib <username> <amount>`: add <amount> to the amount you owe <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount>` or `" << PCOIN_BIN_PATH_W_SPACE  << "-s <username> <amount>`: send <amount> tildecoins to <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount> [\"<message>\"]`: optionally, include a message to be sent to <username>";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "silentsend <username> <amount>`, `" << PCOIN_BIN_PATH_W_SPACE << "send -s <username> <amount>` or `" << PCOIN_BIN_PATH_W_SPACE << "-ss <username> <amount>`: send <amount> tildecoins to <username> without printing anything";
  std::cout << "\nIn the commands with `<username> <amount>`, switching the two arguments around (i.e., from `<username> <amount>` to `<amount> <username>`) will also work";
  std::cout << "\n`" << PCOIN_BIN_PATH_W_SPACE << "--help`, `" << PCOIN_BIN_PATH_W_SPACE << "help` or `" << PCOIN_BIN_PATH_W_SPACE << "-h`: print this help text";
  std::cout << "\nSend an email to `login@tilde.town` (tilde.town local email) or `login@tilde.team` (internet-wide email), or `/query login` on IRC to report any errors or request a key for your program.\n\n";
}

bool is_number(const char* test_string)
{
    char* p;
    strtod(test_string, &p);
    return *p == 0;
}

std::string get_username_from_key(std::string &key)
{
  std::ifstream fin(PCOIN_KEY_PATH);
  std::string word1, word2;
  bool should_return = false;
  //first word is program username, second word is key
  while(fin >> word1)
  {
    fin >> word2;
    if(!strctcmp(word2.c_str(), key.c_str()))
    {
      fin.close();
      return word1;
    }
  }

  word1.assign("n/a");
  fin.close();
  return word1;
}

long long int get_internal_balance(const char* username)
{
  //sometimes, it just helps to double-check things
  //this makes the security of this function decoupled
  //from the 'deny access' mechanism in the "main"
  //function when an incorrect key is entered
  if(program_exists(get_username().c_str()) && username_exists(username))
  {
    std::string internal_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/") + std::string(username) + std::string(".txt");
    //create internal balance file if none exists
    {
      std::ifstream fin(internal_path.c_str());
      if(!fin || file_is_empty(fin))
      {
        fin.close();
        std::ofstream fout(internal_path.c_str(), std::fstream::trunc);
        fout << "0\n";
        fout.close();
      }
      else
        fin.close();
      chmod(internal_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
    }

    std::string internal_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/") + std::string(username);
    return get_file_value(internal_username.c_str());
  }
  return -1;
}

long long int get_internal_total_owed()
{
  if(program_exists(get_username().c_str()))
  {
    std::string internal_total_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/_TOTAL.txt");
    //create internal total file if none exists
    {
      std::ifstream fin(internal_total_path.c_str());
      if(!fin || file_is_empty(fin))
      {
        fin.close();
        std::ofstream fout(internal_total_path.c_str(), std::fstream::trunc);
        fout << "0\n";
        fout.close();
      }
      else
        fin.close();
      chmod(internal_total_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
    }

    std::string internal_total_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/_TOTAL");
    return get_file_value(internal_total_username.c_str());
  }
  return -1;
}

int add_internal_balance(const char* username, const long long int value_to_add)
{
  if(program_exists(get_username().c_str()) && username_exists(username))
  {
    std::string random_string = std::string("rand");
    std::string internal_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/") + std::string(username);
    std::string temp_internal_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/") + std::string(username) + random_string;

    long long int internal_total_owed = get_internal_total_owed();
    if(internal_total_owed == -1)
    {
      std::cerr << "\nError in add_internal_balance()! get_internal_total_owed() failed!\n\n";
      return -3;
    }

    if((value_to_add > 0) && (value_to_add > (base_amount + user_amount - internal_total_owed)))
    {
      return -1; //value_to_add is more than what the program can fulfil using its own current funds
    }

    std::string internal_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/") + std::string(username) + std::string(".txt");
    std::string temp_internal_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/") + std::string(username) + random_string + std::string(".txt");
    //create internal file if none exists
    {
      std::ifstream fin(internal_path.c_str());
      std::ifstream fin2(temp_internal_path.c_str());
      if((!fin || file_is_empty(fin)) && (!fin2))
      {
        fin.close();
        std::ofstream fout(internal_path.c_str(), std::fstream::trunc);
        fout << "0\n";
        fout.close();
      }
      else
        fin.close();
      fin2.close();
      chmod(internal_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
    }

    int return_value;
    int final_return_value = 0;
    while(1)
    {
      if(!std::rename(internal_path.c_str(), temp_internal_path.c_str()))
      {
        return_value = add_file_value(temp_internal_username.c_str(), value_to_add, 0); //cannot make user's internal balance negative, so base_amount is 0

        if(return_value) //if return value is non-zero
        {
          final_return_value = return_value; //we don't simply "return return_value" because we want files to get renamed back to their original filenames
        }
        else //return value is zero, i.e., add_file_value succeeded
        //also need to update _total.txt
        {
          random_string = std::string("rand");
          std::string internal_total_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/_TOTAL");
          std::string temp_internal_total_username = std::string(PROG_ACT_W_SLASH) + get_username() + std::string("/_TOTAL") + random_string;
          std::string internal_total_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/_TOTAL.txt");
          std::string temp_internal_total_path = std::string(TCOIN_PROG_ACT_PATH) + get_username() + std::string("/_TOTAL") + random_string + std::string(".txt");

          //create _total.txt file if none exists
          {
            std::ifstream fin(internal_total_path.c_str());
            std::ifstream fin2(temp_internal_total_path.c_str());
            if((!fin || file_is_empty(fin)) && (!fin2))
            {
              fin.close();
              std::ofstream fout(internal_total_path.c_str(), std::fstream::trunc);
              fout << "0\n";
              fout.close();
            }
            else
              fin.close();
            fin2.close();
            chmod(internal_total_path.c_str(), (S_IRUSR | S_IWUSR) & ~S_IRWXG & ~S_IRWXO);
          }

          while(1)
          {
            if(!std::rename(internal_total_path.c_str(), temp_internal_total_path.c_str()))
            {
              return_value = add_file_value(temp_internal_total_username.c_str(), value_to_add, 0);
              while(1)
              {
                if(!std::rename(temp_internal_total_path.c_str(), internal_total_path.c_str()))
                  break;
              }
              break;
            }
          }
          if(return_value) //if return value is non-zero
            final_return_value = return_value; //we don't simply "return return_value" because we want files to get renamed back to their original filenames
        }

        while(1)
        {
          if(!std::rename(temp_internal_path.c_str(), internal_path.c_str()))
            break;
        }
        break;
      }
    }
    return final_return_value;
  }
  return -2;
}

int main(int argc, char *argv[])
{
  if(argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "help") || !strcmp(argv[1], "-h")))
  {
    help();
    return 0;
  }

  {
    std::string key;
    std::string program_username;
    std::getline(std::cin, key);

    program_username.assign(get_username_from_key(key));

    if(!program_username.compare("n/a"))
    {
      std::cout << "\nSorry, the key you specified is not in use.\n\n";
      return 9;
    }
    set_username(program_username);
  }

  base_amount = 0;
  long long int unaltered_base_amount = base_amount;
  user_amount = 0;
  krowbar_amount = 0;
  minercoin_amount = 0;

  //adding tildebot scores from krowbar to base amount
  {
    std::string line;
    const std::string username = get_username();
    const int username_length = username.length();
    std::ifstream fin(KROWBAR_SCORE_PATH);
    while(std::getline(fin, line))
    {
      char* line_c_string = new char[line.length()+1];
      std::strcpy(line_c_string, line.c_str());

      const int irc_username_length = username_length > USERNAME_LENGTH_LIMIT ? USERNAME_LENGTH_LIMIT : username_length;

      if(!strncasecmp(username.c_str(), line_c_string, irc_username_length)) //username starts with capital letter, but name in database does not
      {
        char number_of_tildes[21];
        number_of_tildes[0] = '0'; //just in case the loop below doesn't detect any digits
        number_of_tildes[1] = '\0';

        for(int i=0; i < 20; ++i)
        {
          if(std::isdigit(line_c_string[irc_username_length+3+i]))
            number_of_tildes[i] = line_c_string[irc_username_length+3+i];
          else
          {
            number_of_tildes[i] = '\0'; //manually terminating the string
            break;
          }
        }
        number_of_tildes[20] = '\0'; //incase the number overflows 20 characters
        krowbar_amount += strtol100(number_of_tildes);
        base_amount += krowbar_amount;
        //multiplied by 100 inside strtol100() to convert tildecoins to centitildecoins, which
        //is the unit used throughout the program (and converted appropriately when displayed)
      }
      delete[] line_c_string;
    }
  }

  //adding minercoin scores from minerobber to base amount
  {
    std::string command_to_exec = std::string(MINERCOIN_CMD_PRE_USERNAME) + get_username() + std::string(MINERCOIN_CMD_POST_USERNAME);
    std::string number_of_tildes = exec(command_to_exec.c_str());
    number_of_tildes.pop_back();
    //to get rid of the newline at the end
    if(is_number(number_of_tildes.c_str()))
      minercoin_amount += strtol100(number_of_tildes.c_str());
      base_amount += minercoin_amount;
      //multiplied by 100 to convert tildecoins to centitildecoins, which
      //is the unit used throughout the program (and converted appropriately when displayed)
  }

  user_amount = get_file_value(get_username().c_str());

  srand((long int)(std::time(NULL)) + strtol_fast(exec(BIN_ECHO_CMD).c_str()));

  long long int user_amount = get_file_value(get_username().c_str());

  long long int total_amount = base_amount + user_amount;

  if(argc < 2)
  {
    std::cout << "\nSorry, `" << PCOIN_BIN_PATH << "` doesn't work. Please use `" << PCOIN_BIN_PATH_W_SPACE << "-m` for messages or `" << PCOIN_BIN_PATH_W_SPACE << "-b` to check your balance. `" << PCOIN_BIN_PATH_W_SPACE << "--help` prints the help text.\n\n";
    return 8;
  }
  else if(!strcmp(argv[1], "breakdown") || !strcmp(argv[1], "-bd"))
  {
    std::cout << "total,";
    cout_formatted_amount(total_amount, ";", ";");
    show_breakdown(unaltered_base_amount, "baseamount", user_amount, "transfers", krowbar_amount, "tildegame", minercoin_amount, "minercoin");
  }
  else if(!strcmp(argv[1], "messages") || !strcmp(argv[1], "-m"))
  {
    double number_of_messages = 0.0;
    bool number_of_messages_is_specified = argc > 2 && is_number(argv[2]); //number of messages specified
    if(number_of_messages_is_specified)
      number_of_messages = std::strtod(argv[2], NULL);
    if(number_of_messages >= 1.0) //number of messages specified is a valid number
      show_messages_tail(get_username().c_str(), (long long int)(number_of_messages));
    else //show all messages
      show_messages(get_username().c_str());
  }
  else if(!strcmp(argv[1], "messages_tsv") || !strcmp(argv[1], "-mtsv"))
  {
    double number_of_messages = 0.0;
    bool number_of_messages_is_specified = argc > 2 && is_number(argv[2]); //number of messages specified
    if(number_of_messages_is_specified)
      number_of_messages = std::strtod(argv[2], NULL);
    if(number_of_messages >= 1.0) //number of messages specified is a valid number
      show_tsv_messages_tail(get_username().c_str(), (long long int)(number_of_messages));
    else //show all messages
      show_tsv_messages(get_username().c_str());
  }
  else if(!strcmp(argv[1], "balance") || !strcmp(argv[1], "-b"))
    cout_formatted_amount(total_amount, "\n");
  else if(!strcmp(argv[1], "total_owed") || !strcmp(argv[1], "-to"))
  {
    long long int total_owed = get_internal_total_owed();
    if(total_owed == -1)
    {
      std::cerr << "\nError in main()! get_internal_total_owed() failed!\n\n";
      return 18;
    }
    cout_formatted_amount(total_owed, "\n");
  }
  else if(!strcmp(argv[1], "internal_balance") || !strcmp(argv[1], "-ib"))
  {
    if(argc == 3) //second argument (the one right after "-ib") is the username
    {
      long long int internal_balance = get_internal_balance(argv[2]);
      if(internal_balance == -1) //username check doesn't pass
      {
        std::cout << "\nSorry, no user with the username `" << argv[2] << "` was found.\n\n";
        return 17;
      }
      cout_formatted_amount(internal_balance, "\n");
    }
    else if(argc == 2) //no username supplied (too few arguments supplied)
    {
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "internal_balance <username>`.\n\n";
      return 10;
    }
    else if(argc > 3) //too many arguments supplied
    {
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "internal_balance <username>`.\n\n";
      return 11;
    }
  }
  else if(!strcmp(argv[1], "add_internal_balance") || !strcmp(argv[1], "-aib"))
  {
    if(argc < 4)
    {
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance <username> <amount>`.\n\n";
      return 12;
    }
    else if(argc > 4)
    {
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "add_internal_balance <username> <amount>`.\n\n";
      return 13;
    }
    // number of arguments is exactly 3
    {
      int return_value = -3, return_value2 = -3;
      if(is_number(argv[3]))
        return_value = add_internal_balance(argv[2], strtol100(argv[3]));
      else
        return_value2 = add_internal_balance(argv[3], strtol100(argv[2]));

      if(return_value == -1 || return_value2 == -1) //value_to_add was too large
      {
        std::cout << "\nSorry, the amount was larger than what the program's current unowed balance could cover.\n\n";
        return 14;
      }
      if(return_value == 1) //value_to_add was too negative
      {
        std::cout << "\nSorry, the amount was more negative than what `" << argv[2] << "` could cover.\n\n";
        return 16;
      }
      if(return_value2 == 1) //value_to_add was too negative
      {
        std::cout << "\nSorry, the amount was more negative than what `" << argv[3] << "` could cover.\n\n";
        return 16;
      }
      if(return_value == -2) //username check doesn't pass
      {
        std::cout << "\nSorry, no user with the username `" << argv[2] << "` was found.\n\n";
        return 15;
      }
      if(return_value2 == -2) //username check doesn't pass
      {
        std::cout << "\nSorry, no user with the username `" << argv[3] << "` was found.\n\n";
        return 15;
      }
    }
  }
  else if(!strcmp(argv[1], "send") || !strcmp(argv[1], "-s"))
  {
    if(argc == 5)
    {
      if(!strcmp(argv[2], "-s"))
        if(is_number(argv[3]))
          send(get_username().c_str(), argv[4], strtol100(argv[3]), base_amount, "silent");
        else
          send(get_username().c_str(), argv[3], strtol100(argv[4]), base_amount, "silent");
      else //argument count is 5 because a custom message was included
      {
        int return_value;
        if(is_number(argv[3]))
          return_value = send(get_username().c_str(), argv[2], strtol100(argv[3]), base_amount, "verbose");
        else
          return_value = send(get_username().c_str(), argv[3], strtol100(argv[2]), base_amount, "verbose");

        if(!return_value) //send was successful
          if(is_number(argv[3]))
            send_message(get_username().c_str(), argv[2], argv[4], strtol100(argv[3]));
          else
            send_message(get_username().c_str(), argv[3], argv[4], strtol100(argv[2]));
      }
    }
    else if(argc < 4)
    {
      std::cout << "\nSorry, too few command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount>`.\n\n";
      return 6;
    }
    else if(argc > 4)
    {
      std::cout << "\nSorry, too many command-line arguments were passed. The correct format is `" << PCOIN_BIN_PATH_W_SPACE << "send <username> <amount>`.\n\n";
      return 7;
    }
    else
    {
      int return_value;
      if(is_number(argv[2]))
        return_value = send(get_username().c_str(), argv[3], strtol100(argv[2]), base_amount, "verbose");
      else
        return_value = send(get_username().c_str(), argv[2], strtol100(argv[3]), base_amount, "verbose");
      if(!return_value) //send was successful
        if(is_number(argv[2]))
          send_message(get_username().c_str(), argv[3], "", strtol100(argv[2]));
        else
          send_message(get_username().c_str(), argv[2], "", strtol100(argv[3]));
    }
  }
  else if(!strcmp(argv[1], "silentsend") || !strcmp(argv[1], "-ss"))
  {
    if(argc==4)
    {
      int return_value;
      if(is_number(argv[3]))
        return_value = send(get_username().c_str(), argv[2], strtol100(argv[3]), base_amount, "silent");
      else
        return_value = send(get_username().c_str(), argv[3], strtol100(argv[2]), base_amount, "silent");
      if(!return_value) //send was successful
        if(is_number(argv[3]))
          send_message(get_username().c_str(), argv[2], "", strtol100(argv[3]));
        else
          send_message(get_username().c_str(), argv[3], "", strtol100(argv[2]));
    }
    else
      return 2;
  }
  else
  {
    std::cout << "\nSorry, an unknown command-line argument was received. `" << PCOIN_BIN_PATH_W_SPACE << "help` will print the help text.\n\n";
    return 3;
  }

  return 0;
}
