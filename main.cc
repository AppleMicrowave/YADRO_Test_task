#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <regex>
#include <sstream>

// ШАБЛОН:

//                       КОЛ-ВО СТОЛОВ

//              ВРЕМЯ НАЧАЛА   |   ВРЕМЯ ОКОНЧАНИЯ

//                      СТОИМОСТЬ ЧАСА

// ВРЕМЯ СОБЫТИЯ N | ИДЕНТИФИКАТОР СО БЫТИЯ N | ТЕЛО СОБЫТИЯ N

struct Client {};

enum InEvent { ARRIVED_I = 1, SIT_I, WAITING_I, LEAVE_I };
enum OutEvent { LEAVE_O = 11, SIT_O, ERR_O };

class Club {
 private:
  size_t tables;
  std::queue<Client> queue;
};

struct Event {
  std::tm eventTime;
  InEvent type;
  std::string client;
  unsigned table = 0;

  Event(const std::string& line);
  void print_event();
};

// Read event
Event::Event(const std::string& line) {
  std::stringstream ss(line);
  std::string timeStr, id, tableNumber;
  const std::regex timeFilter{R"(^(?:[01]\d|2[0-3]):[0-5]\d$)"};

  ss >> timeStr;
  if (!std::regex_match(timeStr, timeFilter)) {
    throw std::exception();
  }

  ss.clear();
  ss.str(line);

  ss >> std::get_time(&eventTime, "%H:%M");

  ss >> id >> client;
  type = static_cast<InEvent>(std::stoi(id));

  ss >> table;
}

void Event::print_event() {
  std::cout << std::put_time(&eventTime, "%H:%M") << " " << type << " "
            << client;
  if (table != 0)
    std::cout << " " << table << std::endl;
  else
    std::cout << std::endl;
}

// For any InEvent
void handle_event() {}

// OutEvent: ERR_O
void print_error() {}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Error! Please specify filename: ./exe 'file' \n";
    return 1;
  }

  std::ifstream f_input;

  f_input.open(argv[1]);
  if (f_input.fail()) {
    std::cerr << "Cannot open file: \n" << argv[1];
    return 2;
  }

  std::string read_buffer;

  unsigned table_amount;
  unsigned price;

  // read table amount
  std::getline(f_input, read_buffer);

  table_amount = std::stoul(read_buffer);
  std::cout << table_amount << std::endl;

  // read hours
  std::getline(f_input, read_buffer);

  std::stringstream ss_time_buffer(read_buffer);
  std::tm t_begin = {};
  std::tm t_end = {};

  ss_time_buffer >> std::get_time(&t_begin, "%H:%M");
  // note: get_time moves buf to end of format
  ss_time_buffer >> std::get_time(&t_end, "%H:%M");

  std::cout << std::put_time(&t_begin, "%H:%M") << " "
            << std::put_time(&t_end, "%H:%M") << std::endl;

  // read price per hour
  std::getline(f_input, read_buffer);
  price = std::stoul(read_buffer);

  std::cout << price << std::endl;

  while (std::getline(f_input, read_buffer)) {
    try {
      Event event(read_buffer);
      // std::cout << "Line: " << read_buffer << std::endl;
      event.print_event();
    } catch (const std::exception& e) {
      std::cout << read_buffer << std::endl;
      return 3;
    }
  }

  return 0;
}