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
// TODO add last checks and move checks to separate func
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
  type = static_cast<InEventEnum>(std::stoi(id));

  // TODO: check if table is >< than setted;
  ss >> table;
}

void Event::print_event() {
  auto temp = system_clock::to_time_t(eventTime);
  std::cout << std::put_time(std::localtime(&temp), "%H:%M");
  std::cout << " " << type << " " << client.name
            << (table == 0 ? "" : " " + std::to_string(table)) << std::endl;

  // If error field is not empty, need to print
  if (!error.empty()) print_error();
}

void Event::print_error() {
  auto temp = system_clock::to_time_t(eventTime);
  std::cout << std::put_time(std::localtime(&temp), "%H:%M") << " " << "13"
            << " " << error << std::endl;
}

// For any InEvent
void Club::handle_event(Event& event) {
  std::string& aName = event.client.name;
  switch (event.type) {
    case ARRIVED_I: {
      if (clients.contains(aName)) {
        event.error = "YouShallNotPass";
      } else if (event.eventTime < openTime || event.eventTime > closeTime) {
        event.error = "NotOpenYet";
      } else {
        clients[aName] = event.client;
        event.client.isIn = true;
      }

      break;
    }
    case SIT_I: {
      if (!clients.contains(aName)) {
        event.error = "ClientUnknown";
      } else if (!tables[event.table].empty() ||
                 (event.table == clients[aName].table)) {
        event.error = "PlaceIsBusy";
      } else {
        clients[aName].table = event.table;
        clients[aName].isPlaying = true;
        tables[event.table] = aName;
      }
      break;
    }
    case 3: {
    }
    case 4: {
    }
    default:
      break;
  }
  // if (event.error.empty()) handle_event(event.type);
  event.print_event();
}

// void Club::handle_event(unsigned type) {
//   switch (type) {
//     case 1: {
//       clients[e]
//     }
//   }
// }

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Error! Please specify filename: ./exe 'file' \n";
    return 1;
  }

  std::ifstream f_input{argv[1]};
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

  // read hours
  std::getline(f_input, read_buffer);

  std::stringstream ss_time_buffer(read_buffer);
  std::tm open_tm = {};
  std::tm close_tm = {};

  ss_time_buffer >> std::get_time(&open_tm, "%H:%M");
  ss_time_buffer >> std::get_time(&close_tm, "%H:%M");

  system_clock::time_point openT =
      system_clock::from_time_t(std::mktime(&open_tm));
  system_clock::time_point closeT =
      system_clock::from_time_t(std::mktime(&close_tm));

  // read price per hour
  std::getline(f_input, read_buffer);
  price = std::stoul(read_buffer);

  Club clubOperator{openT, closeT, price, table_amount};

  while (std::getline(f_input, read_buffer)) {
    if (read_buffer.empty()) continue;
    try {
      Event event(read_buffer);
      // std::cout << "Line: " << read_buffer << std::endl;
      clubOperator.handle_event(event);
    } catch (const std::exception& e) {
      std::cout << read_buffer << std::endl;
      return 3;
    }
  }

  return 0;
}
