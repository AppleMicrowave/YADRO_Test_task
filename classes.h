#include <chrono>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>

enum InEvent { ARRIVED_I = 1, SIT_I, WAITING_I, LEAVE_I };
enum OutEvent { LEAVE_O = 11, SIT_O, ERR_O };

struct Client {
  std::string name;
  bool isIn;
  bool is
};

struct Event {
  std::chrono::system_clock::time_point eventTime;
  int type;
  Client client;
  unsigned table = 0;

  std::string error;

  explicit Event(const std::string& line);
  void print_event();
};

class Club {
 private:
  std::chrono::system_clock::time_point openTime;
  std::chrono::system_clock::time_point closeTime;
  int pricePerHour;

  std::queue<Client> queue;
  std::set<Client> clients;
  std::unordered_map<int, bool> tables;

 public:

    Client()

  void handle_event(Event& event);
};
