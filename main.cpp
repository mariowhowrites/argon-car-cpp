#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>
using namespace std;

/**
 * types for defining our command vectors
 */
typedef map<string, function<void()>> command_map_t;
typedef command_map_t::iterator command_char_pointer;

// How much time, in ms, it takes for the engine to complete a single cycle.
const int ROUND_INTERVAL = 100;

// The engine value at which deceleration should begin. At values above this the engine will write (255,0) or (0, 255)
const int DECELERATION_CUTOFF = 50;

// Given the deceleration cutoff, how much to decelerate on each round to get to 0 by the end.
const double DECELERATION_RATE = 255 / DECELERATION_CUTOFF;

// How much to add to the engine's counter per command received
const int COMMAND_VALUE = 75;

// Mock digitalWrite to display output
void digitalWrite(string pin, int value)
{
  cout << "Writing " << value << " to pin " << pin << endl;
}

/**
 * car_engine class. handles receiving input from Particle and converting into engine inputs.
 */
class car_engine
{
public:
  car_engine(string negative_pin, string negative_str, string positive_pin, string positive_str)
      : negative_pin(negative_pin), negative_str(negative_str), positive_pin(positive_pin), positive_str(positive_str)
  {
    command_map = {
        {positive_str, [&] {
           add_direction(COMMAND_VALUE);
         }},
        {negative_str, [&] {
           add_direction(-COMMAND_VALUE);
         }}};
  };

  void input(string _input)
  {
    command_char_pointer selected_command = command_map.find(_input);

    if (selected_command != command_map.end())
    {
      selected_command->second();
    }
  }

  void run()
  {
    while (count != 0)
    {
      cout << "starting round with count " << count << endl;
      digitalWrite(positive_pin, calculate_positive_value());
      digitalWrite(negative_pin, calculate_negative_value());

      if (count > 0)
      {
        count--;
      }
      else
      {
        count++;
      }

      this_thread::sleep_for(chrono::milliseconds(ROUND_INTERVAL));
    }
  }

protected:
  command_map_t command_map;
  int count;
  string negative_pin;
  string negative_str;
  string positive_pin;
  string positive_str;

  void add_direction(int direction)
  {
    count = count + direction;
  }

  int calculate_positive_value()
  {
    if (count <= 0)
    {
      return 0;
    }

    return calculate_abs_value();
  }

  int calculate_negative_value()
  {
    if (count >= 0)
    {
      return 0;
    }

    return calculate_abs_value();
  }

  int calculate_abs_value()
  {
    int abs_val = abs(count);

    if (abs_val >= DECELERATION_CUTOFF)
    {
      return 255;
    }

    int diff = DECELERATION_CUTOFF - abs_val;

    return 255 - (diff * DECELERATION_RATE);
  }
};

/**
 * initializing our engines with pin locations
 */
car_engine front_engine{"A7", "l", "A8", "r"};
car_engine rear_engine{"B7", "b", "B8", "f"};

/**
 * main function
 */
int main()
{
  while (true)
  {
    // these calls to iostream will be replaced by external input via Particle.
    cout << "Enter a valid input" << endl;
    string command;
    cin >> command;

    // this moves into Particle.function
    for_each(command.begin(), command.end(), [&](char const &c) {
      string command_array[]{"l", "r", "b", "f"};
      string command_char(1, c);
      string *selected_command = find(command_array, command_array + 4, command_char);

      if (selected_command == command_array + 4)
      {
        return;
      }

      if (*selected_command == "l" || *selected_command == "r")
      {
        front_engine.input(*selected_command);
      }

      if (*selected_command == "b" || *selected_command == "f")
      {
        rear_engine.input(*selected_command);
      }
    });

    // this is main function logic
    front_engine.run();
    rear_engine.run();
  }
}

/**
 * 
 * Basic math:
 * 
 * Should be decelerating for 2.5 seconds for 1s of acceleration. 
 * 
 */