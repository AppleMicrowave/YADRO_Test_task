#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#include "classes.h"
using namespace std::chrono;

// Read event
// TODO add last checks and move checks to check_event func
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
  type = std::stoi(id);
  if (type < 1 || type > 13) throw std::exception();

  // TODO: check if table is >< than setted;
  ss >> table;
}

void Table::calculate(system_clock::time_point endTime) {
  if (!currentClient.empty()) {
    auto duration = duration_cast<minutes>(endTime - startTime).count();
    totalUsage += duration;
    unsigned hours = std::ceil(static_cast<double>(duration) / 60.0);
    sum += hours * pricePerHour;
    currentClient.clear();
  }
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
  std::cout << std::put_time(std::localtime(&temp), "%H:%M") << " " << ERR_O
            << " " << error << std::endl;
}

unsigned Club::find_free_table() {
  for (auto& [number, table] : tables) {
    if (table.currentClient.empty()) return number;
  }
  return 0;
}

void Club::close() {
  std::vector<std::string> clientNames;
  for (const auto& [name, _] : clients) {
    clientNames.push_back(name);
  }
  std::sort(clientNames.begin(), clientNames.end());

  for (const auto& name : clientNames) {
    if (!clients.contains(name)) continue;

    Client client = clients[name];
    if (client.table != 0) {
      tables[client.table].calculate(closeTime);
    }

    Event leaveEvent{closeTime, LEAVE_O, client, 0};
    handle_event(leaveEvent);
  }

  auto temp = system_clock::to_time_t(closeTime);
  std::cout << std::put_time(std::localtime(&temp), "%H:%M") << std::endl;

  for (unsigned i = 1; i <= tableAmount; ++i) {
    unsigned minutes = tables[i].totalUsage;
    unsigned hours = minutes / 60;
    unsigned remainingMinutes = minutes % 60;
    std::cout << i << " " << tables[i].sum << " " << std::setfill('0')
              << std::setw(2) << hours << ":" << std::setfill('0')
              << std::setw(2) << remainingMinutes << std::endl;
  }
}

// For any InEvent
void Club::handle_event(Event& event) {
  // ugly crutch
  if (event.table > tableAmount) throw std::exception();

  std::string& aName = event.client.name;
  switch (event.type) {
    case ARRIVED_I: {  // 1
      if (clients.contains(aName)) {
        event.error = "YouShallNotPass";
      } else if (event.eventTime < openTime || event.eventTime > closeTime) {
        event.error = "NotOpenYet";
      } else {
        clients[aName] = event.client;
      }

      break;
    }
    case SIT_I: {  // 2
      if (!clients.contains(aName)) {
        event.error = "ClientUnknown";
      } else if (!tables[event.table].currentClient.empty() ||
                 (event.table == clients[aName].table)) {
        event.error = "PlaceIsBusy";
      } else {
        // remind to fix here
        if (clients[aName].table != 0) {
          tables[clients[aName].table].calculate(event.eventTime);
        }
        clients[aName].table = event.table;
        tables[event.table].begin_usage(aName, event.eventTime);
      }
      break;
    }
    case WAITING_I: {  // 3
      unsigned free_table = find_free_table();
      if (clientQueue.size() + 1 > tableAmount) {
        Event outEvent{event.eventTime, LEAVE_O, event.client, 0};
        event.print_event();
        handle_event(outEvent);
        return;
      } else if (free_table) {
        event.error = "ICanWaitNoLonger!";
      } else {
        clientQueue.push(event.client);
      }
      break;
    }
    case LEAVE_I: {  // 4
      if (!clients.contains(aName)) {
        event.error = "ClientUnknown";
      } else {
        unsigned freed_table = clients[aName].table;
        tables[freed_table].calculate(
            event.eventTime);  // freeing the table and summing the value

        clients.erase(aName);  // client is gone
        event.print_event();

        if (!clientQueue.empty()) {
          Client nextclient = clientQueue.front();
          clientQueue.pop();

          Event outEvent{event.eventTime, SIT_O, nextclient, freed_table};
          handle_event(outEvent);
        }
        return;
      }
      break;
    }
    case LEAVE_O: {  // 11
      if (clients.contains(aName) && clients[aName].table != 0) {
        tables[clients[aName].table].calculate(event.eventTime);
      }
      clients.erase(aName);
      break;
    }
    case SIT_O: {  // 12
      clients[aName].table = event.table;
      tables[event.table].begin_usage(aName, event.eventTime);
      event.print_event();
      return;
    }
    default:
      break;
  }
  event.print_event();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Error! Please specify filename: ./exe 'file'\n";
    return 1;
  }

  std::ifstream f_input{argv[1]};
  if (f_input.fail()) {
    std::cerr << "Cannot open file: " << argv[1] << "\n";
    return 2;
  }

  std::string read_buffer;
  unsigned table_amount;
  unsigned price;
  std::tm open_tm = {}, close_tm = {};

  // ---------- Read table amount ----------
  if (!std::getline(f_input, read_buffer)) {
    std::cout << read_buffer << std::endl;
    return 3;
  }
  try {
    table_amount = stoul(read_buffer);
    if (table_amount == 0) throw std::exception();
  } catch (...) {
    std::cout << read_buffer << std::endl;
    return 3;
  }

  // ---------- Read working hours ----------
  if (!std::getline(f_input, read_buffer)) {
    std::cout << "Missing working hours line\n";
    return 3;
  }
  std::stringstream ss_time(read_buffer);
  ss_time >> std::get_time(&open_tm, "%H:%M");
  ss_time >> std::get_time(&close_tm, "%H:%M");
  if (ss_time.fail()) {
    std::cout << read_buffer << std::endl;
    return 3;
  }

  auto openT = system_clock::from_time_t(mktime(&open_tm));
  auto closeT = system_clock::from_time_t(mktime(&close_tm));

  if (!std::getline(f_input, read_buffer)) {
    std::cout << "Missing price line\n";
    return 3;
  }
  try {
    price = stoul(read_buffer);
    if (price == 0) throw std::exception();
  } catch (...) {
    std::cout << read_buffer << std::endl;
    return 3;
  }

  Club clubOperator{openT, closeT, price, table_amount};

  while (std::getline(f_input, read_buffer)) {
    if (read_buffer.empty()) continue;
    try {
      Event event(read_buffer);
      clubOperator.handle_event(event);
    } catch (const std::exception& e) {
      std::cout << read_buffer << std::endl;
      return 3;
    }
  }

  clubOperator.close();

  return 0;
}
