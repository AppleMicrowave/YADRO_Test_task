#include <chrono>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
using namespace std::chrono;

enum InEventEnum { ARRIVED_I = 1, SIT_I, WAITING_I, LEAVE_I };
enum OutEventEnum { LEAVE_O = 11, SIT_O, ERR_O };

struct Table {
  unsigned number;
  std::string currentClient;
  system_clock::time_point startTime;
  unsigned sum = 0;
  unsigned totalUsage;
  unsigned pricePerHour;

  Table() = default;
  explicit Table(unsigned price_) : pricePerHour(price_), totalUsage(0) {}

  void begin_usage(const std::string& client, system_clock::time_point time) {
    currentClient = client;
    startTime = time;
  }

  void calculate(system_clock::time_point endTime);
};

struct Client {
  std::string name;
  unsigned table = 0;

  // bool operator<(const Client& target) const { return name < target.name; }
};

struct Event {
  system_clock::time_point eventTime;  // <время>
  unsigned type = 0;                   // <id>
  Client client;                       // <клиент>
  unsigned table = 0;                  // <стол*>

  std::string error;
  // std::string outEvent;

  explicit Event(const std::string& line);
  explicit Event(system_clock::time_point time_, unsigned type_,
                 const Client& client_, unsigned table_)
      : eventTime(time_), type(type_), client(client_), table(table_) {}

  void check_event();

  void print_event();
  void print_error();
};

class Club {
 private:
  system_clock::time_point openTime;
  system_clock::time_point closeTime;
  unsigned pricePerHour;
  unsigned tableAmount;

  std::queue<Client> clientQueue;
  std::map<std::string, Client> clients;
  std::map<unsigned, Table> tables;

 public:
  explicit Club(system_clock::time_point open, system_clock::time_point close,
                unsigned price, unsigned amount)
      : openTime(open),
        closeTime(close),
        pricePerHour(price),
        tableAmount(amount) {
    for (unsigned i = 1; i <= tableAmount; ++i) {
      tables.emplace(i, Table(price));
    }
  }

  // void check_event();
  void handle_event(Event& event);
  void close();

  unsigned find_free_table();
};
