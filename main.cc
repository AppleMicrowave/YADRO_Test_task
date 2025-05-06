#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#include "classes.h"
using namespace std::chrono;

// ШАБЛОН:

//                       КОЛ-ВО СТОЛОВ

//              ВРЕМЯ НАЧАЛА   |   ВРЕМЯ ОКОНЧАНИЯ

//                      СТОИМОСТЬ ЧАСА

// ВРЕМЯ СОБЫТИЯ N | ИДЕНТИФИКАТОР СО БЫТИЯ N | ТЕЛО СОБЫТИЯ N

// Read event
// TODO add last checks
Event::Event(const std::string& line) {
  std::stringstream ss(line);
  std::string timeStr, id, tableNumber;
  const std::regex timeFilter{R"(^(?:[01]\d|2[0-3]):[0-5]\d$)"};
  const std::regex nameFilter{R"(^[a-z0-9_-]+$)"};

  ss >> timeStr;
  if (!std::regex_match(timeStr, timeFilter)) {
    throw std::exception();
  }

  ss.clear();
  ss.str(line);

  std::tm temp{};
  ss >> std::get_time(&temp, "%H:%M");
  eventTime = system_clock::from_time_t(std::mktime(&temp));

  ss >> id >> client.name;
  if (!std::regex_match(client.name, nameFilter)) {
    throw std::exception();
  }
  type = static_cast<InEvent>(std::stoi(id));

  ss >> table;
}

void Event::print_event() {
  auto temp = system_clock::to_time_t(eventTime);
  std::cout << std::put_time(std::localtime(&temp), "%H:%M") << " " << type
            << " " << client.name;
  if (table != 0)
    std::cout << " " << table << std::endl;
  else
    std::cout << std::endl;
}

// For any InEvent
void Club::handle_event(Event& event) {
  switch (event.type) {
    case 1: {
      if (clients.contains(event.client))
    }
    case 2: {
    }
    case 3: {
    }
    case 4: {
    }
    default:
      break;
  }
}

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
